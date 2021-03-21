#include <stdio.h>
#include <netinet/in.h>
#include <net/if.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <jansson.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "netlink.h"
#include "config.h"

int main(int argc, char** argv) {
  struct  config cfg;
  int ret = config_parse(&cfg, argv[1]);
  if (ret < 0) {
    fprintf(stderr, "failed on config parser\n");
    return 1;
  }
  char addr_str[256];
  struct sockaddr_in addr;
  struct sockaddr_in client;
  struct bgp_open open;
  int sock0;
  int sock;
  int option = 1;
  char hex_ok[4];
  memset(&open, 0x0, sizeof(open));

  uint8_t marker[16]  = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
  uint8_t bgp_len[2] = {0x00, 0x00};

  uint8_t hold_time[2] = {0x00, 0xB4};
  uint8_t as[2] = {0xFD, 0xE9}; 

  sock0 = socket(AF_INET, SOCK_STREAM, 0);
  int fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
  setsockopt(sock0, SOL_SOCKET, SO_REUSEADDR, 
             &option, sizeof(option));
  
  addr.sin_family = AF_INET;
  addr.sin_port = htons(179);
  addr.sin_addr.s_addr = INADDR_ANY;
  bind(sock0, (struct sockaddr*) &addr, sizeof(addr));

  listen(sock0, 1);

  int len = sizeof(client);
  sock = accept(sock0, (struct sockaddr *)&client, &len);
  memcpy(open.marker, marker, 16);
  open.len = 0x0000;
  open.type = BGP_MSG_TYPE_OPEN; 

  open.version = 0x04;
  open.my_autonomous_system = htons(0xFDE9);
  open.hold_time = htons(0x00B4);
  open.bgp_identifier = cfg.router_id;
  open.opt_parm_length = 0x0000;


  sendto(sock, &open, sizeof(open),
             0, (struct sockaddr *)&addr, sizeof(addr));
  
  close(sock0);
  close(sock);
  

  return 0;

}

