#pragma once

#include <stdio.h>
#include <unistd.h>
#include <limits.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdint.h>

#include <string>

#define MAGIC 0x77
#define MAJOR 1
#define MINOR 0

namespace kiva {

/*

|------------------|
|  ArchiveHeader   |
|------------------|
| ArchiveEntryInfo |
|------------------|
|    file name     |
|------------------|
|   file content   |
|------------------|
| ArchiveEntryInfo |
|------------------|
|    file name     |
|------------------|
|   file content   |
|------------------|
|       ...        |
|------------------|

*/

typedef struct
{
	char magic = MAGIC;
	
	int32_t major = MAJOR;
	int32_t minor = MINOR;
} ArchiveHeader;


typedef struct
{
	uint64_t contentLength = 0;
	uint64_t nameLength = 0;
} ArchiveEntryInfo;


typedef struct
{
	ArchiveEntryInfo *info = NULL;
	char *name = NULL;
	char *content = NULL;
} ArchiveEntry;


class ArchiveReader
{
private:
	FILE *stream;
	ArchiveHeader *header;

public:
	static void deleteEntry(ArchiveEntry *entry);
	static uint64_t calculateNextRead(uint64_t readed, uint64_t maxSize, ArchiveEntryInfo *info);
	
public:
	ArchiveReader();
	ArchiveReader(const std::string &file);
	~ArchiveReader();
	
	bool isOpen() const;
	bool isValid() const;
	
	ArchiveEntry* getNextEntry();
	ArchiveEntryInfo* getNextEntryInfo();
	
	void readEntryContent(char *buffer, uint64_t length, uint64_t offset);
	void readEntryContent(char *buffer, ArchiveEntryInfo *info);
	
	void readEntryName(char *buffer, ArchiveEntryInfo *info);
	
	bool open(const std::string &file);
	void close();
};


class ArchiveWriter
{
private:
	FILE *stream;
	struct stat fileInfo;

public:
	static bool mkdirForFile(const std::string &file);
	
public:
	ArchiveWriter();
	ArchiveWriter(const std::string &file);
	~ArchiveWriter();
	
	bool isOpen() const;
	
	void add(const std::string &path);
	void addFile(const std::string &file);
	void addDir(const std::string &path);
	
	bool open(const std::string &file);
	void close();
};

}

