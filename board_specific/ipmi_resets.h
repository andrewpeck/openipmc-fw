#ifndef __IPMI_RESETS_H__
#define __IPMI_RESETS_H__

#include "mt_printf.h"
#include "apollo.h"

/* 
 * Payload resets: Cold, warm and graceful. 
 * These functions implement the completion code for the reset requests. 
 * 
 * They overwrite the passed compl_code to be 0x00, indicating that the requested
 * reset is implemented within the firmware. Please see implementation under
 * openipmc-fw/CM7/Core/Src/openipmc_fru_payload_ipmc_control.c 
 * for details.
 */
void reply_begin_payload_cold_reset(uint8_t* compl_code);
void reply_begin_payload_warm_reset(uint8_t* compl_code);
void reply_begin_payload_graceful_reboot(uint8_t* compl_code);

/* 
 * Functions where the resets are actually implemented. 
 * These implementations override the __weak functions from the OpenIPMC core FW. 
 * 
 * The overwritten function definitions can be found in: 
 * openipmc-fw/CM7/Core/Src/openipmc_fru_payload_ipmc_control.c 
 */
void impl_begin_payload_cold_reset();
void impl_begin_payload_warm_reset();
void impl_begin_payload_graceful_reboot();

#endif