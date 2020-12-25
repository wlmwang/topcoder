#include <stdint.h>
#include <cstdlib>
#include <cstdio>
#ifndef __linux__
#define fwrite_unlocked fwrite
#define fputc_unlocked fputc
#endif
#include <sys/time.h>
#include <vector>
#include "solution.h"

using namespace std;

static void usage(const char* arg) {
    printf("Usage: %s inputfile outputfile timefile tempdir\n", arg);
}

static FILE *globalOut = NULL;

int main(int argc, char* argv[]){
    if (argc != 5) {
        usage(argv[0]);
        return -1;
    }

    const char *inputPath = argv[1];
    const char *outputPath = argv[2];
    const char *timeFilePath = argv[3];
    const char *tempDir = argv[4];

    globalOut = fopen(outputPath, "w");
    if (!globalOut) {
        fprintf(stderr, "cannot open output file %s\n", outputPath);
        return EXIT_FAILURE;
    }

    Solution solution;
    solution.init(tempDir);

    long long totalTimeUs = 0;
    timeval beginTime, endTime;

    // invoke user's process
    gettimeofday(&beginTime, NULL);
    solution.process(inputPath);
    gettimeofday(&endTime, NULL);
    totalTimeUs = (endTime.tv_sec * 1000000 + endTime.tv_usec) -
            (beginTime.tv_sec * 1000000 + beginTime.tv_usec);
    fclose(globalOut);

    // write total time in microseconds
    FILE *timeFile = fopen(timeFilePath, "w");
    if (timeFile) {
        fprintf(timeFile, "%lld\n", totalTimeUs);
        fclose(timeFile);
    }
    else {
        fprintf(stderr, "cannot open time file %s\n", timeFilePath);
        return EXIT_FAILURE;
    }

    return 0;
}

void addSet(const vector<string> &result)
{
    if (__builtin_expect(result.empty(), 0))
        return;

    FILE *out = globalOut;

    vector<string>::const_iterator it = result.begin();
    fwrite_unlocked(it->data(), 1, it->size(), out);
    for (++it; it != result.end(); ++it) {
        fputc_unlocked(',', out);
        fwrite_unlocked(it->data(), 1, it->size(), out);
    }
    fputc_unlocked('\n', out);
}
