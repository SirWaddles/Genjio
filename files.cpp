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

FileMapEntry::FileMapEntry(int size, websocketpp::connection_hdl hdl) {
	fileSize = size;
	contents.resize(fileSize);
	bytesWrote = 0;
	mHdl = hdl;
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

const websocketpp::connection_hdl FileMapEntry::GetHandle() {
	return mHdl;
}

// u wot
template <typename T, typename U>
inline bool weak_equals(const std::weak_ptr<T>& t, const std::weak_ptr<U>& u)
{
	return !t.owner_before(u) && !u.owner_before(t);
}

void FileUploadDisconnectTask::Run() {
	for (auto it = fileRegister.begin(); it != fileRegister.end();) {
		if (weak_equals(it->second.GetHandle(), mHdl)) {
			fileRegister.erase(it++);
		}
		else {
			++it;
		}
	}
}

void FileCreateTask::Run() {
	int id = DBMan::getDBMan()->CreateNewEntry("test");
	int fileSize = *(reinterpret_cast<const uint32_t*>(contents.c_str() + 4));
	fileRegister.insert(std::make_pair(id, FileMapEntry(fileSize, mHdl)));
	printf("Creating temp file of %d bytes\n", fileSize);

	std::string message = std::string("{\"newid\":") + std::to_string(id) + "}";
	mServer->send(mHdl, message.c_str(), message.length(), websocketpp::frame::opcode::text);
}

void FileAssemblerTask::Run() {
	fileID = *(reinterpret_cast<const uint32_t*>(contents.c_str() + 4));
	fileSize = *(reinterpret_cast<const uint32_t*>(contents.c_str() + 8));
	start = *(reinterpret_cast<const uint32_t*>(contents.c_str() + 12));
	end = *(reinterpret_cast<const uint32_t*>(contents.c_str() + 16));
	double progress = double(end) / double(fileSize);
	printf("FileID: %d Size: %d Start: %d End %d\n", fileID, fileSize, start, end);
	std::string message = std::string("{\"id\":" + std::to_string(fileID) + ",\"start\":" + std::to_string(start) + ",\"end\":" + std::to_string(end) + ",\"progress\":" + std::to_string(progress) + "}");
	mServer->send(mHdl, message.c_str(), message.length(), websocketpp::frame::opcode::text);
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