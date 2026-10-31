#include"net.h"
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


int init_tcp_ser()
{
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0)
  {
    perror("init_tcp_ser socket error");
    return -1;
  }
  struct sockaddr_in as;
  as.sin_family = AF_INET;
  as.sin_port = htons(SER_PORT);
  as.sin_addr.s_addr = inet_addr(SER_IP);
  int ret = bind(sockfd, (struct sockaddr *)&as, sizeof(as));
  if (ret < 0)
  {
    perror("init_tcp_ser bind error");
    close(sockfd);
    return -1;
  }
  ret = listen(sockfd, 10);
  if (ret < 0)
  {
    perror("init_tcp_ser listen error");
    close(sockfd);
    return -1;
  }
  return sockfd;
}

int recv_http_request(int connfd, char *p, int maxlen)
{
    memset(p, 0, maxlen);
  ssize_t size = recv(connfd, p, maxlen, 0);
  if (size < 0)
  {
    perror("recv_http_request recv error");
    return (int)size;
  }
  return (int)size;
}