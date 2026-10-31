
#include <pthread.h>
#include <stdio.h>
#include "dev.h"
#include "mailbox/mailbox.h"
#include "mailbox/net.h"
#include "mailbox/tou.h"
#include "mqtt/mqtt.h"
#include "lv_port_linux_frame_buffer/lvgl/lvgl.h"
#include "lv_port_linux_frame_buffer/gui/generated/gui_guider.h"
#include "lv_port_linux_frame_buffer/lv_drivers/display/fbdev.h"
#include "lv_port_linux_frame_buffer/lv_drivers/indev/evdev.h"

// MBS* mbs;
thread_head *MBS = NULL;

//============ LVGL 界面 ============
#define MAX_LVGL_TASKS 20

// guider_ui 全局界面对象（gui_guider.h 中为 extern，此处定义）
lv_ui guider_ui;

typedef struct
{
  void (*task_func)(void *);  // 更新界面的任务接口
  void *arg;                  // 传递的参数
} lvgl_task_t;

static lvgl_task_t lvgl_task_queue[MAX_LVGL_TASKS];
static int lvgl_task_count = 0;
static pthread_mutex_t lvgl_queue_mutex = PTHREAD_MUTEX_INITIALIZER;

// 添加任务到队列（由业务线程调用，如 mq3/hcsr04/mqtt/wf 线程）
void add_lvgl_task(void (*task_func)(void *), void *arg)
{
  pthread_mutex_lock(&lvgl_queue_mutex);
  if (lvgl_task_count < MAX_LVGL_TASKS)
  {
    lvgl_task_queue[lvgl_task_count].task_func = task_func;
    lvgl_task_queue[lvgl_task_count].arg = arg;
    lvgl_task_count++;
  }
  else
  {
    printf("LVGL任务队列已满！\n");
  }
  pthread_mutex_unlock(&lvgl_queue_mutex);
}

// 执行队列中的所有任务（必须在 LVGL 主线程调用）
void process_lvgl_tasks(void)
{
  pthread_mutex_lock(&lvgl_queue_mutex);
  for (int i = 0; i < lvgl_task_count; i++)
  {
    lvgl_task_queue[i].task_func(lvgl_task_queue[i].arg);
  }
  lvgl_task_count = 0;
  pthread_mutex_unlock(&lvgl_queue_mutex);
}

// LVGL 显示缓冲 800x480
#define DISP_BUF_SIZE (128 * 1024)

// LV_TICK_CUSTOM 需要的系统 tick 函数（lv_conf.h 里引用 custom_tick_get()）
uint32_t custom_tick_get(void)
{
  static uint64_t start_ms = 0;
  if (start_ms == 0)
  {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    start_ms = (tv.tv_sec * 1000000 + tv.tv_usec) / 1000;
  }
  struct timeval tv;
  gettimeofday(&tv, NULL);
  uint64_t now_ms = (tv.tv_sec * 1000000 + tv.tv_usec) / 1000;
  return (uint32_t)(now_ms - start_ms);
}

// LVGL 主线程：初始化 fbdev/evdev/GUI 并跑定时器
void *lvgl_th(void *arg)
{
  lv_init();
  fbdev_init();

  static lv_color_t buf[DISP_BUF_SIZE];
  static lv_disp_draw_buf_t disp_buf;
  lv_disp_draw_buf_init(&disp_buf, buf, NULL, DISP_BUF_SIZE);

  static lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);
  disp_drv.draw_buf  = &disp_buf;
  disp_drv.flush_cb  = fbdev_flush;
  disp_drv.hor_res   = 800;
  disp_drv.ver_res   = 480;
  lv_disp_drv_register(&disp_drv);

  evdev_init();
  static lv_indev_drv_t indev_drv;
  lv_indev_drv_init(&indev_drv);
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  indev_drv.read_cb = evdev_read;
  lv_indev_drv_register(&indev_drv);

  // Gui-Guider 生成的酒精检测界面
  setup_ui(&guider_ui);

  while (1)
  {
    lv_timer_handler();
    process_lvgl_tasks();
    usleep(5000);
  }
  return NULL;
}

// 更新界面标签：把队列消息显示到 screen_label_1（收到后释放传入的堆内存）
void update_label_task(void *arg)
{
  mail_t *m = (mail_t *)arg;
  if (m != NULL)
  {
    lv_label_set_text(guider_ui.screen_label_1, m->txt);
    free(m);
  }
}

// 显示线程：从邮箱队列取数据，交给 LVGL 主线程更新标签
void *lvgl_show_th(void *arg)
{
  while (1)
  {
    mail_t tmp;
    memset(&tmp, 0, sizeof(tmp));
    int ret = out_queue_date_t(MBS, &tmp);
    if (ret == 1)
    {
      mail_t *copy = malloc(sizeof(mail_t));
      if (copy != NULL)
      {
        *copy = tmp;
        add_lvgl_task(update_label_task, copy);
      }
    }
    sleep(1);
  }
  return NULL;
}
int fd_adxl235 = 0;
int fd_mq3 = 0;
int fd_hcsr04 = 0;

void *mq3_date(void *arg)
{
  while (1)
  {
    int raw = 0;
    float scale = 0.0;
    char tmp[64] = {0};
    ssize_t a = read(fd_mq3, &mq3, sizeof(mq3));
    if (a != 64)
    {
      printf("WDNMD\n");
      return NULL;
    }
    raw = atoi(mq3.raw);
    scale = atof(mq3.scale);
    float vol = (raw * scale) / 1000.f;
    sleep(2);

    sprintf(tmp, "%.2f", vol);
    const char *vol_message = tmp;
    insert_queue_date_t(MBS, vol_message, "mq3");
    printf("[mq3 thread] 已完成 1 条消息入队\n");
    send_queue_data_t("vol");
    sleep(1);
  }
  close(fd_mq3);
  return NULL;
}
void *hcsr04_date(void *arg)
{
  int a;
  char tmp[32] = {0};

  while (1)
  {
    int ret = read(fd_hcsr04, &a, sizeof(a));
    if (ret == sizeof(a))
    {
      int cm = (a * 340LL) / 20000;
      snprintf(tmp, sizeof(tmp), "%d", cm);
      const char *hcsr04_message = tmp;
      // HC‑SR04有效区间3~400厘米
      if (cm >= 3 && cm <= 400)
      {
        insert_queue_date_t(MBS, hcsr04_message, "hcsr04");
      }
      else
      {
        insert_queue_date_t(MBS, "-----无效读数-----", "hcsr04");
      }
      send_queue_data_t("hcsr04");
    }
    sleep(1);
  }
  return NULL;
}
// void *adxl235_date(void *arg)
// {
//   while (1)
//   {
//     char buf[6] = {0};
//     int ret = read(fd, buf, 6);
//     if (ret < 0)
//     {
//       perror("read");
//       break;
//     }
//     short adx1_x = buf[1] << 8 | buf[0];
//     short adx1_y = buf[3] << 8 | buf[2];
//     short adx1_z = buf[5] << 8 | buf[4];
//     sleep(1);
//   }
//   return NULL;
//   // insert_queue_date_t(MBS, "message 2", "vol");
//   // send_queue_data_t("vol");
// }
void *vol_date(void *arg)
{
  while (1)
  {
    insert_queue_date_t(MBS, "message 1", "vol");
    insert_queue_date_t(MBS, "message 2", "vol");
    printf("[vol thread] 已完成 2 条消息入队\n");
    send_queue_data_t("vol");
    send_queue_data_t("vol");
    sleep(1);
  }
  return NULL;
}
void *fd_date(void *arg)
{
  while (1)
  {
    sleep(2);
    mail_t as;
    memset(&as, 0, sizeof(as));

    int ret = out_queue_date_t(MBS, &as);
    if (ret == 0)
    {
      printf("[fd thread] 队列为空，等待消息...\n");
      continue;
    }

    printf("[fd thread] 发送者ID：%ld\n", as.id_of_sender);
    printf("[fd thread] 发送者名：%s\n", as.name_of_sender);
    printf("[fd thread] 接收者ID：%ld\n", as.id_of_recver);
    printf("[fd thread] 接收者名：%s\n", as.name_of_recver);
    printf("[fd thread] 消息内容：%s\n", as.txt);
  }
  return NULL;
}

void *wf_date(void *arg)
{
  mail_t as;
  char buff[512] = {0};

  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0)
  {
    perror("wf_date socket error");
    return NULL;
  }

  struct sockaddr_in seraddr;
  seraddr.sin_family = AF_INET;
  seraddr.sin_port = htons(SER_PORT);
  seraddr.sin_addr.s_addr = inet_addr(SER_IP);

  int ret = connect(sockfd, (struct sockaddr *)&seraddr, sizeof(seraddr));
  if (ret < 0)
  {
    perror("wf_date connect error");
    close(sockfd);
    return NULL;
  }

  while (1)
  {
    sleep(2);
    memset(&as, 0, sizeof(as));
    int ret = out_queue_date_t(MBS, &as);
    if (ret == 0)
    {
      printf("[wf thread] 队列为空，等待消息...\n");
      continue;
    }
    strcpy(buff, as.txt);
    send(sockfd, buff, strlen(buff), 0);
    memset(buff, 0, sizeof(buff));
  }

  close(sockfd);
  return NULL;
}

void *mqtt_date(void *arg)
{
  mail_t as;
  char buff[512] = {0};
  while (1)
  {
    sleep(2);
    memset(&as, 0, sizeof(as));
    int ret = out_queue_date_t(MBS, &as);
    if (ret == 0)
    {
      printf("[wf thread] 队列为空，等待消息...\n");
      continue;
    }
    strcpy(buff, as.id_of_sender);
    if (0 == strcmp(buff, "hcsr04"))
    {
      mqtt_send("hcsr04", as.txt);
    }
    if (0 == strcmp(buff, "mq3"))
    {
      mqtt_send("mq3", as.txt);
    }
    memset(buff, 0, sizeof(buff));
  }
  return NULL;
}

int main()
{
  mqtt_init();
  fd_adxl235 = adxl235_init();
  fd_mq3 = mq3_init();
  fd_hcsr04 = hcsr04_init();
  MBS = creat_thread_head();
  sleep(1);
  insert_thread_link(MBS, "vol", mq3_date);
  insert_thread_link(MBS, "vol", hcsr04_date);
  insert_thread_link(MBS, "fd", fd_date);
  insert_thread_link(MBS, "wf", wf_date);
  insert_thread_link(MBS, "wf", mqtt_date);

  // LVGL 界面线程 + 数据显示线程（读 MBS 邮箱队列更新屏幕）
  pthread_t tid_lvgl, tid_show;
  pthread_create(&tid_lvgl, NULL, lvgl_th, NULL);
  pthread_create(&tid_show, NULL, lvgl_show_th, NULL);

  while (1)
  {
  }
  printf("Hello World!");
  return 0;
}