#ifndef FW_VERSION
#define FW_VERSION

#define MANUFACTURER_ID  35057  // Manufacturer ID (IANA code). 20 bit number. 0x00000 = unspecified. 0xFFFFF = reserved (35057 = UNESP)
#define PRODUCT_ID       1      // Product ID. 0x0000 = unspecified. 0xFFFF = reserved

#define FW_VERSION_MAJOR 1      // Integer 0 ~ 127
#define FW_VERSION_MINOR 0x50   // BCD from 00 to 99 (two digits) Example: Ver. 1.2.3 -> 0x23

#define DEVICE_ID_STRING "APOLLO-OPENIPMC";    // String, 16 characters maximum.

#endif
