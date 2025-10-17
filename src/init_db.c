#include "server.h"

void init_db()
{
	sqlite3 *database;

	if (sqlite3_open("chat.db", &database) == SQLITE_OK)
	{
		const char *sql =
			"CREATE TABLE IF NOT EXISTS messages ("
			"id INTEGER PRIMARY KEY AUTOINCREMENT,"
			"user TEXT,"
			"text TEXT,"
			"device_id TEXT,"
			"ts DATETIME DEFAULT CURRENT_TIMESTAMP"
			");";
		sqlite3_exec(database, sql, 0, 0, 0);

		const char *create_users =
			"CREATE TABLE IF NOT EXISTS users ("
			"device_id TEXT PRIMARY KEY,"
			"username  TEXT NOT NULL"
			");";
		sqlite3_exec(database, create_users, NULL, NULL, NULL);

		sqlite3_close(database);
	}
}