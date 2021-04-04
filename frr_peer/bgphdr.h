#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <jansson.h>
#include <string.h>

#define BGP_MSG_TYPE_OPEN 1
#define BGP_MSG_TYPE_UPDATE 2
#define BGP_MSG_TYPE_NOTFICATION 3
#define BGP_MSG_TYPE_KEEPALIVE 4
#define BGP_MSG_TYPE_ROUTEREFRESH 5

#define BGP_MARKER 16
#define BGP_OPEN_OPT 48

struct bgp_open {
 uint8_t marker[BGP_MARKER];
 uint16_t len;
 uint8_t type;
 uint8_t version;
 uint16_t my_autonomous_system;
 uint16_t hold_time;
 struct in_addr bgp_identifier;
 uint8_t opt_parm_length;
 uint8_t opt[BGP_OPEN_OPT];
};
struct bgp_keepalive {
 uint8_t marker[BGP_MARKER];
 uint16_t len;
 uint8_t type;
};

struct bgp_update {
 uint8_t marker[BGP_MARKER];
 uint16_t len;
 uint8_t type;
 uint16_t wl[2];
 uint16_t pl;
};

