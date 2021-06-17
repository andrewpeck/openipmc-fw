#include <apollo.h>
#include <mgm_i2c.h>
#include <stdint.h>
#include <string.h>
#include <user_eeprom.h>

#define MEM_ADDR 0x50<<1
#define ID_PAGE 0x58

char user_eeprom_get_wren(uint8_t * lvl);
char user_eeprom_write_enable(void);
char user_eeprom_write_disable(void);

static struct __attribute__((packed)) {
  uint8_t version;
  union {
    struct __attribute__((packed)) {
      uint8_t serial_number;
      uint8_t revision_number;
      uint8_t mac_eth0[6];
      uint8_t mac_eth1[6];
      uint8_t boot_mode;
    } v0;
  };
} eeprom = {.version = 0xFE};

char user_eeprom_init(void) { return user_eeprom_read(); }

char user_eeprom_set_version(uint8_t v) {
  eeprom.version = v;
  return 0;
}

char user_eeprom_get_version(uint8_t *v) {
  *v = eeprom.version;
  return 0;
}

char user_eeprom_read(void) {

  uint8_t addr[] = {0x00, 0x00};

  HAL_StatusTypeDef status = mgm_i2c_transmit(MEM_ADDR, addr, 2, 100);

  if (status != HAL_OK) {
    return -1;
  }

  status = mgm_i2c_receive(MEM_ADDR, (uint8_t *)&eeprom, sizeof(eeprom), 1000);

  if (status != HAL_OK) {
    return -2;
  }

  return 0;
}

char user_eeprom_write(void) {

  if (sizeof(eeprom) > 128) {
    return -2;
  }

  static uint8_t buffer[2 + sizeof(eeprom)];

  buffer[0] = 0x00;
  buffer[1] = 0x00;
  memcpy(&buffer[2], (uint8_t *)&eeprom, sizeof(eeprom));

  user_eeprom_write_enable();
  HAL_StatusTypeDef status;
  status = mgm_i2c_transmit(MEM_ADDR, buffer, sizeof(buffer), 1000);
  user_eeprom_write_disable();
  if (status != HAL_OK) {
    return -1;
  }

  return 0;
}

char user_eeprom_get_serial_number(uint8_t *sn) {
  if (eeprom.version == 0) {
    *(sn) = eeprom.v0.serial_number;
    return 0;
  } 
  return -1;
}

char user_eeprom_set_serial_number(uint8_t sn) {
  if (eeprom.version == 0) {
    eeprom.v0.serial_number = sn;
    return 0;
  } 
  return -1;
}

char user_eeprom_get_boot_mode(uint8_t *bm) {
  if (eeprom.version == 0) {
    *(bm) = eeprom.v0.boot_mode;
    return 0;
  } 
  return -1;
}

char user_eeprom_set_boot_mode(uint8_t bm) {
  if (eeprom.version == 0) {
    eeprom.v0.boot_mode = bm;
    return 0;
  }
  return -1;
}

char user_eeprom_get_revision_number(uint8_t *rn) {
  if (eeprom.version == 0) {
    *(rn) = eeprom.v0.revision_number;
    return 0;
  } 
  return -1;
}

char user_eeprom_set_revision_number(uint8_t rn) {
  if (eeprom.version == 0) {
    eeprom.v0.revision_number = rn;
    return 0;
  }
  return -1;
}

char user_eeprom_get_mac_addr(uint8_t eth, uint8_t *mac) {

  if (eeprom.version == 0) {
    if (eth == 0) {
      for (int i = 0; i < 6; i++) {
        mac[i] = eeprom.v0.mac_eth0[i];
      }
      return 0;
    } else if (eth == 1) {
      for (int i = 0; i < 6; i++) {
        mac[i] = eeprom.v0.mac_eth1[i];
      }
      return 0;
    } 
  }

  return -1;
}

char user_eeprom_set_mac_addr(uint8_t eth, uint8_t *mac) {

  if (eeprom.version == 0) {

    if (eth == 0) {
      for (int i = 0; i < 6; i++) {
        eeprom.v0.mac_eth0[i] = mac[i];
      }
      return 0;

    } else if (eth == 1) {
      for (int i = 0; i < 6; i++) {
        eeprom.v0.mac_eth1[i] = mac[i];
      }
      return 0;
    }

  }
  return -1;
}

char user_eeprom_write_enable(void) {
  apollo_set_eeprom_we_n(0);
  return 0;
}

char user_eeprom_write_disable(void) {
  apollo_set_eeprom_we_n(1);
  return 0;
}
