#ifndef GPS_PACK_H_
#define GPS_PACK_H_

#include "gps_pack.h"
#include "pack.h" // your existing varargs-based pack/unpack functions
#include <stdint.h>

#define GPS_PACKET_SIZE     18

typedef struct {
    float latitude;
    float longitude;
    uint8_t day;
    uint8_t month;
    uint8_t year;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    uint16_t altitude;
    uint16_t speed;
} gps_data_t;

uint32_t pack_gps(uint8_t *buf, gps_data_t *data);
void unpack_gps(uint8_t *buf, gps_data_t *data);

#endif