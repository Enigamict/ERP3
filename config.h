#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <jansson.h>
#include <string.h>

#define MAX_NEIGH 32
#define MAX_NETWORK 256
#define MAX_HOP 32
#define BGP_MSG_TYPE_OPEN 1
#define BGP_MSG_TYPE_UPDATE 2
#define BGP_MSG_TYPE_NOTFICATION 3
#define BGP_MSG_TYPE_KEEPALIVE 4
#define BGP_MSG_TYPE_ROUTEREFRESH 5

#define BGP_MARKER 16

struct config {
  uint16_t as_number;
  struct in_addr router_id;
  struct neighbor *neighbors[MAX_NEIGH];
  struct network *networks[MAX_NETWORK];
};

struct neighbor {
  struct in_addr address;
};

struct prefix {
  struct in_addr addr;
  uint32_t length;
};

struct network {
  struct prefix prefix;
};

struct message {
  uint32_t type; // MSG_TYPE_XX
  struct in_addr path[MAX_HOP];
  struct in_addr nexthop;
  struct prefix networks[MAX_NETWORK];
};


struct bgp_open {
 uint8_t marker[BGP_MARKER];
 uint16_t len;
 uint8_t type;
 uint8_t version;
 uint16_t my_autonomous_system;
 uint16_t hold_time;
 struct in_addr bgp_identifier;
 uint8_t opt_parm_length;
 // optionどうやって書こう
};

static inline void print_config(const struct config* cfg) 
{
  char addr_str[256];
  int as_number[256];
  inet_ntop(AF_INET, &cfg->router_id, addr_str, sizeof(addr_str));
  printf("router-id: %s\n", addr_str);
  printf("%d", cfg->as_number);
  for (size_t i = 0; i < MAX_NEIGH; i++){
    if (!cfg->neighbors[i])
      continue;
    inet_ntop(AF_INET, &cfg->neighbors[i]->address,
              addr_str, sizeof(addr_str));
    printf("neighbor[%ld]address: %s\n", i, addr_str);
  }
  for (size_t i = 0; i < MAX_NETWORK; i++){
    if (!cfg->networks[i])
      continue;
    inet_ntop(AF_INET, &cfg->networks[i]->prefix.addr,
              addr_str, sizeof(addr_str));
    printf("network[%ld]: %s/%d\n", i, addr_str,
	  cfg->networks[i]->prefix.length);
  }
}
static inline int
config_parse(struct config* cfg, char* json)
{
  int as_number[256];
  char addr_str[256];
  json_t *read_json_ob;
  json_error_t error;
  read_json_ob = json_load_file(json, 0, &error);
  if (read_json_ob == NULL ) {
    printf("json configが読み取れません\n");
    return -1;
  }
  inet_pton(AF_INET, json_string_value(json_object_get(
            read_json_ob, "router-id")), 
            &cfg->router_id);
  inet_ntop(AF_INET, &cfg->router_id, addr_str ,256);
  //printf("router-id: %s\n", addr_str);
  
  json_int_t jo =json_integer_value(json_object_get(read_json_ob, "as"));
  cfg->as_number = jo;
  /* ここからneighbors  */

  json_t *json_nei;
  json_nei = json_object_get(
             read_json_ob, "neighbors");
  if (json_nei == NULL) {
    printf("neighborsの値を取得することができません \n");
    return -1;  
  }

  int index;
  json_t *json_neiaddr;
  json_array_foreach(json_nei, index, json_neiaddr) {
	  cfg->neighbors[index] = (struct neighbor *)malloc(sizeof(
				  struct neighbor));
          json_t* jo = json_object_get(json_neiaddr, "address"); 
	  inet_pton(AF_INET, json_string_value(jo),
	            &cfg->neighbors[index]->address);
          //inet_ntop(AF_INET, &cfg->neighbors[index]->address, addr_str, 256);
          //printf("neighbor[%d]: %s\n", index, addr_str);
  }

 /* network */
  json_t *json_net;
  json_net = json_object_get(
             read_json_ob, "networks");
  if (json_net == NULL) {
    printf("networkの値を取得することができません \n");
    return -1;  
  }
  char *p;
  int lenp;
  int index_pre;
  char prex_addr_len[256];
  json_t *json_pre;
  char prex_addr[256];

  json_array_foreach(json_net, index_pre, json_pre) {
	  strcpy(prex_addr_len, json_string_value(
				  json_object_get(json_pre, "prefix")));
	  cfg->networks[index_pre] = (struct network *)malloc(sizeof(struct network));
	  /* ここの時点で文字列はCIDR表記となっているため
	     表記の部分から後のアドレスを取り出す */ 
	  p = strrchr(prex_addr_len, '/');

	  /* アドレスから表記の部分まで  */
	  lenp = p - &prex_addr_len[0];

	  /* prex_addrに元の文字列から表記を取り出したもの文字列を取り出す  */
	  strncpy(prex_addr, prex_addr_len, lenp);

	  /* 終端文字入れ  */
	  prex_addr[lenp] = '\0';

	  inet_pton(AF_INET, prex_addr, &cfg->networks[index_pre]->prefix.addr);
	  inet_ntop(AF_INET,  &cfg->networks[index_pre]->prefix.addr, addr_str, 256);
	  cfg->networks[index_pre]->prefix.length = atoi(p + 1);
	  //printf("network[%d]: %s/%d\n", index_pre, addr_str,
	  //cfg->networks[index_pre]->prefix.length);
  }

}
