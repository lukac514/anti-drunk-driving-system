#include "mailbox.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 创建邮箱系统
thread_head *creat_thread_head()
{
    thread_head *p = malloc(sizeof(thread_head));
    if (p == NULL)
    {
        perror("creat_thread_head malloc error");
        return NULL;
    }

    int ret = pthread_mutex_init(&p->mutex, NULL);
    if (ret != 0)
    {
        perror("creat_thread_head mutex init failure");
        free(p);
        return NULL;
    }

    p->pthead = malloc(sizeof(thread_link));
    if (p->pthead == NULL)
    {
        perror("creat_thread_head sentinel node error");
        pthread_mutex_destroy(&p->mutex);
        free(p);
        return NULL;
    }
    p->pthead->date = NULL;
    p->pthead->next = NULL;

    printf("mail box create ok!!\n");
    return p;
}

// 插入线程（加锁保护链表操作）
int insert_thread_link(thread_head *threadhead, char *threadname, th_fun th)
{
    if (threadhead == NULL || threadname == NULL || th == NULL)
    {
        fprintf(stderr, "insert_thread_link: invalid parameter\n");
        return -1;
    }

    thread_link *p = malloc(sizeof(thread_link));
    if (p == NULL)
    {
        perror("insert_thread_link malloc thread_link error");
        return -1;
    }

    p->date = malloc(sizeof(thread_date));
    if (p->date == NULL)
    {
        perror("insert_thread_link malloc thread_date error");
        free(p);
        return -1;
    }

    // 创建该线程专属的消息队列
    p->date->head = creat_date_phead();
    if (p->date->head == NULL)
    {
        perror("insert_thread_link create queue error");
        free(p->date);
        free(p);
        return -1;
    }

    // 初始化线程数据
    strcpy(p->date->name, threadname);
    p->date->th = th;
    p->date->head->head = NULL;
    p->date->head->tail = NULL;
    p->date->head->len = 0;

    // 加锁后插入链表
    pthread_mutex_lock(&threadhead->mutex);
    p->next = threadhead->pthead->next;
    threadhead->pthead->next = p;
    pthread_mutex_unlock(&threadhead->mutex);

    // 创建线程
    int ret = pthread_create(&(p->date->tid), NULL, th, NULL);
    if (ret != 0)
    {
        perror("insert_thread_link pthread_create error");
        // 创建失败时从链表中移除
        pthread_mutex_lock(&threadhead->mutex);
        threadhead->pthead->next = p->next;
        pthread_mutex_unlock(&threadhead->mutex);
        free(p->date->head);
        free(p->date);
        free(p);
        return -1;
    }

    printf("thread %s create ok!!\n", threadname);
    return 1;
}
int send_queue_data_t(char *from_thread)
{
    mail_t tmp;
    int ret = out_queue_date_t(MBS, &tmp);
    if (ret == 0)
    {
        printf("[%s] 发送失败：队列为空\n", from_thread);
        return 0;
    }
    // 分别向 fd 和 wf 发送消息
    ret = insert_queue_date_t(MBS, tmp.txt, "fd");
    if (ret == 0)
    {
        printf("[%s] 发送到 fd 失败\n", from_thread);
    }
    ret = insert_queue_date_t(MBS, tmp.txt, "wf");
    if (ret == 0)
    {
        printf("[%s] 发送到 wf 失败\n", from_thread);
    }
    printf("[%s] 发送成功：%s\n", from_thread, tmp.txt);
    return 1;
}
