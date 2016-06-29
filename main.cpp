#include <iostream>
#include <string>

#include "sqlite3.h"

#include "utility.h"

static const char* const db_name = "test.db";
static const size_t col_width = 12;

static const char* const prepare_sql[] = {
	u8"CREATE TABLE tbl_example (id INTEGER PRIMARY KEY AUTOINCREMENT, text TEXT);",
	u8"INSERT INTO tbl_example(text) VALUES('a');",
	u8"INSERT INTO tbl_example(text) VALUES('あ');",
	u8"INSERT INTO tbl_example(text) VALUES('ん');",
	u8"INSERT INTO tbl_example(text) VALUES('朝');",
	u8"INSERT INTO tbl_example(text) VALUES('淺');",
};
static const char* const select_sql = "SELECT * FROM tbl_example order by text;";

static char *sqlite_error = nullptr;

void print_table_results(sqlite3* db) {
	int ret = 0;
	int rows, columns;
	char **results = nullptr;

	try {
		print("select query=[", (const char*)select_sql, "]");
		ret = sqlite3_get_table(db, select_sql, &results, &rows, &columns, &sqlite_error);

		// Display Table
		for (int rowcnt = 0; rowcnt <= rows; ++rowcnt)
		{
			for (int colcnt = 0; colcnt < columns; ++colcnt)
			{
				// Determine Cell Position
				int cell_pos = (rowcnt * columns) + colcnt;

				// Display Cell Value
				std::cout.width(col_width);
				std::cout.setf(std::ios::left);
				std::cout << results[cell_pos] << " ";
			}

			// End Line
			std::cout << std::endl;

			// Display Separator For Header
			if (0 == rowcnt)
			{
				for (int colcnt = 0; colcnt < columns; ++colcnt)
				{
					std::cout.width(col_width);
					std::cout.setf(std::ios::left);
					std::cout << "~~~~~~~~~~~~ ";
				}
				std::cout << std::endl;
			}
		}
	}
	catch (std::exception& except) {
		if (results != nullptr) {
			sqlite3_free_table(results);
		}
		
		throw except;
	}
}

int main()
{
	int ret = 0;
	print("starting application...");

	remove(db_name);
	
	sqlite3 *db = nullptr;
	char *sqlite_error = nullptr;
	try {
		ret = sqlite3_open(db_name, &db);
		if (ret) {
			print("Error opening SQLite3 database: ", sqlite3_errmsg(db));
			throw std::exception();
		}

		for (auto sql : prepare_sql) {
			print("exec query=[", (const char*) prepare_sql, "]");
			ret = sqlite3_exec(db, sql, NULL, NULL, &sqlite_error);
			if (ret) {
				print("exec query=[", prepare_sql, "]");
				print("Error executing SQLite3 statement: ", sqlite3_errmsg(db));
				throw std::exception();
			}
		}

		// Display MyTable
		print_table_results(db);
	}
	catch (std::exception&) {
		if (db != nullptr) {
			print("db closed");
			sqlite3_close(db);
		}

		if (sqlite_error != nullptr) {
			sqlite3_free(sqlite_error);
		}
	}


	return 0;
}
