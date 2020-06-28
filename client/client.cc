#include <iostream>
#include <stdlib.h>
#include <time.h>
using namespace std;
 
 char *randstr(char *str, const int len)
 {
       srand(time(NULL));
       int i;
       for (i = 0; i < len; ++i)
   {
             switch ((rand() % 3))
     {
               case 1:
                   str[i] = 'A' + rand() % 26;
                   break;
               case 2:
                   str[i] = 'a' + rand() % 26;
                   break;
               default:
                   str[i] = '0' + rand() % 10;
                   break;
               
     }
         
   }
       str[++i] = '\0';
       return str;

 }
  
/*  int main(void)
  {
     
        char name[20];
        cout << randstr(name, 8) << endl;
        system("pause");
        return 0;

  }
*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <event2/event.h>
#include <event2/bufferevent.h>
#include <atomic>

int pipefd[2];
pthread_t tid;
std::atomic<int64_t> counter(0);
std::atomic<int64_t> pre_cnt(0);
std::atomic<int32_t> pkg_len(0);

void read_cb(struct bufferevent *bev, void *arg)
{
      char buf[1024] = {0};
      bufferevent_read(bev, buf, sizeof(buf));
      printf("say:%s\n", buf);
      bufferevent_write(bev, buf, strlen(buf) +1);
      sleep(1);
}

void write_cb(struct bufferevent *bev, void *arg)
{
      //printf("我是客户端的写回调函数，没卵用\n");
}
void event_cb(struct bufferevent *bev, short events, void * arg)
{
  if(events & BEV_EVENT_EOF) {
    printf("connection closed \n");
  }
  else if(events & BEV_EVENT_ERROR) {
    printf("some other error\n");
  }
  else if(events & BEV_EVENT_CONNECTED) {
    printf("已经连接了服务器.....\n");                        
    return ;
  }
  //释放资源
  bufferevent_free(bev);
}

void read_terminal(evutil_socket_t fd, short what, void *arg)
{
  char buf[1024] = {0};
  int len = read(fd, buf, sizeof(buf));
  struct bufferevent* bev = (struct bufferevent*)arg;
  bufferevent_write(bev, buf, len+1);
  pkg_len.store(len);
  counter.fetch_add(1);
}

void* func(void* arg)
{
  int* pipefd = (int*)arg;
  char buf[1024] = {0};
  while (true) {
    memset(buf, 0, sizeof(buf));
    randstr(buf, sizeof(buf));
    //printf("rand str: %s\n", buf);
    write(*pipefd, buf, strlen(buf));
    //sleep(1);
  }
}

void stats(evutil_socket_t, short, void*)
{
  printf("pre_cnt: %ld, cur_cnt: %ld, qps: %ld, pkg_len: %d\n",
         pre_cnt.load(), counter.load(), counter.load() - pre_cnt.load(),
        pkg_len.load());
  pre_cnt.store(counter.load());
}

int main(int argc, char **argv)
{
  if (argc < 2) {
    printf("usage: client <ip> <port>\n");
    return -1;
  }
  struct event_base* base = NULL;
  base = event_base_new();
  struct bufferevent *bev = NULL;
  bev = bufferevent_socket_new(base, -1, BEV_OPT_CLOSE_ON_FREE);

  char* server_ip = argv[1];
  int32_t server_port = atoi(argv[2]);

  struct sockaddr_in serv;
  memset(&serv, 0, sizeof(serv));
  serv.sin_family = AF_INET;
  serv.sin_port = htons((int16_t)server_port);
  inet_pton(AF_INET, server_ip, &serv.sin_addr.s_addr);

  bufferevent_socket_connect(bev, (struct sockaddr*)&serv, sizeof(serv));
  bufferevent_setcb(bev, read_cb, write_cb, event_cb, NULL);

  bufferevent_enable(bev, EV_READ);

  pipe(pipefd);
  pthread_create(&tid, nullptr, func, &pipefd[1]);

  struct event* ev = event_new(base, pipefd[0], EV_READ | EV_PERSIST, read_terminal, bev);
  event_add(ev, nullptr);

  struct timeval one_sec;
  one_sec.tv_sec = 1;
  one_sec.tv_usec = 0;
  struct event* timer_ev = event_new(base, -1, EV_PERSIST, stats, nullptr);
  event_add(timer_ev, &one_sec);

  event_base_dispatch(base);
  event_free(ev);
  event_free(timer_ev);
  event_base_free(base);

  return 0;
}

