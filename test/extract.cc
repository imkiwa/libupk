#include "Archive.h"

using namespace kiva;


int main(int argc, char **argv) {
	ArchiveReader re(argv[1]);
	
	ArchiveEntryInfo *e;
	while ((e = re.getNextEntryInfo()) != NULL) {
		char name[e->nameLength+1] = {0};
		re.readEntryName(name, e);
		
		printf("%s\n", name);
		if (!ArchiveWriter::mkdirForFile(name)) {
			continue;
		}
		
		FILE *fp = fopen(name, "wb");
		
		const int size = 1024;
		char buffer[size] = {0};
		size_t readed = 0;
		size_t next = 0;
		
		while (readed < e->contentLength) {
			next = ArchiveReader::calculateNextRead(readed, size, e);
			
			memset(buffer, '\0', size);
			
			re.readEntryContent(buffer, next, 0);
			fwrite(buffer, next, 1, fp);
			
			readed += next;
		}
		
		fclose(fp);
	}
	
	re.close();
}
