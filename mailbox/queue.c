#include "queue.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "mailbox.h"
// 创建对头
queue_date_phead* creat_date_phead()
{
  queue_date_phead* p = malloc(sizeof(queue_date_phead));
  if (NULL == p)
  {
    perror("error creat_date_phead");
    return NULL;
  }
  p->head = NULL;
  p->tail = NULL;
  p->len = 0;
  return p;
}

// 入队（尾插法，加锁）  成功返回1，失败返回0
int insert_queue_date_t(thread_head* threadhead, char const* txt, char* threadlinkname)
{
  if (threadhead == NULL || txt == NULL || threadlinkname == NULL)
  {
    fprintf(stderr, "insert_queue_date_t: invalid parameter\n");
    return 0;
  }

  pthread_mutex_lock(&threadhead->mutex);

  // 查找目标收件线程
  thread_link* find = threadhead->pthead->next;
  while (find != NULL)
  {
    if (0 == strcmp(find->date->name, threadlinkname))
    {
      break;
    }
    find = find->next;
  }
  if (NULL == find)
  {
    fprintf(stderr, "insert_queue_date_t: target thread '%s' not registered\n", threadlinkname);
    pthread_mutex_unlock(&threadhead->mutex);
    return 0;
  }

  // 查找发送者线程
  thread_link* sender = threadhead->pthead->next;
  while (sender != NULL)
  {
    if (sender->date->tid == pthread_self())
    {
      break;
    }
    sender = sender->next;
  }
  if (NULL == sender)
  {
    fprintf(stderr, "insert_queue_date_t: sender thread not registered\n");
    pthread_mutex_unlock(&threadhead->mutex);
    return 0;
  }

  // 创建新节点
  queue_date_t* p = malloc(sizeof(queue_date_t));
  if (p == NULL)
  {
    perror("insert_queue_date_t malloc failed");
    pthread_mutex_unlock(&threadhead->mutex);
    return 0;
  }
  memset(p, 0, sizeof(queue_date_t));

  // 填充邮件数据
  strcpy(p->maildate.txt, txt);
  p->maildate.id_of_recver = find->date->tid;
  p->maildate.id_of_sender = pthread_self();
  strcpy(p->maildate.name_of_recver, find->date->name);
  strcpy(p->maildate.name_of_sender, sender->date->name);

  // 尾插法入队
  queue_date_phead* q = find->date->head;
  p->next = NULL;
  if (q->tail == NULL)
  {
    // 队列为空，头尾都指向新节点
    q->head = p;
    q->tail = p;
  }
  else
  {
    // 队列非空，追加到尾部
    q->tail->next = p;
    q->tail = p;
  }
  q->len++;

  pthread_mutex_unlock(&threadhead->mutex);
  return 1;
}

// 出队（头删法，加锁）  成功返回1，失败返回0
int out_queue_date_t(thread_head* threadhead, mail_t* maill)
{
  if (threadhead == NULL || maill == NULL)
  {
    fprintf(stderr, "out_queue_date_t: invalid parameter\n");
    return 0;
  }

  pthread_mutex_lock(&threadhead->mutex);

  // 查找当前线程
  thread_link* find = threadhead->pthead->next;
  while (find != NULL)
  {
    if (find->date->tid == pthread_self())
    {
      break;
    }
    find = find->next;
  }
  if (NULL == find)
  {
    fprintf(stderr, "out_queue_date_t: current thread not registered\n");
    pthread_mutex_unlock(&threadhead->mutex);
    return 0;
  }

  queue_date_phead* q = find->date->head;

  // 队列为空
  if (q->head == NULL)
  {
    pthread_mutex_unlock(&threadhead->mutex);
    return 0;
  }

  // 取出头节点
  queue_date_t* p = q->head;

  // 复制数据到输出参数
  maill->id_of_recver = p->maildate.id_of_recver;
  strcpy(maill->name_of_recver, p->maildate.name_of_recver);
  maill->id_of_sender = p->maildate.id_of_sender;
  strcpy(maill->name_of_sender, p->maildate.name_of_sender);
  strcpy(maill->txt, p->maildate.txt);

  // 更新队列头指针
  q->head = p->next;
  if (q->head == NULL)
  {
    // 队列已空，尾指针也置空
    q->tail = NULL;
  }
  q->len--;

  free(p);
  pthread_mutex_unlock(&threadhead->mutex);
  return 1;
}
// 销毁
void destroy_all_queue(queue_date_phead* ps)
{
  if (ps == NULL)
  {
    return;
  }

  while (ps->head != NULL)
  {
    queue_date_t* tmp = ps->head;
    ps->head = ps->head->next;
    free(tmp);
  }

  free(ps);
}