#ifndef __NET_H
#define __NET_H

#include <stdint.h>

struct ap_session;

/**
 * Interface para operações de rede
 */
struct net {
    int (*get_ifindex)(const char *ifname);
    void (*release)(struct net *);
    
    /* Outros métodos do net podem ser adicionados conforme necessário */
};

extern struct net *current_net;

#endif
