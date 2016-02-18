#include <dirent.h>
#include <errno.h>
#include <string.h>

#include "Archive.h"

using namespace kiva;


static bool isSameFile(struct stat *l, struct stat *r)
{
	if (l->st_dev == r->st_dev 
		&& l->st_ino == r->st_ino) {
		return true;
	}
	
	return false;
}

static std::string parseName(const std::string &file)
{
	int pos;
	std::string ret(file);
	
	while ((pos = ret.find("../")) != std::string::npos) {
		ret = std::move(ret.substr(pos+3));
	}
	
	if (ret[0] == '/') {
		ret = std::move(ret.substr(1));
	}
	
	return std::move(ret);
}


static bool mkdirp(char *path, int mode)
{
	int ret = 0;
	char currpath[PATH_MAX];
	char *pathpiece;
	
	struct stat st;
	strcpy(currpath, "");
	
	pathpiece = strtok(path, "/");
	if(path[0] == '/') {
		strcat(currpath, "/");
	}
	
	while (pathpiece != NULL) {
		if (strlen(currpath) + strlen(pathpiece) + 2 > PATH_MAX) {
			fprintf(stderr, "path too long\n");
			return false;
		}
		
		strcat(currpath, pathpiece);
		strcat(currpath, "/");
		
		if (stat(currpath, &st) != 0) {
			ret = mkdir(currpath, mode);
			if (ret < 0) {
				fprintf(stderr, "mkdir failed for %s: %s\n", currpath, strerror(errno));
				return false;
			}
		}
		pathpiece = strtok(NULL, "/");
	}
	
	return true;
}


bool ArchiveWriter::mkdirForFile(const std::string &file)
{
	int pos = file.find_last_of('/');
	if (pos == std::string::npos) {
		return true;
	}

	std::string path = std::move(file.substr(0, pos));
	return mkdirp((char*) path.c_str(), 0755);
}


ArchiveWriter::ArchiveWriter()
	:stream(NULL)
{
}


ArchiveWriter::ArchiveWriter(const std::string &file)
	:ArchiveWriter()
{
	open(file);
}


ArchiveWriter::~ArchiveWriter()
{
	close();
}

	
bool ArchiveWriter::isOpen() const
{
	return stream != NULL;
}


void ArchiveWriter::add(const std::string &path)
{
	if (!isOpen()) {
		return;
	}
	
	struct stat buf;
	if (stat(path.c_str(), &buf) < 0) {
		return;
	}
	
	if (S_ISSOCK(buf.st_mode)) {
		fprintf(stderr, "%s: socket ignored\n", path.c_str());
		return;
	}
	
	if (isSameFile(&buf, &fileInfo)) {
		fprintf(stderr, "%s: is the archive, skipping\n", path.c_str());
		return;
	}
	
	if (S_ISDIR(buf.st_mode)) {
		addDir(path);
	} else if (S_ISREG(buf.st_mode)) {
		addFile(path);
	}
}


void ArchiveWriter::addFile(const std::string &file)
{
	if (!isOpen() || file.empty()) {
		return;
	}
	
	std::string parsedName = parseName(file);
	if (parsedName.empty()) {
		return;
	}
	
	FILE *fp = fopen(file.c_str(), "rb");
	if (!fp) {
		return;
	}
	
	fprintf(stderr, "%s\n", parsedName.c_str());
	
	ArchiveEntry ent;
	ent.info = new ArchiveEntryInfo;
		
	ent.name = (char*) parsedName.c_str();
	ent.info->nameLength = parsedName.length();
	
	fseek(fp, 0, SEEK_END);
	ent.info->contentLength = ftell(fp);
	ent.content = new char[ent.info->contentLength];
	fseek(fp, 0, SEEK_SET);
	
	fread(ent.content, ent.info->contentLength, 1, fp);
	fclose(fp);
	
	fwrite(ent.info, sizeof(ArchiveEntryInfo), 1, stream);
	fwrite(ent.name, ent.info->nameLength, 1, stream);
	fwrite(ent.content, ent.info->contentLength, 1, stream);
	
	delete[] ent.content;
	delete ent.info;
}


void ArchiveWriter::addDir(const std::string &path)
{
	if (!isOpen()) {
		return;
	}
	
	DIR *dir;
	struct dirent *e;
	
	dir = opendir(path.c_str());
	if (!dir) {
		return;
	}
	
	while ((e = readdir(dir)) != NULL) {
		if (!strcmp(e->d_name, ".") 
			|| !strcmp(e->d_name, "..")) {
			continue;
		}
		
		int length = path.length();	
		if (path[length - 1] == '/') {
			add(path + e->d_name);
		} else {
			add(path + '/' + e->d_name);
		}
	}
		
	closedir(dir);
}


bool ArchiveWriter::open(const std::string &file)
{
	stream = fopen(file.c_str(), "wb");
	if (!stream) {
		return false;
	}
	
	ArchiveHeader hdr;
	fwrite(&hdr, sizeof(ArchiveHeader), 1, stream);
	
	if (stat(file.c_str(), &fileInfo) < 0) {
		return false;
	}
	
	return true;
}


void ArchiveWriter::close()
{
	if (isOpen()) {
		fclose(stream);
		stream = NULL;
	}
}
