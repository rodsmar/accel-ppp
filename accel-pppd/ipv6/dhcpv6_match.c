#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <net/ethernet.h>

#include "log.h"
#include "ppp.h"
#include "ipdb.h"
#include "events.h"
#include "ipv6.h"
#include "ipv6/dhcpv6.h"
#include "ctrl/ipoe/ipoe.h"

// Função para registrar conversões de Client-ID para debug
static void log_client_id(const uint8_t *hwaddr, const struct dhcpv6_option *client_id)
{
    if (!client_id)
        return;
        
    const uint8_t *data = client_id->data;
    int duid_type = ntohs(*(uint16_t *)data);
    
    if (conf_verbose >= 5) {
        int i;
        char str[128] = {0};
        char *ptr;
        
        // MAC address do cliente
        ptr = str;
        for (i = 0; i < ETH_ALEN; i++) {
            sprintf(ptr, "%02x:", hwaddr[i]);
            ptr += 3;
        }
        *(ptr - 1) = 0; // Remove o último ':'
        
        log_ppp_info2("dhcpv6: matching client hwaddr=%s with duid-type=%d\n", str, duid_type);
        
        // DUID do cliente
        ptr = str;
        *ptr = 0;
        for (i = 0; i < client_id->len; i++) {
            sprintf(ptr, "%02x", client_id->data[i]);
            ptr += 2;
        }
        log_ppp_info2("dhcpv6: client DUID=%s\n", str);
    }
}

// Função principal de correspondência entre MAC e DUID
int dhcpv6_match_client_id(struct ap_session *ses, const struct dhcpv6_option *client_id)
{
    if (!client_id || client_id->len < 2)
        return 0;
        
    const uint8_t *data = client_id->data;
    int duid_type = ntohs(*(uint16_t *)data);
    
    // Obter MAC do cliente da sessão IPoE
    const uint8_t *hwaddr = NULL;
    if (ses->ctrl->type == CTRL_TYPE_IPOE) {
        struct ipoe_session *ipoe_ses = container_of(ses, typeof(*ipoe_ses), ses);
        hwaddr = ipoe_ses->hwaddr;
        log_client_id(hwaddr, client_id);
    }
    
    if (!hwaddr)
        return 0;
       
    // Correspondência de DUID baseado no tipo
    switch (duid_type) {
    case 1: // DUID-LLT (Link-layer address plus time)
        // Tipo 1 octet + Time 4 octets + Hardware Type 2 octets + Link Layer Address 6 octets
        if (client_id->len >= 9 && ntohs(*(uint16_t *)(data + 6)) == 1) // Hardware type 1 = Ethernet
            return memcmp(data + 8, hwaddr, ETH_ALEN) == 0;
        break;
        
    case 3: // DUID-LL (Link-layer address)
        // Tipo 2 octets + Hardware Type 2 octets + Link Layer Address 6 octets
        if (client_id->len >= 4 && ntohs(*(uint16_t *)(data + 2)) == 1) // Hardware type 1 = Ethernet
            return memcmp(data + 4, hwaddr, ETH_ALEN) == 0;
        break;
    
    case 2: // DUID-EN (Enterprise Number)
        // Este tipo não contém diretamente o MAC, então verificamos se termina com MAC
        if (client_id->len >= (ETH_ALEN + 2)) {
            // Último ETH_ALEN bytes podem ser MAC
            if (memcmp(data + (client_id->len - ETH_ALEN), hwaddr, ETH_ALEN) == 0)
                return 1;
        }
        break;
    }
    
    // Se chegamos aqui, não encontramos correspondência
    // Último recurso: procurar o MAC em qualquer lugar na DUID
    for (int i = 0; i <= client_id->len - ETH_ALEN; i++) {
        if (memcmp(data + i, hwaddr, ETH_ALEN) == 0) {
            if (conf_verbose >= 5)
                log_ppp_info2("dhcpv6: found MAC at offset %d in DUID\n", i);
            return 1;
        }
    }
    
    return 0;
}
