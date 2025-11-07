#include "../include/db.h"

void init_db()
{
	sqlite3 *database;

	if (sqlite3_open("data.db", &database) == SQLITE_OK)
	{
		char *sql, *err_msg = 0;

		// - - - Create the basic tables if they do not exist - - - //

		// Table for registered IoT devices

		// device_id (TEXT PRIMARY KEY) - Unique identifier for each device
		// data_id (INTEGER) - Identifier for the type of data sent by the device
		// data_designation (TEXT) - Description of the data type
		sql =
			"CREATE TABLE IF NOT EXISTS devices ("
			"device_id TEXT PRIMARY KEY,"
			"data_id INTEGER,"
			"data_designation TEXT"
			");";
		if (sqlite3_exec(database, sql, 0, 0, &err_msg) != SQLITE_OK)
		{
			fprintf(stderr, "SQL error: %s\n", err_msg);
			if (err_msg)
			{
				sqlite3_free(err_msg);
			}
			err_msg = 0;
		}

		// Table for data

		// timestamp (DATETIME DEFAULT CURRENT_TIMESTAMP)
		// device_id (foreign key)
		// temperature (REAL)
		// humidity (REAL)
		sql =
			"CREATE TABLE IF NOT EXISTS data ("
			"ts DATETIME DEFAULT CURRENT_TIMESTAMP,"
			"device_id TEXT PRIMARY KEY,"
			"data_id INTEGER,"
			"value REAL"
			");";
		if (sqlite3_exec(database, sql, 0, 0, &err_msg) != SQLITE_OK)
		{
			fprintf(stderr, "SQL error: %s\n", err_msg);
			if (err_msg)
			{
				sqlite3_free(err_msg);
			}
			err_msg = 0;
		}
	}

	sqlite3_close(database);
}

/*
 * Function to add a new device to the database
 * Parameters:
 *  - db: Pointer to the SQLite3 database connection
 * 	- device_id: Unique identifier for the device
*/
void add_device(sqlite3 *db, const char *device_id)
{
	char *err_msg = 0;
	char *sql;

	sql = sqlite3_mprintf(
		"INSERT INTO devices (device_id) "
		"VALUES ('%q');",
		device_id);
	if (sql == NULL)
	{
		fprintf(stderr, "Failed to allocate memory for SQL statement\n");
		return;
	}

	if (sqlite3_exec(db, sql, 0, 0, &err_msg) != SQLITE_OK)
	{
		fprintf(stderr, "SQL error: %s\n", err_msg);
		if (err_msg)
		{
			sqlite3_free(err_msg);
		}
	}

	sqlite3_free(sql);
}

/*
 * Function to remove a device from the database
 * Parameters:
 *  - db: Pointer to the SQLite3 database connection
 * 	- device_id: Unique identifier for the device
*/
void remove_device(sqlite3 *db, const char *device_id)
{
	char *err_msg = 0;
	char *sql;

	// Delete device from devices table
	sql = sqlite3_mprintf(
		"DELETE FROM devices WHERE device_id = '%q';",
		device_id);
	if (sql == NULL)
	{
		fprintf(stderr, "Failed to allocate memory for SQL statement\n");
		return;
	}

	if (sqlite3_exec(db, sql, 0, 0, &err_msg) != SQLITE_OK)
	{
		fprintf(stderr, "SQL error: %s\n", err_msg);
		if (err_msg)
		{
			sqlite3_free(err_msg);
		}
	}

	sqlite3_free(sql);
}

/*
 * Function to open a connection to the database
 * Parameters:
 *  - db: Pointer to the SQLite3 database connection pointer
 * 	- db_name: Name of the database file
*/
void open_db(sqlite3 **db, const char *db_name)
{
	if (sqlite3_open(db_name, db) != SQLITE_OK)
	{
		fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(*db));
		sqlite3_close(*db);
		*db = NULL;
	}
}

/*
 * Function to close the connection to the database
 * Parameters:
 *  - db: Pointer to the SQLite3 database connection
*/
void close_db(sqlite3 *db)
{
	if (db)
	{
		sqlite3_close(db);
	}
}
