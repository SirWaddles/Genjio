#pragma once

#include <string>

class DBMan {
public:
	static DBMan* getDBMan();
	virtual ~DBMan() {}
	virtual void CreateTables() = 0;
	virtual int CreateNewEntry(std::string templateName) = 0;
};