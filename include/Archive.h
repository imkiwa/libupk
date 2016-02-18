#pragma once

#include <stdio.h>
#include <unistd.h>
#include <limits.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <string>

#define MG0 0x77
#define MG1 0xa5
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
	char mg0 = MG0;
	char mg1 = MG1;
	
	int major = MAJOR;
	int minor = MINOR;
} ArchiveHeader;


typedef struct
{
	long int contentLength = 0;
	size_t nameLength = 0;
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
	static size_t calculateNextRead(size_t readed, size_t maxSize, ArchiveEntryInfo *info);
	
public:
	ArchiveReader();
	ArchiveReader(const std::string &file);
	~ArchiveReader();
	
	bool isOpen() const;
	bool isValid() const;
	
	ArchiveEntry* getNextEntry();
	ArchiveEntryInfo* getNextEntryInfo();
	
	void readEntryContent(char *buffer, size_t length, size_t offset);
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

