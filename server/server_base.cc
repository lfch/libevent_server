/*************************************************************************
	> File Name: server_base.cpp
	> Author: Fengchang Lv 
	> Mail:  
	> Note: 
	> Created Time: Sun 28 Jun 2020 03:18:43 PM CST
 ************************************************************************/

#include "server_base.h"

#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>

std::atomic<int64_t> pre_cnt(0);
std::atomic<int64_t> cur_cnt(0);
std::atomic<int32_t> pkg_len(0);

ServerBase::ServerBase(const std::string& ip, int16_t port, bool reuse_port)
    : reuse_port_(reuse_port)
    , ip_(ip)
    , port_(port) {}

ServerBase::~ServerBase() {}

bool ServerBase::start() {
  // base_.reset(event_base_new());
  base_ = event_base_new();
  if (!base_) {
    fprintf(stderr, "open event base failed\n");
    return false;
  }
  struct sockaddr_in sin;
  memset(&sin, 0, sizeof(sin));
  sin.sin_family = AF_INET;
  if (inet_pton(AF_INET, ip_.c_str(), &sin.sin_addr.s_addr) < 0) {
    fprintf(stderr, "convert ip failed, src: %s\n", ip_.c_str());
    return false;
  }
  sin.sin_port = htons(port_);
  unsigned flags = LEV_OPT_CLOSE_ON_FREE;
  if (reuse_port_) {
    flags |= LEV_OPT_REUSEABLE;
  }
  // listener_.reset(evconnlistener_new_bind(base_, conn_cb_, nullptr,
  listener_ = evconnlistener_new_bind(base_, conn_cb_, nullptr,
        flags, -1, (struct sockaddr*)&sin, sizeof(sin));
  if (!listener_) {
    fprintf(stderr, "create listener failed\n");
    return false;
  }
  evconnlistener_set_error_cb(listener_, err_cb_);

  struct timeval one_sec;
  one_sec.tv_sec = 1;
  one_sec.tv_usec = 0;
  struct event* timer_ev = event_new(base_, -1, EV_PERSIST, stats, nullptr);
  event_add(timer_ev, &one_sec);

  event_base_dispatch(base_);
  return true;
}

void ServerBase::stats(evutil_socket_t, short, void*)
{
    printf("pre_cnt: %ld, cur_cnt: %ld, qps: %ld, pkg_len: %d\n",
            pre_cnt.load(), cur_cnt.load(),
            cur_cnt.load() - pre_cnt.load(),
            pkg_len.load());
    pre_cnt.store(cur_cnt.load());
}
