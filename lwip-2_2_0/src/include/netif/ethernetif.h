#ifndef _ETHERNETIF_H_
#define _ETHERNETIF_H_

#include "lwip/netif.h"

extern struct netif ch390_netif;

/**
 * Helper struct to hold private data used to operate your ethernet interface.
 * Keeping the ethernet address of the MAC in this struct is not necessary
 * as it is already kept in the struct netif.
 * But this is only an example, anyway...
 */
struct ethernetif {
//    struct eth_addr *ethaddr;
    /* Add whatever per-interface state that is needed here. */
    uint16_t rx_len;
    uint8_t rx_status;
};

void init_lwip_netif(const ip4_addr_t *ipaddr, const ip4_addr_t *netmask, const ip4_addr_t *gw);
err_t ethernetif_init(struct netif *netif);
void ethernetif_input(struct netif *netif);

void print_netif(struct netif *netif);

#endif /* _ETHERNETIF_H_ */
