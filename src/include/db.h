#ifndef DB_H
#define DB_H

// Include necessary standard headers

#include "standard.h"

// Database management using SQLite3

#include <sqlite3.h>

// Function declarations for database initialization and device management

void init_db();
void add_device(sqlite3 *db, const char *device_ip);
void remove_device(sqlite3 *db, const char *device_id);

void open_db(sqlite3 **db, const char *db_name);
void close_db(sqlite3 *db);

#endif