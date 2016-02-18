#include "Archive.h"

using namespace kiva;


int main(int argc, char **argv) {
	argv++;
	ArchiveWriter wr(argv[0]);
	
	while (++argv && argv[0]) {
		wr.add(argv[0]);
	}
	
	wr.close();
}
