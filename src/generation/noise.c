#include <stdio.h> /* fprintf, printf, stderr */
#include <stdlib.h> /* rand, RAND_MAX */
#include <stdint.h> /* uint8_t */

#include "config.h" /* SAMPLING_RATE */
#include "parser.h" /* LINE, ParserError */ 
#include "utils.h" /* Generator */

#include "components.h"

static inline uint8_t
Config(const LINE line)
{
    uint8_t ret = 1;
    (void) line;
    ParserError("Invalid key", line.raw);

    return ret;
}

static inline void
Play(void)
{
    printf("%f\n", (((double) (rand()) / (double) RAND_MAX) - 0.5) * 2);
}

extern int
Noise(int argc, char* argv[])
{
    uint8_t ret = 0;

    if (argc == 2)
    {
        ret = Generator(argv[1], Play, Config);
    }
    else
    {
        fprintf(stderr, "Usage: noise NAME\n");
        fprintf(stderr, "Generates noise\n");
        ret = 1;
    }

    return ret;
}
