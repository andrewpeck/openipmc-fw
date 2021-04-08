#ifndef ETH_CTRLS_H
#define ETH_CTRLS_H

#include "main.h"


// User defined MAC Address.
// This definition overrides the address defined in the IOC file.
//#define USER_MAC_ADDRESS 0x0080E1000001UL


/*
 * This function is called by the "low_level_init()" functions in ethernetif.c file.
 *
 * It is responsible for defining the MAC Address before initialize the interface.
 *
 * It overrides the MAC Address defined in the IOC file via STM32CubeIDE graphical interface.
 */
void load_user_defined_mac_addr( uint8_t mac_addr[] );

/*
 * Function to change the IP address "on the fly" (after interface is already initialized
 * with some default IP address)
 */
void eth_ctrls_change_ip_addr( uint8_t ip0,   uint8_t ip1,   uint8_t ip2,   uint8_t ip3,
                               uint8_t mask0, uint8_t mask1, uint8_t mask2, uint8_t mask3,
                               uint8_t gw0,   uint8_t gw1,   uint8_t gw2,   uint8_t gw3    );


#endif // ETH_CTRLS_H
