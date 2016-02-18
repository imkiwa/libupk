#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/wait.h>

#include "Archive.h"

extern int minigzip(int, char**);

using namespace kiva;

typedef int(*ActionFn)(const char *file, char **args);


void do_extract_entry(ArchiveReader &re, ArchiveEntryInfo *info, const char *name)
{
	if (!ArchiveWriter::mkdirForFile(name)) {
		return;
	}
	
	FILE *fp = fopen(name, "wb");
	
	const int size = 1024;
	char buffer[size] = {0};
	size_t readed = 0;
	size_t next = 0;
	
	while (readed < info->contentLength) {
		next = ArchiveReader::calculateNextRead(readed, size, info);
		
		memset(buffer, '\0', size);
		
		re.readEntryContent(buffer, next, 0);
		fwrite(buffer, next, 1, fp);
		
		readed += next;
	}
	
	fclose(fp);
}


void do_extract_archive(const char *file, bool onlyList)
{
	ArchiveReader re(file);
	
	ArchiveEntryInfo *e;
	while ((e = re.getNextEntryInfo()) != NULL) {
		char name[e->nameLength+1] = {0};
		re.readEntryName(name, e);
		
		fprintf(stderr, "%s\n", name);
		
		if (onlyList) {
			re.readEntryContent(NULL, e);
		
		} else {
			do_extract_entry(re, e, name);
		}
	}
	
	re.close();
}


int usage(const char *file, char **args)
{
	fprintf(stderr,
		"\n"
		"Usage: %s -[lcxh] [...]\n"
		"\n"
		"  -l <upk>          List upk\n"
		"  -c <upk> <list>   Create upk from list\n"
		"  -x <upk>          Extract upk\n"
		"  -h                Help\n"
		"\n",
		file);
	
	return 0;
}


void do_minigzip(const char *file, char *tmpfile, bool comp)
{
	int status;
	pid_t pid = fork();
	
	if (pid == 0) {
		// child
		int argc;
		char *argv[] = { (char*) "minigzip", (char*) "-c", NULL, NULL, NULL };
		
		if (comp) {
			argc = 3;
			argv[2] = (char*) file;
		} else {
			argc = 4;
			argv[2] = (char*) "-d";
			argv[3] = (char*) file;
		}
		
		int fd = mkstemp(tmpfile);
		
		dup2(fd, STDOUT_FILENO);
		minigzip(argc, argv);
		close(fd);
		
	} else if (pid > 0) {
		// parent
		waitpid(pid, &status, 0);
	
	} else {
		fprintf(stderr, "fork: %s\n", strerror(errno));
		exit(1);
	}
}


void common_extract(const char *file, bool listOnly)
{
	char tmpfile[] = ".upk-tmp-XXXXXXX";
	
	do_minigzip(file, tmpfile, false);
	do_extract_archive(tmpfile, listOnly);
	
	unlink(tmpfile);
}


int listArchive(const char *file, char **args)
{
	common_extract(file, true);
	return 0;
}


int extractArchive(const char *file, char **args)
{
	common_extract(file, false);
	return 0;
}


int createArchive(const char *file, char **args)
{
	if (!args[0]) {
		fprintf(stderr, "empty archive\n");
		return 1;
	}
	
	ArchiveWriter wr(file);
	
	while (args[0]) {
		wr.add(args[0]);
		args++;
	}
	
	wr.close();
	
	char tmpfile[] = ".upk-tmp-XXXXXXXX";
	do_minigzip(file, tmpfile, true);
	rename(tmpfile, file);
	
	return 0;
}


int main(int argc, char **argv)
{
	opterr = 0;
	
	ActionFn fn = NULL;
	char *file = NULL;
	
	if (argc == 1) {
		return usage(argv[0], argv);
	}
	
	int result;
	while ((result = getopt(argc, argv, "l:c:x:h")) != -1) {
		switch(result) {
		case 'l':
			fn = listArchive;
			file = optarg;
			break;
		
		case 'c':
			fn = createArchive;
			file = optarg;
			break;
		
		case 'x':
			fn = extractArchive;
			file = optarg;
			break;
		
		case 'h':
			fn = usage;
			file = argv[0];
			break;
		
		case '-':
			break;
		
		case '?':
		default:
			fprintf(stderr, "Unrecognized option\n");
			fn = usage;
			file = argv[0];
			break;
		}
	}
	
	int ret = 0;
	
	if (fn) {
		ret = fn(file, argv + optind);
	}
	
	return ret;
}

