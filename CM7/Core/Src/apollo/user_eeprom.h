#ifndef USER_EEPROM_H
#define USER_EEPROM_H

// m24512_we_n

// 16-bit address map

// The Page Write mode allows up to 128 byte to be written in a single
// Write cycle

// Same page: A15/A7, are the same

// If more bytes are sent than will fit up to the end of the page, a
// “roll-over” occurs

// Write Identification Page
//
// - Device type identifier = 1011b MSB
//
// - Address bits A15/A7 are don't care except for address bit A10
// which must be ‘0’.
//
// - LSB address bits A6/A0 define the byte address inside the
// Identification page.

typedef unsigned char uint8_t;

char
user_eeprom_init(void);

char
user_eeprom_read(void);

char
user_eeprom_write(void);

char
user_eeprom_set_version(uint8_t v);

char
user_eeprom_get_version(uint8_t * v);

char
user_eeprom_get_serial_number(uint8_t * sn);

char
user_eeprom_set_serial_number(uint8_t sn);

char
user_eeprom_get_revision_number(uint8_t * rn);

char
user_eeprom_set_revision_number(uint8_t rn);

char
user_eeprom_get_mac_addr(uint8_t eth
                         , uint8_t * mac);

char
user_eeprom_set_mac_addr(uint8_t eth
                         , uint8_t * mac);

char
user_eeprom_write_enable(void);

char
user_eeprom_write_disable(void);

char
user_eeprom_get_wren(uint8_t * lvl);
char user_eeprom_set_boot_mode(uint8_t bm);
char user_eeprom_get_boot_mode(uint8_t *bm);

#endif // USER_EEPROM_H
