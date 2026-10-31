#ifndef __QUEUE__H__
#define __QUEUE__H__
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


typedef struct Link thread_head;



typedef struct maill
{
   pthread_t   id_of_sender;
    char       name_of_sender[256];
    pthread_t   id_of_recver;
    char       name_of_recver[256];
    char       txt[256];
}mail_t;


// 定义节点
typedef struct date
{
mail_t     maildate;
struct date*next;
}queue_date_t;

// 定义队列头
typedef struct head
{
queue_date_t*head;
queue_date_t*tail;
int len;
}queue_date_phead;


queue_date_phead* creat_date_phead();
int insert_queue_date_t(thread_head* threadhead,char const*txt,char *threadlinkname);
int out_queue_date_t(thread_head* threadhead,mail_t*maill);
#endif  //!__QUEUE__H__