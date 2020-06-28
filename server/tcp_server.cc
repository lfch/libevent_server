#include "tcp_server.h"
#include <string.h>
#include <event2/bufferevent.h>

extern std::atomic<int32_t> pkg_len;

char TcpServer::buf_[];
std::mutex TcpServer::mutex_;

bool TcpServer::start()
{
  return server_.start();
}

void TcpServer::on_read(struct bufferevent* bev, void *ctx)
{
  evbuffer *input = bufferevent_get_input(bev) ;
  //size_t input_len = bufferevent_get_length(input);
  //fprintf(stdout, "input length: %u\n", input_len);
  size_t len;
  while (true) {
    mutex_.try_lock();
    memset(buf_, 0, sizeof(buf_));
    len = bufferevent_read(bev, buf_, sizeof(buf_));
    mutex_.unlock();
    if (len > 0) {
      //fprintf(stdout, "%s", buf_);
      cur_cnt.fetch_add(1);
      pkg_len.store(len);
    } 
    if (len <= 0) {
      //fprintf(stdout, "no more data.\n");
      break;
    }

  }
}

void TcpServer::on_write(struct bufferevent* bev, void *ctx)
{
  evbuffer* output = bufferevent_get_output(bev);
  mutex_.try_lock();
  evbuffer_add(output, buf_, strlen(buf_));
  mutex_.unlock();
}
  
void TcpServer::on_conn(struct evconnlistener *listener,
                        evutil_socket_t fd,
                        struct sockaddr *addr,
                        int socklen,
                        void *ctx)
{
  event_base *base = evconnlistener_get_base(listener);
  bufferevent *bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);
  bufferevent_setcb(bev, TcpServer::on_read,
      TcpServer::on_write, TcpServer::on_event, nullptr);
  bufferevent_enable(bev, EV_READ | EV_WRITE);
}
  
void TcpServer::on_error(struct evconnlistener *listener, void *ctx)
{
  event_base *base = evconnlistener_get_base(listener);
  int err = EVUTIL_SOCKET_ERROR();
  fprintf(stderr, "got an error %d (%s) on the listener, shutting down\n",
         err, evutil_socket_error_to_string(err));
  event_base_loopexit(base, nullptr);
}

void TcpServer::on_event(struct bufferevent* bev, short events, void* ctx)
{
  if (events & BEV_EVENT_ERROR) {
    fprintf(stderr, "error from bufferevent\n");
  } 
  if (events & BEV_EVENT_EOF) {
    fprintf(stdout, "eof now, free now\n");
    bufferevent_free(bev);
  }
}
