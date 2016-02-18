#include "Archive.h"

using namespace kiva;

void ArchiveReader::deleteEntry(ArchiveEntry *entry)
{
	if (!entry) {
		return;
	}
	
	if (entry->name) {
		delete[] entry->name;
		entry->name = NULL;
	}
	
	if (entry->content) {
		delete[] entry->content;
		entry->content = NULL;
	}
	
	if (entry->info) {
		delete entry->info;
		entry->info = NULL;
	}
	
	delete entry;
}


uint64_t ArchiveReader::calculateNextRead(uint64_t readed, uint64_t maxSize, ArchiveEntryInfo *info)
{
	if (info->contentLength < maxSize) {
		return info->contentLength;
	} else if (info->contentLength - readed < maxSize) {
		return info->contentLength - readed;
	} else {
		return maxSize;
	}
}


ArchiveReader::ArchiveReader()
	:stream(NULL), header(NULL)
{
}


ArchiveReader::ArchiveReader(const std::string &file)
	:ArchiveReader()
{
	open(file);
}


ArchiveReader::~ArchiveReader()
{
	if (header) {
		delete header;
		header = NULL;
	}

	close();
}


bool ArchiveReader::isOpen() const
{
	return stream != NULL;
}


bool ArchiveReader::isValid() const
{
	return header != NULL;
}


ArchiveEntry* ArchiveReader::getNextEntry()
{
	if (!isOpen() || !isValid()) {
		return NULL;
	}
	
	ArchiveEntry *ent = new ArchiveEntry;
	
	ent->info = getNextEntryInfo();
	if (!ent->info) {
		ArchiveReader::deleteEntry(ent);
		return NULL;
	}
	
	ent->name = new char[ent->info->nameLength+1];
	if (!ent->name) {
		ArchiveReader::deleteEntry(ent);
		return NULL;
	}
	
	ent->content = new char[ent->info->contentLength];
	if (!ent->content) {
		ArchiveReader::deleteEntry(ent);
		return NULL;
	}
	
	readEntryName(ent->name, ent->info);
	readEntryContent(ent->content, ent->info);
	
	return ent;
}


ArchiveEntryInfo* ArchiveReader::getNextEntryInfo()
{
	if (!isOpen() || !isValid()) {
		return NULL;
	}
	
	ArchiveEntryInfo *info = new ArchiveEntryInfo;
	
	fread(info, sizeof(ArchiveEntryInfo), 1, stream);
	
	if (info->nameLength <= 0 || info->contentLength <= 0) {
		delete info;
		return NULL;
	}
	
	return info;
}


void ArchiveReader::readEntryContent(char *buffer, uint64_t length, uint64_t offset)
{
	if (!isOpen() || !isValid()) {
		return;
	}
	
	fseek(stream, offset, SEEK_CUR);
	
	if (!buffer) {
		fseek(stream, length, SEEK_CUR);
		return;
	}
	
	fread(buffer, length, 1, stream);
}


void ArchiveReader::readEntryContent(char *buffer, ArchiveEntryInfo *info)
{
	readEntryContent(buffer, info->contentLength, 0);
}


void ArchiveReader::readEntryName(char *buffer, ArchiveEntryInfo *info)
{
	readEntryContent(buffer, info->nameLength, 0);
	buffer[info->nameLength] = '\0';
}


bool ArchiveReader::open(const std::string &file)
{
	stream = fopen(file.c_str(), "rb");
	if (!stream) {
		return false;
	}
	
	fseek(stream, 0, SEEK_END);
	uint64_t size = ftell(stream);
	fseek(stream, 0, SEEK_SET);
	
	if (size < sizeof(ArchiveHeader)) {
		return false;
	}
	
	header = new ArchiveHeader;
	if (!header) {
		return false;
	}
	
	fread(header, sizeof(ArchiveHeader), 1, stream);
	if (header->mg0 != MG0 || header->mg1 != MG1) {
		delete header;
		return false;
	}
	
	return true;
}


void ArchiveReader::close()
{
	if (isOpen()) {
		fclose(stream);
		stream = NULL;
	}
}

