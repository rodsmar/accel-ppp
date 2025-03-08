#ifndef __AP_SESSION_H__
#define __AP_SESSION_H__

#include <stdint.h>
#include <sys/socket.h>
#include <time.h>
#include <linux/rtnetlink.h>
#include <netinet/in.h>

#include "list.h"
#include "triton.h"

/* Define atomic_t */
typedef struct { volatile int counter; } atomic_t;

/* AP_STATE flags */
#define AP_STATE_STARTING     1
#define AP_STATE_ACTIVE       2
#define AP_STATE_FINISHING    4
#define AP_STATE_RESTORE      8

/* AP_CTRL flags */
#define AP_CTRL_STARTED       1
#define AP_CTRL_TERMINATED    2
#define AP_CTRL_FINISH        4

/* Termination causes */
#define TERM_NAS_REQUEST               1
#define TERM_NAS_REBOOT                2
#define TERM_ADMIN_RESET               3
#define TERM_USER_REQUEST              4
#define TERM_SESSION_TIMEOUT           5
#define TERM_IDLE_TIMEOUT              6
#define TERM_ECHO_TIMEOUT              7
#define TERM_MSG_AUTH_FAILED           8
#define TERM_AUTH_ERROR                9
#define TERM_CHAP_TIMEOUT              10
#define TERM_PAP_TIMEOUT               11
#define TERM_CALC_AUTH_FAILED          12
#define TERM_POLICY                    13
#define TERM_PEER_AUTH_FAILED          14
#define TERM_IPv6_CP_NO_CFG            15
#define TERM_IPCPv6_DISABLED           16
#define TERM_KEEPALIVE_TIMEOUT         17
#define TERM_PACKET_TIMEOUT            18
#define TERM_SERVICE_UNAVAILABLE        19
#define TERM_NAS_ERROR                 20

struct ap_session;
struct backup_data;
struct rtnl_link_stats;
struct rtnl_link_stats64;
struct ipv4db_item_t;
struct ipv6db_item_t;
struct net;

struct ap_ctrl {
    struct triton_context_t *ctx;
    struct ap_session *ppp;
    int (*start)(struct ap_session*);
    int (*restart)(struct ap_session*);
    void (*finished)(struct ap_session*);
    int (*terminate)(struct ap_session*, int hard);  /* Alterado para int */
    int (*change_shaper)(struct ap_session*, int down, int up);
    int (*ifcfg)(struct ap_session*);
    int (*update_ipv6_addr)(struct ap_session*);
    int (*delay_close)(struct ap_session*);
    void (*read_stats)(struct ap_session*, struct rtnl_link_stats*);
    int (*mtu)(struct ap_session*, int mtu);
    int (*gw_addr)(struct ap_session*, in_addr_t addr);
    int (*acct_start)(struct ap_session*);
    void (*set_mppe_keys)(struct ap_session*, uint8_t *send_key, int send_key_len, uint8_t *recv_key, int recv_key_len);
};

struct ap_private {
    struct list_head entry;
    void *key;
};

struct ap_session {
    struct list_head entry;
    
    atomic_t refs;
    struct ap_ctrl *ctrl;
    
    int state;
    int terminating:1;
    int terminated:1;
    int down:1;
    int unit_idx;
    
    struct triton_timer_t timer;
    
    unsigned int ifindex;
    char *ifname;
    char *ifname_rename;  /* Alterado de bit-field para ponteiro */
    
    char *hwaddr;
    
    struct sockaddr_in addr_ipv4;
    struct sockaddr_in6 addr_ipv6;
    
    uint64_t rx_bytes;
    uint64_t tx_bytes;
    uint64_t rx_packets;
    uint64_t tx_packets;

    /* Contadores para estatísticas de accounting */
    uint64_t acct_rx_bytes;
    uint64_t acct_tx_bytes;
    uint64_t acct_rx_packets;
    uint64_t acct_tx_packets;
    uint64_t acct_rx_bytes_i;
    uint64_t acct_tx_bytes_i;
    uint64_t acct_rx_packets_i;
    uint64_t acct_tx_packets_i;
    
    int terminate_cause;
    
    char *ipv4_pool_name;
    char *ipv6_pool_name;
    char *dpv6_pool_name;
    
    uint8_t *auth_key;
    int auth_key_len;
    
    char *username;
    char *sessionid;
    char *comp;
    
    time_t start_time;
    time_t idle_time;
    time_t stop_time;
    
    int idle_timeout;
    int session_timeout;
    
    /* Contexto para despertar a thread */
    struct triton_context_t *wakeup;
    
    /* Recursos de rede */
    struct net *net;
    struct ipv4db_item_t *ipv4;
    struct ipv6db_item_t *ipv6;
    
    /* Nome da VRF */
    char *vrf_name;
    
    struct list_head pd_list;
};

struct ap_session_stat {
    int starting;
    int active;
    int finishing;
};

extern struct ap_session_stat ap_session_stat;
extern struct net *default_net;

void ap_session_init(struct ap_session*);
void ap_session_set_ifname(struct ap_session*, const char *ifname);
int ap_session_starting(struct ap_session*);
int ap_session_started(struct ap_session*);
void ap_session_finished(struct ap_session*);
void ap_session_terminate(struct ap_session*, int cause, int hard);
void ap_session_free(struct ap_session*);
int ap_session_read_stats(struct ap_session*, struct rtnl_link_stats64 *stats);
int ap_session_rename(struct ap_session*, const char *ifname, int len);
int ap_session_ifcfg(struct ap_session*);
void ap_session_accounting_started(struct ap_session*);
int ap_session_set_username(struct ap_session*, const char *username);
int ap_session_vrf(struct ap_session*, const char *vrf_name, int len);
void ap_session_ifup(struct ap_session*);
void ap_session_ifdown(struct ap_session*);

void ap_session_mppe_keys(struct ap_session *ses, uint8_t *send_key, int send_key_len, uint8_t *recv_key, int recv_key_len);

int ap_shutdown_soft(void (*cb)(void), int term);

#endif
