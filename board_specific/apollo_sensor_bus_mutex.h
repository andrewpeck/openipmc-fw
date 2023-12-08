/**
 * apollo_sensor_bus_mutex.c and apollo_sensor_bus_mutex.h implement the logic to take
 * and release the mutex for the I2C transactions on the sensor bus (I2C3).
 * 
 * This mutex will be taken for each transaction where there is a write to the multiplexer,
 * followed by a read/write. At the end of these two I2C transactions, the mutex is released.
 * The aim here is to lock the sensor bus during these two transactions so that no other task
 * can get in between and use the same bus. 
 */

#ifndef APOLLO_SENSOR_BUS_MUTEX_H
#define APOLLO_SENSOR_BUS_MUTEX_H

#include "apollo.h"
#include "h7i2c_bare.h"

#include "FreeRTOS.h"
#include "task.h"

/* Mutex describing the state of the sensor bus. Initiated in board_specific/apollo.c */
extern uint8_t apollo_sensor_bus_mutex;

/* 
 * Possible states for the sensor bus mutex. If a thread is currently in the middle
 * of an I2C transaction on this bus, it locks this bus using the mutex.  
 */
enum {
    APOLLO_SENSOR_BUS_MUTEX_UNLOCKED = 0, 
    APOLLO_SENSOR_BUS_MUTEX_LOCKED   = 1
};

/* Return codes for the sensor bus mutex related functions. */
typedef enum {
    APOLLO_SENSOR_BUS_RET_CODE_OK,
    APOLLO_SENSOR_BUS_RET_CODE_BUSY
} apollo_i2c_ret_code_t;

/* Functions that implement the sensor bus mutex lock and release logic. */
apollo_i2c_ret_code_t apollo_sensor_bus_mutex_lock(uint32_t timeout);
apollo_i2c_ret_code_t apollo_sensor_bus_mutex_lock_impl();
apollo_i2c_ret_code_t apollo_sensor_bus_mutex_release();

#endif