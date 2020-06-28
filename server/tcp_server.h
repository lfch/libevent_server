/*************************************************************************
	> File Name: tcp_server.h
	> Author: Fengchang Lv 
	> Mail:  
	> Note: 
	> Created Time: Sun 28 Jun 2020 03:21:35 PM CST
 ************************************************************************/

#ifndef _TCP_SERVER_H
#define _TCP_SERVER_H

#include <string.h>
#include <mutex>
#include "server_base.h"

class TcpServer {
 public:
  TcpServer(const std::string ip, int32_t port, bool reuse_port)
    : server_(ip, port, reuse_port)
  {
    server_.set_read_cb(TcpServer::on_read);
    server_.set_write_cb(TcpServer::on_write);
    server_.set_conn_cb(TcpServer::on_conn);
    server_.set_err_cb(TcpServer::on_error);
    server_.set_event_cb(TcpServer::on_event);
    memset(buf_, 0, sizeof(buf_));
  }

  bool start();

 private:
  static void on_read(struct bufferevent* bev, void *ctx);
  static void on_write(struct bufferevent* bev, void *ctx);
  static void on_conn(struct evconnlistener *, evutil_socket_t,
               struct sockaddr *, int socklen, void *);
  static void on_error(struct evconnlistener *, void *);
  static void on_event(struct bufferevent* bev, short events, void* ctx);

 private:
  ServerBase server_;
  static char buf_[1024*1024];
  static std::mutex mutex_;
};

#endif
