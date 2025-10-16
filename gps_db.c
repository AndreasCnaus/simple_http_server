#include "gps_db.h"
#include <stdio.h>


int db_init(const char *db_name, const char *table_name, sqlite3 **db)
{
    
    int rc = db_open(db_name, db);
    if (rc) return rc;

    rc = db_create_table(*db, table_name);
    if (rc) return rc;

    return SQLITE_OK;
}

int db_open(const char *database_name, sqlite3 **db)
{
    int rc = sqlite3_open(database_name, db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Could not open database '%s': %s\n", database_name, sqlite3_errmsg(*db));
        sqlite3_close(*db);
        return rc;
    }

    printf("Database: '%s' successfully opened\n", database_name);
    return SQLITE_OK;
}

void db_close(sqlite3 *db)
{
    sqlite3_close(db);
}

int db_create_table(sqlite3 *db, const char *table_name)
{
    char *err_msg = NULL;
    char sql[256];

     // Build the CREATE TABLE SQL statement 
    snprintf(sql, sizeof(sql), "CREATE TABLE IF NOT EXISTS %s ("
                    "id INTEGER PRIMARY KEY AUTOINCREMENT," 
                    "latitude REAL, "
                    "longitude REAL, "
                    "day INTEGER, "
                    "month INTEGER, "
                    "year INTEGER, "
                    "hour INTEGER, "
                    "minute INTEGER, "
                    "second INTEGER, "
                    "altitude INTEGER, "
                    "speed INTEGER" 
                    ");", table_name); 
                    
    // Execute SQL 
    int rc = sqlite3_exec(db, sql, 0, 0, &err_msg); 
    if (rc != SQLITE_OK) { 
        fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg); 
        return rc;
    } else { 
        printf("Database table: '%s' created or already exists\n", table_name); 
    }

    return SQLITE_OK;
}

int db_delete_table(sqlite3 *db, const char *table_name)
{
    char *error_msg = NULL; 
    char sql[128];

    // Build the DROP TABLE SQL statement 
    snprintf(sql, sizeof(sql), "DROP TABLE IF EXISTS %s;", table_name);

    // Execute the SQL
    int rc = sqlite3_exec(db, sql, 0, 0, &error_msg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to drop table '%s': %s.\n", table_name, error_msg);
        sqlite3_free(error_msg);
        return rc;
    }

    printf("Table '%s' dropped successfully (if it existed).\n", table_name);
    return SQLITE_OK;
}

int db_clear_table(sqlite3 *db, const char *table_name)
{
    // ToDo: 
    return 0;
}

int db_write_entry(sqlite3 *db, const char *table_name, float latitude, float longitude,
                uint8_t day, uint8_t month, uint8_t year, 
                uint8_t hour, uint8_t minute, uint8_t second,
                uint16_t altitude, uint16_t speed)
{
    char *error_msg = NULL;
    char sql[256];

    // Build INSERT SQL statement
    snprintf(sql, sizeof(sql), "INSERT INTO %s ("
            "latitude, longitude, day, month, year,"
            "hour, minute, second," 
            "altitude, speed)"
            "VALUES (%f, %f, %d, %d, %d, %d, %d, %d, %d, %d);", 
            table_name, latitude, longitude, day, month, year, hour, minute, second, altitude, speed);

    // Execute the SQL
    int rc = sqlite3_exec(db, sql, 0, 0, &error_msg);
    if (rc != SQLITE_OK) {
        // fprintf(stderr, "Could not insert GPS data into the '%s': %s.\n", table_name, error_msg);
        sqlite3_free(error_msg);
        return rc;
    }

    // printf("New GPS data were successfully inserted into %s.\n", table_name);
    return SQLITE_OK;
}

int db_read_entry(sqlite3 *db, const char *table_name, int id, float *latitude, float *longitude,
            uint8_t *day, uint8_t *month, uint8_t *year, 
            uint8_t *hour, uint8_t *minute, uint8_t *second,
            uint16_t *altitude, uint16_t *speed)
{
    sqlite3_stmt *stmt;
    char sql[256];

    // Build the SELECT SQL statement
    snprintf(sql, sizeof(sql), 
                    "SELECT latitude, longitude,"
                    "day, month, year,"
                    "hour, minute, second,"
                    "altitude, speed "
                    "FROM %s WHERE id = %d;",
                    table_name, id);
    
    // Prepare SQL statement
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare SELECT statement: %s\n", sqlite3_errmsg(db));
        return rc;
    }

    // Execute and read the row
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        *latitude = sqlite3_column_double(stmt, 0);
        *longitude = sqlite3_column_double(stmt, 1);
        *day = sqlite3_column_int(stmt, 2);
        *month = sqlite3_column_int(stmt, 3);
        *year = sqlite3_column_int(stmt, 4);
        *hour = sqlite3_column_int(stmt, 5);
        *minute = sqlite3_column_int(stmt, 6);
        *second = sqlite3_column_int(stmt, 7);
        *altitude = sqlite3_column_int(stmt, 8);
        *speed = sqlite3_column_int(stmt, 9);
    } else {
        fprintf(stderr, "No entry found with id = %d\n", id);
        sqlite3_finalize(stmt);
        return SQLITE_ERROR;
    }


    sqlite3_finalize(stmt);
    printf("Entry with id = %d successfully read from table '%s'.\n", id, table_name);
    return SQLITE_OK;
}
