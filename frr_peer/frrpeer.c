#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <stdio.h>
#include <net/if.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "bgphdr.h"

int main(int argc, char** argv)
{
    char addr_str[256];
    struct sockaddr_in addr;
    struct sockaddr_in server;
    struct bgp_open open;
    struct bgp_keepalive kep;
    struct bgp_update upd;
    int sock;
    char buf[1000];
    int n;

    memset(&open, 0x0, sizeof(open));

    uint8_t marker[BGP_MARKER] = {0xFF, 0xFF, 0xFF, 0xFF
        , 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
          0xFF, 0xFF, 0xFF, 0xFF};

    uint8_t option_val[BGP_OPEN_OPT] = {0x02, 0x06, 0x01
        , 0x04, 0x00, 0x01, 0x00, 0x01, 0x02, 0x02, 0x80,
    0x00, 0x02, 0x02, 0x02, 0x00, 0x02, 0x06, 0x41, 0x04,
    0x00, 0x00, 0xFD, 0xE8, 0x02, 0x06, 0x45, 0x04, 0x00,
    0x01, 0x01, 0x00, 0x02, 0x08, 0x49, 0x06, 0x04, 0x6E,
    0x65, 0x74, 0x31, 0x00, 0x02, 0x04, 0x40, 0x02, 0x00,
    0x78};
   

    sock = socket(AF_INET, SOCK_STREAM, 0);

    server.sin_family = AF_INET;
    server.sin_port = htons(179);
    server.sin_addr.s_addr = inet_addr("10.255.1.1");

    connect(sock, (struct sockaddr *)&server, sizeof(server));
    memcpy(open.marker, marker, 16);

    open.len = htons(0x004D);

    open.type = BGP_MSG_TYPE_OPEN;

    open.version = 0x04;

    open.my_autonomous_system = htons(0xFDE8);

    open.hold_time = htons(0x00B4);

    inet_pton(AF_INET, "1.1.1.1", &open.bgp_identifier);

    open.opt_parm_length = 0x30;

    memcpy(open.opt, option_val, 48);

    sendto(sock, &open, sizeof(open) - 3,
        0, (struct sockaddr *)&addr, sizeof(addr));

    printf("BGP Open\n");

    while(1) {
      memset(buf, 0, sizeof(buf));
      n = recv(sock, buf, sizeof(buf), 0);
      if (n < 1) {
        printf("okasii");
        return -1;
      }

      struct bgp_keepalive *bgp_keepalive =  (struct bgp_keepalive *)buf;
      if (bgp_keepalive->type == BGP_MSG_TYPE_KEEPALIVE) {

         printf("keep alive send to... \n");

         memcpy(kep.marker, marker, 16);

         kep.len = htons(0x0013);

         kep.type = 0x04;

         sendto(sock, &kep, sizeof(kep) - 1,
             0, (struct sockaddr *)&addr, sizeof(addr));

      }
      struct bgp_update *bgp_update =  (struct bgp_update *)buf;
      if (bgp_update->type == BGP_MSG_TYPE_UPDATE) {

         printf("BGP UPDATE! \n");
         printf("send to... \n");

         memcpy(upd.marker, marker, 16);

         upd.len = htons(0x0017);

         upd.type = 0x02;

         upd.wl = 0x0000;

         upd.pl = 0x0000;

         sendto(sock, &upd, sizeof(upd) - 1,
             0, (struct sockaddr *)&addr, sizeof(addr));
     }
    }
  close(sock);
  return 0;
}

