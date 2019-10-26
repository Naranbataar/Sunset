#include <math.h> /* pow */
#include <stdio.h> /* fprintf, printf, stderr */
#include <string.h> /* strcmp */
#include <stdint.h> /* uint8_t */
#include <stdbool.h> /* bool, false */

#include "config.h" /* SAMPLING_RATE, SOFT_CLIPPER */
#include "parser.h" /* LINE, ParserError */
#include "utils.h" /* GetRange, GetState */

#include "components.h"

static double CONFIG_GAIN;
static bool CONFIG_CLIPPING;

static inline uint8_t
Config(const LINE line)
{
    uint8_t ret = 0;

    const char* key = line.data.config.key;
    const char* value = line.data.config.value;
    if (strcmp(key, "gain") == 0)
    {
        double gain = 0;
        ret = GetRange(value, -30.0, 30.0, &gain);
        if (ret == 0)
        {
            CONFIG_GAIN = pow(10, gain/20);
        }
    }
    else if (strcmp(key, "clipping") == 0)
    {
        ret = GetState(value, "soft", "hard", &CONFIG_CLIPPING);
    }
    else
    {
        ret = 1;
        ParserError("Invalid config key", line.raw);
    }

    return ret;
}

static inline void
Process(const double x)
{
    double y = x * CONFIG_GAIN;
    if (CONFIG_CLIPPING)
    {
        y = (y <=  1) ? y :  1;
        y = (y >= -1) ? y : -1;
    }
    else
    {
        y = SOFT_CLIPPER(y);
    }
    printf("%f\n", y);
}

extern int
Vol(int argc, char* argv[])
{
    uint8_t ret = 0;

    if (argc == 2)
    {
        CONFIG_GAIN = 0;
        CONFIG_CLIPPING = false;        

        ret = Processor(argv[1], Process, Config);
    }
    else
    {
        fprintf(stderr, "Usage: vol NAME\n");
        fprintf(stderr, "Increases/decreases signal amplitude\n");
        fprintf(stderr, "Variables:\n");
        fprintf(stderr, "    gain: -30 - 30dB (0)\n");
        fprintf(stderr, "    clipping: soft/hard (soft)\n"); 
        ret = 1;
    }

    return ret;
}
