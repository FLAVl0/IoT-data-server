#include "../include/db.h"

void init_db()
{
	sqlite3 *database;

	if (sqlite3_open("data.db", &database) == SQLITE_OK)
	{
		char *sql, *err_msg = 0;

		// Create the basic tables if they do not exist

		// Table for registered IoT devices
		sql =
			"CREATE TABLE IF NOT EXISTS devices ("
			"device_ip TEXT NOT NULL,"
			"device_id TEXT PRIMARY KEY"
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

		// Table for data from IoT devices which includes:
		// device_id (foreign key)
		// temperature (REAL)
		// humidity (REAL)
		// timestamp (DATETIME DEFAULT CURRENT_TIMESTAMP)

		sql =
			"CREATE TABLE IF NOT EXISTS data ("
			"device_id TEXT,"
			"temperature REAL,"
			"humidity REAL,"
			"ts DATETIME DEFAULT CURRENT_TIMESTAMP,"
			"FOREIGN KEY(device_id) REFERENCES devices(device_id)"
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

void add_device(sqlite3 *db, const char *device_ip)
{
	char *err_msg = 0;
	char *sql;

	// Insert a new device with a unique device_id
	uuid_t binuuid;
	uuid_generate(binuuid);
	char device_id[37];
	uuid_unparse(binuuid, device_id);

	sql = sqlite3_mprintf(
		"INSERT INTO devices (device_ip, device_id) "
		"VALUES ('%q', '%q');",
		device_ip, device_id);
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

void open_db(sqlite3 **db, const char *db_name)
{
	if (sqlite3_open(db_name, db) != SQLITE_OK)
	{
		fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(*db));
		sqlite3_close(*db);
		*db = NULL;
	}
}

void close_db(sqlite3 *db)
{
	if (db)
	{
		sqlite3_close(db);
	}
}
