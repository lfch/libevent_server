/*************************************************************************
	> File Name: server_base.h
	> Author: Fengchang Lv 
	> Mail:  
	> Note: 
	> Created Time: Sun 28 Jun 2020 02:45:09 PM CST
 ************************************************************************/

#ifndef _SERVER_BASE_H
#define _SERVER_BASE_H

#include <string>
#include <sstream>
#include <functional>
// #include <memory>
#include <atomic>

#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/event.h>

using ReadCallback = bufferevent_data_cb;
using WriteCallback = bufferevent_data_cb;
using ConnectionCallback = evconnlistener_cb;
using ErrorCallback = evconnlistener_errorcb;
using EventCallback = bufferevent_event_cb;

extern std::atomic<int64_t> pre_cnt;
extern std::atomic<int64_t> cur_cnt;

class ServerBase {
 public:
  ServerBase(const std::string& ip, int16_t port, bool reuse_port);
  ~ServerBase();

  bool start();

  void set_read_cb(const ReadCallback& read_cb) { read_cb_ = read_cb; }
  const ReadCallback& read_cb() { return read_cb_; }

  void set_write_cb(const WriteCallback& write_cb) { write_cb_ = write_cb; }
  const WriteCallback& write_cb() { return write_cb_; }

  void set_conn_cb(const ConnectionCallback& conn_cb) { conn_cb_ = conn_cb; }
  const ConnectionCallback& conn_cb() { return conn_cb_; }

  void set_err_cb(const ErrorCallback& err_cb) { err_cb_ = err_cb; }
  const ErrorCallback& err_cb() { return err_cb_; }

  void set_event_cb(const EventCallback& event_cb) { event_cb_ = event_cb; }
  const EventCallback& event_cb() { return event_cb_; }

  const std::string& ip_port() const {
    std::stringstream ss;
    ss << ip_;
    ss << ",";
    ss << port_;
    return ss.str();
  }

  static void stats(evutil_socket_t, short, void*);

 private:
  ReadCallback read_cb_;
  WriteCallback write_cb_;
  ConnectionCallback conn_cb_;
  ErrorCallback err_cb_;
  EventCallback event_cb_;
  bool reuse_port_ = false;
  std::string ip_;
  int16_t port_;
  // std::unique_ptr<event_base> base_;
  // std::unique_ptr<evconnlistener> listener_; 
  event_base *base_;
  evconnlistener* listener_;
  event* timer_ev_;
};
#endif
