#include <sqlite3.h>
#include "db.h"

class DBManLite : public DBMan {
public:
	DBManLite();
	virtual ~DBManLite();
	virtual void CreateTables();
	virtual int CreateNewEntry(std::string templateName);
private:
	sqlite3* db;
};

DBMan* singletonDBMan = nullptr;

DBMan* DBMan::getDBMan() {
	if (!singletonDBMan) {
		singletonDBMan = new DBManLite();
	}
	return singletonDBMan;
}

DBManLite::DBManLite() {
	sqlite3_open("genjio.db", &db);
	CreateTables();
}

DBManLite::~DBManLite() {
	sqlite3_close(db);
}

void DBManLite::CreateTables() {
	const char query[] = "CREATE TABLE IF NOT EXISTS videos (id INTEGER PRIMARY KEY AUTOINCREMENT, template TEXT)";

	char* errmsg = new char[500];
	sqlite3_exec(db, query, nullptr, nullptr, &errmsg);
	if (errmsg) {
		printf("%s\n", errmsg);
	}
	delete[] errmsg;
}

int DBManLite::CreateNewEntry(std::string templateName) {
	const char query[] = "INSERT INTO videos(template) VALUES(?)";
	sqlite3_stmt* prepared = nullptr;

	sqlite3_prepare_v2(db, query, 50, &prepared, NULL);
	sqlite3_bind_text(prepared, 1, templateName.c_str(), templateName.size(), SQLITE_TRANSIENT);
	sqlite3_step(prepared);
	sqlite3_finalize(prepared);
	prepared = nullptr;

	const char query2[] = "SELECT seq FROM sqlite_sequence WHERE name='videos'";
	sqlite3_prepare_v2(db, query2, 100, &prepared, NULL);
	
	int status = sqlite3_step(prepared);
	int rowid = 0;
	switch (status) {
	case SQLITE_DONE:
		printf("Query complete\n");
		break;
	case SQLITE_ERROR:
		printf("Error!\n");
		break;
	case SQLITE_ROW:
		rowid = sqlite3_column_int(prepared, 0);
		printf("New row: %d\n", rowid);
		break;
	default:
		break;
	}
	sqlite3_finalize(prepared);
	return rowid;
}