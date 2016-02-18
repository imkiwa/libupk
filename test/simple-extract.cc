#include "Archive.h"

using namespace kiva;


int main(int argc, char **argv) {
	ArchiveReader re(argv[1]);
	
	ArchiveEntry *e;
	while ((e = re.getNextEntry()) != NULL) {
		printf("extracting: %s\n", e->name);
		
		if (!ArchiveWriter::mkdirForFile(e->name)) {
			continue;
		}
		
		FILE *fp = fopen(e->name, "wb");
		fwrite(e->content, e->info->contentLength, 1, fp);
		fclose(fp);
		
		ArchiveReader::deleteEntry(e);
	}
	
	re.close();
}
