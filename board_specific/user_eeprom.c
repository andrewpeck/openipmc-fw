#include <apollo.h>
#include <stdint.h>
#include <string.h>
#include <user_eeprom.h>
#include <apollo_i2c.h>

#include <h7i2c_bare.h>
#include <h7i2c_rtos.h>

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
      uint8_t dis_shutoff;
      uint8_t sdsel;
    } v0;
    struct __attribute__((packed)) {
      uint8_t serial_number;
      uint8_t revision_number;
      uint8_t mac_eth0[6];
      uint8_t mac_eth1[6];
      uint8_t boot_mode;
      uint8_t dis_shutoff;
      uint8_t sdsel;
      uint8_t mac_eth0_checksum;
      uint8_t mac_eth1_checksum;
    } v1;
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

  h7i2c_i2c_ret_code_t status = h7i2c_i2c_clear_error_state_and_write(H7I2C_I2C4, MEM_ADDR, 2, addr, 100);

  if (status != H7I2C_RET_CODE_OK) {
    return -1;
  }

  status = h7i2c_i2c_clear_error_state_and_read(H7I2C_I2C4, MEM_ADDR, sizeof(eeprom), (uint8_t *)&eeprom, 1000);

  if (status != H7I2C_RET_CODE_OK) {
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
  h7i2c_i2c_ret_code_t status;
  status = h7i2c_i2c_clear_error_state_and_write(H7I2C_I2C4, MEM_ADDR, sizeof(buffer), buffer, 1000);
  user_eeprom_write_disable();
  
  if (status != H7I2C_RET_CODE_OK) {
    return -1;
  }

  return 0;
}

char user_eeprom_get_serial_number(uint8_t *sn) {
  char ret = -1;
  switch (eeprom.version) {
    case 1:
    case 0:
      *(sn) = eeprom.v0.serial_number;
      ret = 0;
      break;
    
    default:
      break;
  }
  return ret;
}

char user_eeprom_set_serial_number(uint8_t sn) {
  char ret = -1;
  switch (eeprom.version) {
    case 1:
    case 0:
      eeprom.v0.serial_number = sn;
      ret = 0;
      break;
    
    default:
      break;
  }
  return ret;
}

char user_eeprom_get_mac_eth_checksum(uint8_t eth, uint8_t* checksum) {
  char ret = -1;
  switch (eeprom.version) {
    case 1:
      if (eth == 0) {
        *(checksum) = eeprom.v1.mac_eth0_checksum;
        ret = 0;
        break;
      }
      else if (eth == 1) {
        *(checksum) = eeprom.v1.mac_eth1_checksum;
        ret = 0;
        break;
      }
      // ETH can be either 0 or 1
      else {
        break;
      }
    
    // Checksum entries do not exist on EEPROM revision 0
    case 0:
    default:
      break;
  }
  return ret;
}

char user_eeprom_set_mac_eth_checksum(uint8_t eth, uint8_t checksum) {
  char ret = -1;
  switch (eeprom.version) {
    case 1:
      if (eth == 0) {
        eeprom.v1.mac_eth0_checksum = checksum;
        ret = 0;
        break;
      }
      else if (eth == 1) {
        eeprom.v1.mac_eth1_checksum = checksum;
        ret = 0;
        break;
      }
      // ETH can be either 0 or 1
      else {
        break;
      }
    
    // Checksum entries do not exist on EEPROM revision 0
    case 0:
    default:
      break;
  }
  return ret;
}

char user_eeprom_get_boot_mode(uint8_t *bm) {
  char ret = -1;
  switch (eeprom.version) {
    case 1:
    case 0:
      *(bm) = eeprom.v0.boot_mode;
      ret = 0;
      break;
    
    default:
      break;
  }
  return ret;
}

char user_eeprom_set_boot_mode(uint8_t bm) {
  char ret = -1;
  switch (eeprom.version) {
    case 1:
    case 0:
      eeprom.v0.boot_mode = bm;
      ret = 0;
      break;
    
    default:
      break;
  }
  return ret;
}

char user_eeprom_get_sdsel(uint8_t *ds) {
  char ret = -1;
  switch (eeprom.version) {
    case 1:
    case 0:
      *(ds) = eeprom.v0.sdsel;
      ret = 0;
      break;
    
    default:
      break;
  }
  return ret;
}

char user_eeprom_set_sdsel(uint8_t sdsel) {
  char ret = -1;
  switch (eeprom.version) {
    case 1:
    case 0:
      eeprom.v0.sdsel = sdsel;
      ret = 0;
      break;

    default:
      break;
  }
  return ret;
}

char user_eeprom_get_disable_shutoff(uint8_t *ds) {
  char ret = -1;
  switch (eeprom.version) {
    case 1:
    case 0:
      *(ds) = eeprom.v0.dis_shutoff;
      ret = 0;
      break;
    
    default:
      break;
  }
  return ret;
}

char user_eeprom_set_disable_shutoff(uint8_t ds) {
  char ret = -1;
  switch (eeprom.version) {
    case 1:
    case 0:
      eeprom.v0.dis_shutoff = ds;
      ret = 0;
      break;
    
    default:
      break;
  }
  return ret;
}

char user_eeprom_get_revision_number(uint8_t *rn) {
  char ret = -1;
  switch (eeprom.version) {
    case 1:
    case 0:
      *(rn) = eeprom.v0.revision_number;
      ret = 0;
      break;
    
    default:
      break;
  }
  return ret;
}

char user_eeprom_set_revision_number(uint8_t rn) {
  char ret = -1;
  switch (eeprom.version) {
    case 1:
    case 0:
      eeprom.v0.revision_number = rn;
      ret = 0;
      break;
    
    default:
      break;
  }
  return ret;
}

char user_eeprom_get_mac_addr(uint8_t eth, uint8_t *mac) {
  char ret = -1;
  switch (eeprom.version) {
    case 1:
    case 0:
      if (eth == 0) {
        for (int i = 0; i < 6; i++) {
          mac[i] = eeprom.v0.mac_eth0[i];
        }
        ret = 0;
        break;
      } 
      else if (eth == 1) {
        for (int i = 0; i < 6; i++) {
          mac[i] = eeprom.v0.mac_eth1[i];
        }
        ret = 0;
        break;
      }
      // ETH can be either 0 or 1
      else {
        break;
      }
    
    default:
      break;
  }
  return ret;
}

char user_eeprom_set_mac_addr(uint8_t eth, uint8_t *mac) {
  char ret = -1;
  switch (eeprom.version) {
    case 1:
    case 0:
      if (eth == 0) {
        for (int i = 0; i < 6; i++) {
          eeprom.v0.mac_eth0[i] = mac[i];
        }
        ret = 0;
        break;
      } 
      else if (eth == 1) {
        for (int i = 0; i < 6; i++) {
          eeprom.v0.mac_eth1[i] = mac[i];
        }
        ret = 0;
        break;
      }
      // ETH can be either 0 or 1
      else {
        break;
      }
    
    default:
      break;
  }
  return ret;
}

char user_eeprom_write_enable(void) {
  apollo_set_eeprom_we_n(0);
  return 0;
}

char user_eeprom_write_disable(void) {
  apollo_set_eeprom_we_n(1);
  return 0;
}
