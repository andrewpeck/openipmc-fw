#include "board_and_product_info.h"
#include "printf.h"

// These headers are generated by Eclipse or Make before compiling. See header_gen.sh file.
#include "head_commit_sha1.h"
#include "compiling_epoch.h"

// apollo headers
#include "apollo.h"
#include "user_eeprom.h"

/*
 * board_mfg_date_time: Number of minutes from 0:00 hrs 1/1/96
 * 
 * String fields: max of 63 ASCII chars
 */
static uint32_t board_mfg_date_time = COMPILING_EPOCH/60 - 13674240;  // Manufacturing Date/Time (Here is being inserted the compiling time in GMT)
static char*    board_manufacturer  = "BOSTON-UNIVERSITY";
static char*    board_product_name  = "APOLLO-BLADE";
static char*    board_fru_file_id   = "file.xml";

static char* product_manufacturer      = "SPRACE-KIT";
static char* product_name              = "OpenIPMC-HW";
static char* product_part_model_number = "123456AB";
static char* product_version           = "2.0";
static char* product_fru_file_id       = "file.xml";


/*
 * This function is called during the OpenIPMC initialization in order to get
 * the strings used to build "Board Info" and "Product Info" fields in the
 * FRU Info data.
 * 
 * FRU Info is mandatory, therefore this function must be defined in the 
 * board-specific project.
 * 
 * NOTE: The content of all these strings are COPIED into the FRU Info when it
 * is being created during OpenIPMC initialization. Therefore, changes in the
 * content of these strings will not be reflected into the FRU Info content.
 */
void get_product_and_board_info( board_and_product_info_t* info )
{
  uint8_t sn = 0;
  if (0==user_eeprom_read()) {
    user_eeprom_get_serial_number(&sn);
  }

  char sm_sn [9];
  snprintf(sm_sn, 9, "SM%05d", sn);

  char sm_rev [9] = "REV00000";
  uint8_t rev = apollo_get_revision();

  if (rev==APOLLO_REV1)
    snprintf(sm_rev, 9, "REV00001");
  else if (rev==APOLLO_REV2)
    snprintf(sm_rev, 9, "REV00002");
  else if (rev==APOLLO_REV2A)
    snprintf(sm_rev, 9, "REV0002A");

  unsigned int id = HAL_GetUIDw0() + HAL_GetUIDw1() + HAL_GetUIDw2();

  char ipmc_id [9];
  snprintf(ipmc_id, 9, "%08X", id);

  char ipmc_sha [9];
  snprintf(ipmc_sha, 9, "%08X", HEAD_COMMIT_SHA1);

  info->board_mfg_date_time         =  board_mfg_date_time;
  info->board_manufacturer          =  board_manufacturer;
  info->board_product_name          =  board_product_name;
  info->board_serial_number         =  sm_sn,
  info->board_part_number           =  sm_rev,
  info->board_fru_file_id           =  board_fru_file_id;

  info->product_manufacturer        =  product_manufacturer;
  info->product_name                =  product_name;
  info->product_part_model_number   =  product_part_model_number;
  info->product_version             =  product_version;
  info->product_serial_number       =  ipmc_id,
  info->product_asset_tag           =  ipmc_sha,
  info->product_fru_file_id         =  product_fru_file_id;
}
