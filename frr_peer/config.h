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
};

struct neighbor {
  struct in_addr address;
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
};

static inline void print_config(const struct config* cfg) 
{
  char addr_str[256];
  int as_number[256];
  inet_ntop(AF_INET, &cfg->router_id, addr_str, sizeof(addr_str));
  printf("router-id: %s\n", addr_str);
  printf("%d", cfg->as_number);
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
}
