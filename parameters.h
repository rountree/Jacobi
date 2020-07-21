#include <stdint.h>
#include <stdbool.h>        // requires --std=c99

#ifndef PARAMETERS_H
#define PARAMETERS_H


struct source{              // source and sinks are treated identically, so
    uint64_t x;             // we'll use this struct for both.
    uint64_t y;
    double temp;
};

struct parameters{
    uint64_t X;             // x field dimension
    uint64_t Y;             // y field dimension
    uint64_t nSource;       // number of sources and sinks
    struct source *source;  // array of sources and sinks
    uint64_t threads;       // number of threads to use
    double target_delta;    // Halt when maximum change is below this threshold
    char* output;     // Dump output to this filename
    char* kernel;           // Which kernel to run
    uint32_t bathsize;      // Width (and height) of the margin
    double bathtemp;        // Temperature of the bath
    bool debug;             // Generate debugging info, whatever that means
    bool report;            // Generate a report
    bool random;            // Initialize non-source, non-sink locations to random values.  Default is 0.0.
    bool dryrun;            // Just print the parameters and exit.
    // To be filled in by the kernel
    uint32_t thread_id;
};

extern void parse_options( int argc, char **argv, struct parameters *p );

#endif // PARAMETERS_H
