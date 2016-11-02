#include <cstdint>
#include <map>
#include <fstream>
#include <iostream>
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

void FileAssemblerTask::Run() {
	fileSize = *(reinterpret_cast<const uint32_t*>(contents.c_str() + 4));
	start = *(reinterpret_cast<const uint32_t*>(contents.c_str() + 8));
	end = *(reinterpret_cast<const uint32_t*>(contents.c_str() + 12));
	if (fileRegister.count(1) != 1) {
		fileRegister.insert(std::make_pair(1, FileMapEntry(fileSize)));
	}

	FileMapEntry& entry = fileRegister[1];
	entry.WriteBytes(start, end - start, contents, 16);
	printf("%f\n", double(entry.GetBytesWrote()) / double(fileSize));
	if (entry.IsComplete()) {
		std::string filename = entry.WriteFile(1);
		RunTask<VideoAnalysisTask>(mServer, mHdl, filename);
		fileRegister.erase(1);
	}
	
}