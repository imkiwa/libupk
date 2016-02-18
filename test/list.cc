#include "Archive.h"

using namespace kiva;


int main(int argc, char **argv) {
	ArchiveReader re(argv[1]);
	
	ArchiveEntryInfo *e;
	while ((e = re.getNextEntryInfo()) != NULL) {
		char *name = new char[e->nameLength+1];
		re.readEntryName(name, e);
		
		/* skip content */
		re.readEntryContent(NULL, e);
		
		printf("%s\n", name);
	}
	
	re.close();
}
