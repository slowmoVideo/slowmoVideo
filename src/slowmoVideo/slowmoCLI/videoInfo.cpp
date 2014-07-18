
#include <cstdio>
#include <cstring>

#include "../lib/videoInfo_sV.h"

void printUsage(const char progName[])
{
    printf("Displays information like frame rate of a video file. \nUsage: %s file\n", progName);
}

int main(int argc, char *argv[])
{
    if (argc < 2 || strcmp("-h", argv[1]) == 0) {
        printUsage(argv[0]);
	return -1;
    }
    getInfo(argv[1]);
    return 0;

}
