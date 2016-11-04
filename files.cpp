#include <cstdint>
#include <map>
#include <fstream>
#include <iostream>
#include "db.h"
#include "files.h"
#include "videocheck.h"

std::map<int, FileMapEntry> fileRegister;

FileMapEntry& GetFile(int id) {
	return fileRegister[id];
}

FileMapEntry::FileMapEntry(int size) {
	fileSize = size;
	contents.resize(fileSize);
	bytesWrote = 0;
}

FileMapEntry::FileMapEntry() {
	bytesWrote = 0;
}

bool FileMapEntry::IsComplete() {
	return bytesWrote >= fileSize;
}

int FileMapEntry::GetBytesWrote() {
	return bytesWrote;
}

void FileMapEntry::WriteBytes(int start, int length, std::string& chunk, int offset) {
	if (start < 0 || length < 0 || offset < 0) {
		printf("Bounds1\n");
		return;
	}
	if (length > chunk.size() - offset) {
		length = chunk.size() - offset;
	}
	if (start + length > contents.size()) {
		printf("Bounds3\n");
		return;
	}
	contents.replace(start, length, chunk, offset, length);
	bytesWrote += length;
}

std::string FileMapEntry::WriteFile(int id) {
	std::string filename = std::string("tmp/") + std::to_string(id) + ".mp4";
	std::ofstream out(filename, std::ios::binary);
	out.write(contents.c_str(), contents.size());
	return filename;
}

std::string& FileMapEntry::GetContents() {
	return contents;
}

void FileCreateTask::Run() {
	int id = DBMan::getDBMan()->CreateNewEntry("test");
	int fileSize = *(reinterpret_cast<const uint32_t*>(contents.c_str() + 4));
	fileRegister.insert(std::make_pair(id, FileMapEntry(fileSize)));

	std::string message = std::string("{\"newid\":") + std::to_string(id) + "}";
	mServer->send(mHdl, message.c_str(), message.length(), websocketpp::frame::opcode::text);
}

void FileAssemblerTask::Run() {
	
	fileID = *(reinterpret_cast<const uint32_t*>(contents.c_str() + 4));
	fileSize = *(reinterpret_cast<const uint32_t*>(contents.c_str() + 8));
	start = *(reinterpret_cast<const uint32_t*>(contents.c_str() + 12));
	end = *(reinterpret_cast<const uint32_t*>(contents.c_str() + 16));
	printf("FileID: %d Size: %d Start: %d End %d\n", fileID, fileSize, start, end);
	if (fileRegister.count(fileID) != 1) {
		return;
	}

	FileMapEntry& entry = fileRegister[fileID];
	entry.WriteBytes(start, end - start, contents, 20);
	printf("%f\n", double(entry.GetBytesWrote()) / double(fileSize));
	
	if (entry.IsComplete()) {
		std::string filename = entry.WriteFile(fileID);
		RunTask<VideoAnalysisTask>(mServer, mHdl, filename);
		fileRegister.erase(1);
	}
	
}