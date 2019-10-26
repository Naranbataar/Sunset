#include <float.h> /* DBL_MAX */
#include <stdio.h> /* fprintf, printf, stderr */
#include <string.h> /* strcmp */
#include <stdint.h> /* uint8_t */
#include <stdbool.h> /* bool, true */

#include "config.h" /* SAMPLING_RATE, SOFT_CLIPPER */
#include "parser.h" /* LINE, ParserError */
#include "utils.h" /* GetRange, GetState */

#include "components.h"

static double CONFIG_ATTACK;
static double CONFIG_RELEASE;
static double CONFIG_THRESHOLD;
static double CONFIG_GAIN;
static bool CONFIG_ACTIVATION;

static uint8_t STAGE;
static double STAGE_PROGRESS;

static inline uint8_t
Config(const LINE* line)
{
    uint8_t ret = 0;

    const char* key = line->data.config.key;
    const char* value = line->data.config.value;
    if (strcmp(key, "attack") == 0)
    {
        ret = GetRange(value, 1.0 / SAMPLING_RATE, DBL_MAX, 
                       &CONFIG_ATTACK);
    }
    else if (strcmp(key, "release") == 0)
    {
        ret = GetRange(value, 1.0 / SAMPLING_RATE, DBL_MAX, 
                       &CONFIG_RELEASE);
    }
    else if (strcmp(key, "gain") == 0)
    {
        double gain = 0;
        ret = GetRange(value, -30.0, 30.0, &gain);
        if (ret == 0)
        {
            CONFIG_GAIN = pow(10, gain/20);
        }
    }
    else if (strcmp(key, "threshold") == 0)
    {
        double threshold = 0;
        ret = GetRange(value, -30.0, 30.0, &threshold);
        if (ret == 0)
        {
            CONFIG_THRESHOLD = pow(10, threshold/20);
        }
    }
    else if (strcmp(key, "activation") == 0)
    {
        ret = GetState(value, "below", "above", &CONFIG_ACTIVATION);
    }
    else
    {
        ret = 1;
        ParserError("Invalid config key", line->raw);
    }

    return ret;
}

static inline bool
OnLimit(const double x)
{
    return (x > CONFIG_THRESHOLD && CONFIG_ACTIVATION) ||
           (x < CONFIG_THRESHOLD && !CONFIG_ACTIVATION);
}

static inline void
Process(const double x)
{
    if (STAGE == 0)
    {
        if (OnLimit(x))
        {
            STAGE = 1;
            STAGE_PROGRESS = x; 
        }
    }
    else if (STAGE == 1)
    {
        if (CONFIG_GAIN > 1 && STAGE_PROGRESS < CONFIG_GAIN)
        {
            STAGE_PROGRESS += (STAGE_PROGRESS + CONFIG_GAIN) / 
                              (CONFIG_ATTACK * SAMPLING_RATE);
        }
        else if (CONFIG_GAIN < 1 && STAGE_PROGRESS > CONFIG_GAIN)
        {
            STAGE_PROGRESS -= (STAGE_PROGRESS + CONFIG_GAIN) / 
                              (CONFIG_ATTACK * SAMPLING_RATE);
        }
        
        if (!(OnLimit(x))) 
        {
            STAGE = 2; 
            STAGE_PROGRESS = CONFIG_GAIN; 
        }
    }
    else if (STAGE == 2)
    {
        if (CONFIG_GAIN > 1 && STAGE_PROGRESS > 1)
        {
            STAGE_PROGRESS -= (STAGE_PROGRESS + CONFIG_GAIN) / 
                              (CONFIG_RELEASE * SAMPLING_RATE);
        }
        else if (CONFIG_GAIN < 1 && STAGE_PROGRESS < 1)
        {
            STAGE_PROGRESS += (STAGE_PROGRESS + CONFIG_GAIN) / 
                              (CONFIG_RELEASE * SAMPLING_RATE);
        }
        else
        {
            STAGE = 0;
            STAGE_PROGRESS = x; 
        }
    }
    printf("%f\n", SOFT_CLIPPER(x * STAGE_PROGRESS));
}

extern int
Range(int argc, char* argv[])
{
    uint8_t ret = 0;

    if (argc == 2)
    {
        CONFIG_ATTACK = 0.5;
        CONFIG_RELEASE = 0.5;
        CONFIG_THRESHOLD = 0;
        CONFIG_GAIN = 1;
        CONFIG_ACTIVATION = true;
        STAGE_PROGRESS = 1;
        ret = Processor(argv[1], Process, Config);
    }
    else
    {
        fprintf(stderr, "Usage: range IDENTIFIER\n");
        fprintf(stderr, "Increases/decreases signal dynamic range\n");
        fprintf(stderr, "Variables:\n");
        fprintf(stderr, "    attack: 0.0 - infinity (0.5)\n");
        fprintf(stderr, "    release: 0.0 - infinity (0.5)\n");
        fprintf(stderr, "    threshold: -30 - 30dB (0)\n");
        fprintf(stderr, "    gain: -30 - 30dB (0)\n");
        fprintf(stderr, "    activation: below/above (above)\n"); 
        ret = 1;
    }

    return ret;
}
