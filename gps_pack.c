#include "gps_pack.h"
#include "pack.h"

uint32_t pack_gps(uint8_t *buf, gps_data_t *data) {
    return pack(buf, "ddCCCCCCHH",
                data->latitude,
                data->longitude,
                data->day,
                data->month,
                data->year,
                data->hour,
                data->minute,
                data->second,
                data->altitude,
                data->speed);
}

void unpack_gps(uint8_t *buf, gps_data_t *data) {
    double lat = 0.0;
    double lon = 0.0;

    unpack(buf, "ddCCCCCCHH",
            &lat,
            &lon,
           &data->day,
           &data->month,
           &data->year,
           &data->hour,
           &data->minute,
           &data->second,
           &data->altitude,
           &data->speed);
           
    data->latitude = (float)lat;
    data->longitude = (float)lon;

}
