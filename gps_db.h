#ifndef GPS_DB_H_
#define GPS_DB_H_

#include <sqlite3.h>
#include <stdint.h>

int db_init(const char *db_name, const char *table_name, sqlite3 **db);
int db_open(const char *db_name, sqlite3 **db);
void db_close(sqlite3 *db);
int db_create_table(sqlite3 *db, const char *table_name);
int db_delete_table(sqlite3 *db, const char *table_name);
int db_clear_table(sqlite3 *db, const char *table_name);
int db_write_entry(sqlite3 *db, const char *table_name, float latitude, float longitude, uint8_t day, uint8_t month, uint8_t year, 
                    uint8_t hour, uint8_t minute, uint8_t second, uint16_t altitude, uint16_t speed);
int db_read_entry(sqlite3 *db, const char *table_name, int id, float *latitude, float *longitude, uint8_t *day, uint8_t *month, uint8_t *year, 
                    uint8_t *hour, uint8_t *minute, uint8_t *second, uint16_t *altitude, uint16_t *speed);


#endif// GPS_DB_H_