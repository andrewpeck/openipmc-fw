#ifndef APOLLO_I2C_MUTEX_H
#define APOLLO_I2C_MUTEX_H

#include "apollo.h"
#include "h7i2c_bare.h"

#include "FreeRTOS.h"
#include "task.h"

/* Mutex describing the state of the I2C3 bus. Initiated in board_specific/apollo.c */
extern uint8_t apollo_i2c3_mutex;

/* 
 * Possible states for the I2C3 bus. If a thread is currently in the middle
 * of an I2C transaction on this bus, it locks this bus using the mutex.  
 */
enum {
    APOLLO_I2C3_MUTEX_UNLOCKED = 0, 
    APOLLO_I2C3_MUTEX_LOCKED   = 1
};

/* Return codes for the I2C3 mutex related functions. */
typedef enum {
    APOLLO_I2C3_RET_CODE_OK,
    APOLLO_I2C3_RET_CODE_BUSY
} apollo_i2c_ret_code_t;

/* Functions that implement the mutex lock and release logic. */
apollo_i2c_ret_code_t apollo_i2c3_mutex_lock(uint32_t timeout);
apollo_i2c_ret_code_t apollo_i2c3_mutex_lock_impl();
apollo_i2c_ret_code_t apollo_i2c3_mutex_release();

#endif