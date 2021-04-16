
#include "lwip.h"

#include "network_ctrls.h"

// These variables are globally defined in lwip.c file.
extern struct netif gnetif;
//extern ip4_addr_t ipaddr;
//extern ip4_addr_t netmask;
//extern ip4_addr_t gw;



/*
 * This function is called by the "low_level_init()" functions in ethernetif.c file.
 *
 * It is responsible for defining the MAC Address before initialize the interface.
 *
 * It overrides the MAC Address defined in the IOC file via STM32CubeIDE graphical interface.
 */
void load_user_defined_mac_addr( uint8_t mac_addr[] )
{
	// Load the hard coded user defined MAC address
 	//mac_addr[0] = ((uint64_t)USER_MAC_ADDRESS >> 40)&0xFF;
	//mac_addr[1] = ((uint64_t)USER_MAC_ADDRESS >> 32)&0xFF;
	//mac_addr[2] = ((uint64_t)USER_MAC_ADDRESS >> 24)&0xFF;
	//mac_addr[3] = ((uint64_t)USER_MAC_ADDRESS >> 16)&0xFF;
	//mac_addr[4] = ((uint64_t)USER_MAC_ADDRESS >> 8 )&0xFF;
	//mac_addr[5] = ((uint64_t)USER_MAC_ADDRESS >> 0 )&0xFF;

	// STMicroelectronics Prefix (00:80:E1:xx:xx:xx) + 24 LSB from the device Unique ID
	uint32_t id = HAL_GetUIDw0() + HAL_GetUIDw1() + HAL_GetUIDw2();
	mac_addr[0] = 0x00;
	mac_addr[1] = 0x80;
	mac_addr[2] = 0xE1;
	mac_addr[3] = (id >> 16)&0xFF;
	mac_addr[4] = (id >> 8 )&0xFF;
	mac_addr[5] = (id >> 0 )&0xFF;
}



/*
 * Function to change the IP address "on the fly" (after interface is already initialized
 * with some default IP address)
 */
void eth_ctrls_change_ip_addr( uint8_t ip0,   uint8_t ip1,   uint8_t ip2,   uint8_t ip3,
                               uint8_t mask0, uint8_t mask1, uint8_t mask2, uint8_t mask3,
                               uint8_t gw0,   uint8_t gw1,   uint8_t gw2,   uint8_t gw3    )
{
	ip4_addr_t ipaddr;
	ip4_addr_t netmask;
	ip4_addr_t gw;

	IP4_ADDR( &ipaddr,  ip0,  ip1,    ip2,   ip3   );
	IP4_ADDR( &netmask, mask0, mask1, mask2, mask3 );
	IP4_ADDR( &gw,      gw0,   gw1,   gw2,   gw3   );

	netif_set_addr( &gnetif, &ipaddr, &netmask, &gw);
}
