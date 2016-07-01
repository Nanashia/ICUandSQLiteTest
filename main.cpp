#include <iostream>
#include <string>
#include <memory>

#include "sqlite3.h"      // SQLite
#include "unicode/coll.h" // ICU

#include "utility.h"

static const char* const db_name = "test.db";
static const size_t col_width = 12;

// Colletionの調整
// http://www.unicode.org/reports/tr35/tr35-collation.html#Collation_Settings
static const char* const locale_lang = 
	"de-u-co-phonebk"
	"-ka-shifted"     // 空白無視
	"-ks-level1";     // strength（厳密さを最もゆるく）

static const char* const select_sql[] = {

	// WHERE text = ? 
	"SELECT * FROM tbl_example where text = 'a' COLLATE CUSTOM;",
	"SELECT * FROM tbl_example where text = 'ae' COLLATE CUSTOM;",
	"SELECT * FROM tbl_example where text = 'b b' COLLATE CUSTOM;",

	// ORDER BY
	"SELECT * FROM tbl_example order by text COLLATE CUSTOM;",
};

static const char* const prepare_sql[] = {
	u8"CREATE TABLE tbl_example (id INTEGER PRIMARY KEY AUTOINCREMENT, text TEXT);",
	u8"INSERT INTO tbl_example(text) VALUES('a');",
	u8"INSERT INTO tbl_example(text) VALUES('A');",
	u8"INSERT INTO tbl_example(text) VALUES('Å');",
	u8"INSERT INTO tbl_example(text) VALUES('∀');",
	u8"INSERT INTO tbl_example(text) VALUES('à');",
	u8"INSERT INTO tbl_example(text) VALUES('æ');",
	u8"INSERT INTO tbl_example(text) VALUES('Ǣ');",
	u8"INSERT INTO tbl_example(text) VALUES('Ǟ');",
	u8"INSERT INTO tbl_example(text) VALUES('å');",
	u8"INSERT INTO tbl_example(text) VALUES('Ａ');",
	u8"INSERT INTO tbl_example(text) VALUES('ａ');",
	
	u8"INSERT INTO tbl_example(text) VALUES('bb');",
	u8"INSERT INTO tbl_example(text) VALUES('b b');",
	u8"INSERT INTO tbl_example(text) VALUES('b  b');",
	u8"INSERT INTO tbl_example(text) VALUES('b　b');",
	u8"INSERT INTO tbl_example(text) VALUES('b　　b');",
	u8"INSERT INTO tbl_example(text) VALUES('b\tb');",
	u8"INSERT INTO tbl_example(text) VALUES('b\t\tb');",
};

static char *sqlite_error = nullptr;

void prepare_records(sqlite3* db) {
	int ret = 0;

	for (auto sql : prepare_sql) {
		print("exec query=[", sql, "]");
		ret = sqlite3_exec(db, sql, NULL, NULL, &sqlite_error);
		if (ret) {
			print("Error executing SQLite3 statement: ", sqlite3_errmsg(db));
			throw std::exception();
		}
	}
}

void print_table_results(sqlite3* db, const char* sql) {
	int ret = 0;
	int rows, columns;
	char **results = nullptr;

	try {
		print("select query=[", sql, "]");
		ret = sqlite3_get_table(db, sql, &results, &rows, &columns, &sqlite_error);

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
	if (results != nullptr) {
		sqlite3_free_table(results);
	}
}

int collate_CUSTOM(void* ptr, int len1, const void* ptr1, int len2, const void* ptr2) {
	StringPiece s1((const char*)ptr1, len1);
	StringPiece s2((const char*)ptr2, len2);
	UErrorCode status;
	return ((Collator*)ptr)->compareUTF8(s1, s2, status);
}

void setup_collater(sqlite3 *db, std::unique_ptr<Collator>& coll) {
	int ret = 0;
	UErrorCode status = U_ZERO_ERROR;

	print("init collator with", locale_lang);
	coll.reset(Collator::createInstance(Locale(locale_lang), status));
	if (U_FAILURE(status) || !coll) {
		print("Collator create instance failed");
		throw std::exception();
	}

	ret = sqlite3_create_collation(db, "CUSTOM", SQLITE_UTF8, coll.get(), collate_CUSTOM);
	if (ret) {
		print("Error sqlite3_create_collation failed: ", sqlite3_errmsg(db));
		throw std::exception();
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
		std::unique_ptr<Collator> coll;

		ret = sqlite3_open(db_name, &db);
		if (ret) {
			print("Error opening SQLite3 database: ", sqlite3_errmsg(db));
			throw std::exception();
		}

		setup_collater(db, coll);

		prepare_records(db);

		for (auto sql : select_sql) {
			print_table_results(db, sql);
		}
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
