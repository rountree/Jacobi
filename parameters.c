#include <stdio.h>
#include <limits.h>     // ARG_MAX
#include <stdlib.h>     // realloc, exit(3)
#include <unistd.h>     // get_longopt(3)
#include <getopt.h>     // get_longopt(3)
#include <stdint.h>
#include <inttypes.h>
#include <string.h>     // memset(3)
#include "parameters.h"

#define MAX_STR_LEN 1024

static void sanity_check();
static void print_help();
static void print_version();
static void print_parameters(struct parameters *p);
static void set_defaults( struct parameters *p );

static struct option long_options[] = {
    {"XY",      required_argument,  0, 0},      // surface dimensions, comma separated, e.g. --XY=10000,20000 (can use -x and -y )
    {"threads", required_argument,  0, 'n'},    // number of threads to use
    {"source",  required_argument,  0, 's'},    // xy-location and fixed value, e.g., --source=200,300,100.0 (repeatable)
    {"sink",    required_argument,  0, 'S'},    // identical to --source
    {"delta",   required_argument,  0, 'd'},    // Halt when maximum difference between timesteps is less than this value.
    {"output",  required_argument,  0, 'o'},    // Dump final state of the field to this filename.
    {"kernel",  required_argument,  0, 'k'},
    {"bathsize",required_argument,  0, 'b'},    // Width of the surrounding bath.
    {"bathtemp",required_argument,  0, 'c'},    // Fixed temperature of the surrounding bath.
    {"report",  no_argument,        0, 'r'},    // Generate report (number of timesteps, timing info, etc.)
    {"verbose", no_argument,        0, 'v'},    // Generate verbose output helpful for debugging
    {"random",  no_argument,        0, 'z'},    // Initialize non-source, non-sink space to random numbers instead of 0.0
    {"dryrun",  no_argument,        0, 'D'},    // Print parameter values and exit.
    {"help",    no_argument,        0, 'h'},    // List command line parameters, version info, etc. and exit.
    {"version", no_argument,        0, 'V'},    // Print version and exit.
    {0,         0,                  0, 0  }};

static char filename[ MAX_STR_LEN ];
static char kernel[ MAX_STR_LEN ];

void
parse_options(int argc, char **argv, struct parameters *p){
    int c, option_index, rc;
    opterr = 1; // Allow getopt_long to print error messages.
    set_defaults( p );

    while(1){
        c = getopt_long( argc, argv, "x:y:n:s:S:d:o:k:rqzhV", long_options, &option_index );
        if( c == -1 ){
            break;
        }

        switch(c){
            case 0: // --XY
                rc = sscanf( optarg, "%" PRIu64 ",%" PRIu64, &(p->X), &(p->Y) );
                if( rc != 2 ){
                    fprintf( stderr, "Error parsing \"--XY=%s\".", optarg );
                    fprintf( stderr, "Expected '--XY=x,y' where x and y are positive integers.\n");
                    exit(-1);
                }
                break;

            case 'x':
                rc = sscanf( optarg, "%" PRIu64, &(p->X) );
                if( rc != 1 ){
                    fprintf( stderr, "Error parsing \"-x %s\".", optarg );
                    fprintf( stderr, "Expected '-x val' where val is a positive integer.\n");
                    exit(-1);
                }
                break;

            case 'y':
                rc = sscanf( optarg, "%" PRIu64, &(p->Y) );
                if( rc != 1 ){
                    fprintf( stderr, "Error parsing \"-y %s\".", optarg );
                    fprintf( stderr, "Expected '-y val' where val is a positive integer.\n");
                    exit(-1);
                }
                break;

            case 'n':   // --threads
                rc = sscanf( optarg, "%" PRIu64, &(p->threads) );
                if( rc != 1 ){
                    fprintf( stderr, "Error parsing \"-n/--threads %s\".", optarg );
                    fprintf( stderr, "Expected '-n val' where val is a positive integer.\n");
                    exit(-1);
                }
                break;

            case 's':   // --source (Fallthrough)
            case 'S':   // --sink
                p->source = (struct source *) realloc( p->source, sizeof(struct source) * ++(p->nSource) );
                if( !(p->source) ){
                    fprintf( stderr, "Error allocating memory in -s/-S/--source/--sink.  Bye!\n" );
                    exit(-1);
                }
                rc = sscanf( optarg, "%" PRIu64 ",%" PRIu64 ",%lf",
                        &(p->source[p->nSource-1].x),
                        &(p->source[p->nSource-1].y),
                        &(p->source[p->nSource-1].temp));
                if( rc != 3 ){
                    fprintf( stderr, "Error parsing \"-s/-S/--source/--sink %s\".", optarg );
                    fprintf( stderr, "Expected x,y,val where x and y are positive integer and val is a double.\n");
                    exit(-1);
                }
                break;

            case 'o':   // --output
                rc = sscanf( optarg, "%s", (char*)(&filename) );
                if( rc != 1 ){
                    fprintf( stderr, "Error parsing \"-o/--output %s\".", optarg );
                    fprintf( stderr, "Expected -o filename\n");
                    exit(-1);
                }
                p->output = (char*)(&filename);
                break;

            case 'k':   // --kernel
                rc = sscanf( optarg, "%s", (char*)(&kernel) );
                if( rc != 1 ){
                    fprintf( stderr, "Error parsing \"-k/--kernel %s\".", optarg );
                    fprintf( stderr, "Expected -k kernelname\n");
                    exit(-1);
                }
                p->kernel = (char*)(&kernel);
                break;

            case 'b':   // --bathsize
                rc = sscanf( optarg, "%" PRIu32, &(p->bathsize) );
                if( rc != 1 ){
                    fprintf( stderr, "Error parsing \"-b/--bathsize %s\".", optarg );
                    fprintf( stderr, "Expected '-b val' where val is the size of the margin.\n");
                    exit(-1);
                }
                break;

            case 'c':   // --bathtemp
                rc = sscanf( optarg, "%lf", &(p->bathtemp) );
                if( rc != 1 ){
                    fprintf( stderr, "Error parsing \"-c/--bathtemp %s\".", optarg );
                    fprintf( stderr, "Expected '-c double' where double is the fixed temperature.\n");
                    exit(-1);
                }
                break;

            case 'r':   p->report = true;           break;
            case 'v':   p->debug = true;            break;
            case 'z':   p->random = true;           break;
            case 'D':   p->dryrun = true;           break;
            case 'h':   print_help(); exit(0);      break;
            case 'V':   print_version(); exit(0);   break;
            case ':':   fprintf( stderr, "Missing required parameter for '%c', bye!\n", (char)c ); exit(-1); break;
            case '?':   fprintf( stderr, "Bye!\n" ); exit(-1); break;
            default:    fprintf( stderr, "Unknown error parsing parameters, c=0x%x.\n", c );       exit(-1); break;
        }
    }
    sanity_check( p );
    if( p->dryrun ){
    fprintf( stderr, "%s:%d p->dryrun = %d\n", __FILE__, __LINE__, p->dryrun );
       print_parameters( p ); 
    }
}

static void
print_help(){
    fprintf(stdout, "\n");
    fprintf(stdout, "Welcome to the Jacobi solver harness!\n" );
    fprintf(stdout, "\n");
    fprintf(stdout, "--XY              Surface dimensions in the form --XY=x,y. Default is 100x100.\n");
    fprintf(stdout, "--threads    -n   Number of threads to use.  Default is 1.\n" );
    fprintf(stdout, "--source     -s   xy-location and fixed value, e.g., --source=200,300,100.0 (repeatable, no default)\n" );
    fprintf(stdout, "--sink       -S   Identical to --source\n" );
    fprintf(stdout, "--delta      -d   Halt when maximum difference between timesteps is less than this value.\n" );
    fprintf(stdout, "                   Default is 0.001.\n");
    fprintf(stdout, "--output     -o   Dump final state of the field to this filename.  Default is Jacobi.out.\n" );
    fprintf(stdout, "--kernel     -k   Selects which algorithm to run.  Default is \"naive\".\n" );
    fprintf(stdout, "--bathsize   -b   Width of the margin that's kept at a constant temperature\n" );
    fprintf(stdout, "--bathtemp   -c   Static temperature of the surrounding bath.\n" );
    fprintf(stdout, "--report     -r   Generate report (number of timesteps, timing info, etc.)\n" );
    fprintf(stdout, "--verbose    -v   Generate verbose output helpful for debugging.\n" );
    fprintf(stdout, "--random     -z   Initialize non-source, non-sink space to random numbers instead of 0.0\n" );
    fprintf(stdout, "--dryrun     -D   Prints parameter values, attempt to allocate requested memory and exits.\n" );
    fprintf(stdout, "--help       -h   List available command line parameters and exit.\n" );
    fprintf(stdout, "--version    -V   Print version and exit.\n" );
    fprintf(stdout, "\n");
}

static void
print_version(){
    fprintf( stdout, "Authored by Barry Rountree, rountree@llnl.gov.\n" );
    fprintf( stdout, "This software is not yet released.\n" );
}

static void
set_defaults(struct parameters *p){
    memset( p, 0, sizeof( struct parameters ) );
    p->X = 100;
    p->Y = 100;
    p->threads = 1;
    p->bathsize = 2;
    p->bathtemp = 0.0;
    p->target_delta = 0.001;
    p->output = "Jacobi.out";
    p->kernel = "naive";
}

static void
sanity_check( struct parameters *p ){
    if( p->nSource ){
        for( int i=0; i<(p->nSource); i++ ){
            if ( p->source[i].x >= p->X
                    || p->source[i].x < 0
                    || p->source[i].y >= p->Y
                    || p->source[i].y < 0 ){
                fprintf( stdout, "source/sink out of bounds at x=%" PRIu64 " y=%" PRIu64 ", boundaries are X=%" PRIu64 " and Y=%" PRIu64 ".\n",
                    p->source[i].x, p->source[i].y, p->X, p->Y );
                exit(-1);
            }
        }
    }
}

static void
print_parameters(struct parameters *p){
    // At the moment this is the only thing --dryrun does.
    // Might be interesting later to test memory allocation.

    void *dummy;
    fprintf( stdout, "X=%" PRIu64 ", Y=%" PRIu64 ", margin=%" PRIu32 ", margin temp.=%lf, delta=%lf.\n",
            p->X, p->Y, p->bathsize, p->bathtemp, p->target_delta);
    if( p->nSource ){
        for( int i=0; i<(p->nSource); i++ ){
            fprintf( stdout, "source/sink at x=%" PRIu64 " y=%" PRIu64 " temp=%lf.\n",
                    p->source[i].x, p->source[i].y, p->source[i].temp);
        }
    }else{
        fprintf( stdout, "No sources/sinks specified.\n");
    }
    fprintf( stdout, "Output file=%s.\n", p->output);
    fprintf( stdout, "Kernel=%s\n", p->kernel);
    fprintf( stdout, "Number of threads=%" PRIu64 ".\n", p->threads);
    fprintf( stdout, "Debug=%s Report=%s random=%s dryrun=%s\n", 
            p->debug    ? "true" : "false",
            p->report   ? "true" : "false",
            p->random   ? "true" : "false",
            p->dryrun   ? "true" : "false");
    fprintf( stdout, "Testing ability to calloc %" PRIu64 " bytes:  (%" PRIu64 " x %" PRIu64 " x 2 x 8)... ", p->X * p->Y * 16, p->X, p->Y );
    dummy = calloc( p->X * p->Y * 2 , 8);
    if( dummy ){
        fprintf( stdout, "Success!\n" );
        free( dummy );
    }else{
        fprintf( stdout, "Nope....\n" );
    }
}

