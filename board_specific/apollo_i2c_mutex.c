#include "apollo_i2c_mutex.h"

apollo_i2c_ret_code_t apollo_i2c3_mutex_lock_impl() {
    uint8_t* p_mutex = &apollo_i2c3_mutex;

    /* Cortex-M exclusive monitor read: Read if the mutex is taken or not. */
    if (APOLLO_I2C3_MUTEX_UNLOCKED == __LDREXB(p_mutex)) {
        /* Take the mutex. */
        if (0 == __STREXB(APOLLO_I2C3_MUTEX_LOCKED, p_mutex)) {
            __DMB(); // Data Memory Barrier
            return APOLLO_I2C3_RET_CODE_OK;
        }
    }

    /* We failed to take the mutex because it was already taken by another process. */
    return APOLLO_I2C3_RET_CODE_BUSY;
}

/*
 * Function implementing the I2C3 mutex locking in a FreeRTOS compatible way. 
 * This function will try to retrieve the mutex for a given period of time 
 * (specified by timeout), and will return BUSY status code if we fail to 
 * get the mutex.
 */
apollo_i2c_ret_code_t apollo_i2c3_mutex_lock(uint32_t timeout) {
    uint32_t const timestart = HAL_GetTick();

    /* 
     * Start a loop where we try to take the mutex. If we cannot take the mutex
     * in the given timeout period, we will return BUSY state.
     */
    while (HAL_GetTick() - timestart < timeout) {
        switch (apollo_i2c3_mutex_lock_impl())
        {
        /* We got the mutex */
        case APOLLO_I2C3_RET_CODE_OK:
            return APOLLO_I2C3_RET_CODE_OK;
        
        /* We failed to get the mutex because it is already taken. */
        case APOLLO_I2C3_RET_CODE_BUSY:
        default:
            taskYIELD();
        }
    }
    /* We timed out and failed to get the mutex. */
    return APOLLO_I2C3_RET_CODE_BUSY;
}

/* Function implementing the release of the I2C3 bus mutex. */
apollo_i2c_ret_code_t apollo_i2c3_mutex_release() {
    uint8_t* p_mutex = &apollo_i2c3_mutex;

    /* Release the mutex by the calling process. */
    *p_mutex = APOLLO_I2C3_MUTEX_UNLOCKED;

    return APOLLO_I2C3_RET_CODE_OK;
}