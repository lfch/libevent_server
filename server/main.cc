#include "tcp_server.h"
#include <stdlib.h>

int main(int argc, char **argv)
{
  if (argc < 2) {
    fprintf(stderr, "usage: tcp_server <ip> <port>\n");
    return 1;
  }
  int port = atoi(argv[2]);
  if (port <= 0 || port > 65535) {
    fprintf(stderr, "invalid port\n");
    return 1;
  }
  TcpServer *server = new TcpServer(argv[1], (int16_t)port, true);
  server->start();
  return 0;
}
