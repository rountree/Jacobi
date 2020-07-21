#include "parameters.h"

int
main( int argc, char **argv ){
    struct parameters p;
    parse_options(argc, argv, &p);
    return 0;
}

