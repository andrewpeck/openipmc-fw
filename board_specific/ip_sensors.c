#include "ip_sensors.h"

const linear_sensor_constants_t ipmc_ip_consts = {
    .sensor_type = IPMB0_SENSOR,
    .unit_type = UNSPECIFIED_UNIT,
    .lower_nonrecoverable = 0,
    .lower_noncritical = 0,
    .lower_critical = 0,
    .upper_nonrecoverable = 0,
    .upper_noncritical = 0,
    .upper_critical = 0,
    .m = 1,
    .b = 0,
    .re = 0,
    .be = 0,
};

/*
 * Retrieve the IP address of the IPMC and set the sensor reading data.
 */
sensor_reading_status_t sensor_reading_ipmc_ip_addr(uint8_t byte, sensor_reading_t *sensor_reading) {
    // IP address of the IPMC, being read from the LWIP interface
    extern struct netif gnetif;
    ip4_addr_t* ipaddr  = netif_ip4_addr   ( &gnetif );

    // Set the value and state of the sensor reading
    sensor_reading->raw_value = (ipaddr->addr >> byte*8) & 0xFF;
    sensor_reading->present_state = 0;

    sensor_reading_status_t sensor_status = SENSOR_READING_OK;
    return sensor_status;
}

/*
 * IPMC sensor readings per each byte of the IP address.
 */
sensor_reading_status_t sensor_reading_ipmc_ip_addr_byte0(sensor_reading_t *sensor_reading) {
    return sensor_reading_ipmc_ip_addr(0, sensor_reading);
}

sensor_reading_status_t sensor_reading_ipmc_ip_addr_byte1(sensor_reading_t *sensor_reading) {
    return sensor_reading_ipmc_ip_addr(1, sensor_reading);
}

sensor_reading_status_t sensor_reading_ipmc_ip_addr_byte2(sensor_reading_t *sensor_reading) {
    return sensor_reading_ipmc_ip_addr(2, sensor_reading);
}

sensor_reading_status_t sensor_reading_ipmc_ip_addr_byte3(sensor_reading_t *sensor_reading) {
    return sensor_reading_ipmc_ip_addr(3, sensor_reading);
}