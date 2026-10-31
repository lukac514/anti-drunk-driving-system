#ifndef __MAILBOX_H__
#define __MAILBOX_H__

#include <arpa/inet.h>
#include <dirent.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#include "queue.h"
extern thread_head *MBS;
typedef void* (*th_fun)(void* arg);

// 线程信息
typedef struct thread_node
{
  pthread_t tid;           //线程id号
  char name[256];          //线程名字 ,必须唯一
  queue_date_phead* head;  //创建队列
  th_fun th;
} thread_date;

// 线程链表
typedef struct threadlink
{
  thread_date* date;
  struct threadlink* next;
} thread_link;

// 线程头
typedef struct Link
{
  pthread_mutex_t mutex;
  thread_link* pthead;
} thread_head;

thread_head* creat_thread_head();
int insert_thread_link(thread_head* threadhead, char* threadname, th_fun th);
int send_queue_data_t(char* from_thread);
#endif