#pragma once

#include "task.h"

class FileAssemblerTask : public Task {
public:
	using Task::Task;
	virtual void Run();
protected:
	int fileSize;
	int start;
	int end;
	int fileID;
};

class FileCreateTask : public Task {
public:
	using Task::Task;
	virtual void Run();
};

class FileMapEntry {
public:
	FileMapEntry(int size);
	FileMapEntry();
	int GetBytesWrote();
	bool IsComplete();
	void WriteBytes(int start, int length, std::string& chunk, int offset);
	std::string WriteFile(int id);
	std::string& GetContents();
private:
	int fileSize;
	int bytesWrote;
	std::string contents;
};

FileMapEntry& GetFile(int id);