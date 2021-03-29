#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <stdio.h>
#include <net/if.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <jansson.h>
#include <string.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "config.h"

// 実行時  -ljansson が必要 janssonがインストールされてない場合 sudo apt-get install libjansson-dev


int main(int argc, char** argv)
{
  struct  config cfg;
  int ret = config_parse(&cfg, argv[1]);
  if (ret < 0) {
    fprintf(stderr, "failed on config parser\n");
    return 1;
  }
  char addr_str[256];
  struct sockaddr_in addr;
  struct sockaddr_in server;
  struct bgp_open open;
  struct bgp_keepalive kep;
  int sock;
  int sock0;
  char buf[1000];
  int n;
  char nei_addr[256];

  memset(&open, 0x0, sizeof(open));

  uint8_t marker[BGP_MARKER] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

   /* ソケットの作成 */
  sock = socket(AF_INET, SOCK_STREAM, 0);
   /* 接続先指定用構造体の準備 */
  server.sin_family = AF_INET;
  server.sin_port = htons(179);
  strcpy(nei_addr,inet_ntoa(cfg.neighbors[0]->address));
  server.sin_addr.s_addr = inet_addr(nei_addr);

   /* サーバに接続 */
  connect(sock, (struct sockaddr *)&server, sizeof(server));
  
  printf("BGP OPEN");
  printf("send to...");


  memcpy(open.marker, marker, 16);
  open.len = htons(0x001D);
  open.type = BGP_MSG_TYPE_OPEN; 

  open.version = 0x04;
  open.my_autonomous_system = htons(0xFDE9);
  open.hold_time = htons(0x00B4);
  open.bgp_identifier = cfg.router_id;
  open.opt_parm_length = 0x00;
  sendto(sock, &open, sizeof(open) - 3,
      0, (struct sockaddr *)&addr, sizeof(addr));

  }
  while(1) {
    memset(buf, 0, sizeof(buf));
    n = recv(sock, buf, sizeof(buf), 0);
    if (n < 1) {
      printf("okasii");
      return -1;
  }
    struct bgp_keepalive *bgp_keepalive = (struct bgp_keepalive *)buf;
    if (bgp_keepalive->type == BGP_MSG_TYPE_KEEPALIVE) {
    memcpy(kep.marker, marker, 16);
    kep.len = htons(0x0013);
    open.type = BGP_MSG_TYPE_OPEN; 
    sendto(sock, &kep, sizeof(kep) - 1,
        0, (struct sockaddr *)&addr, sizeof(addr));
  }
 }
   /* socketの終了 */
  close(sock);
  return 0;
}
