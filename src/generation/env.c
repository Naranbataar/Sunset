#include <float.h> /* DBL_MAX */
#include <stdio.h> /* fprintf, printf, stderr */
#include <string.h> /* strcmp */
#include <stdint.h> /* uint8_t */
#include <stdbool.h> /* bool, true, false */

#include "config.h" /* SAMPLING_RATE */
#include "parser.h" /* LINE, ParserError */
#include "utils.h" /* GetRange, GetState, Generator */

#include "components.h"

static double CONFIG_ATTACK;
static double CONFIG_DELAY;
static double CONFIG_SUSTAIN;
static double CONFIG_RELEASE;
static bool CONFIG_GATE;

static uint8_t STAGE;
static double STAGE_PROGRESS;
static double STAGE_RELEASE;

static inline uint8_t
Config(const LINE line)
{
    uint8_t ret = 0;

    const char* key = line.data.config.key;
    const char* value = line.data.config.value;
    if (strcmp(key, "attack") == 0)
    {
        ret = GetRange(value, 1.0 / SAMPLING_RATE, DBL_MAX, 
                       &CONFIG_ATTACK);
    }
    else if (strcmp(key, "delay") == 0)
    {
        ret = GetRange(value, 1.0 / SAMPLING_RATE, DBL_MAX, 
                       &CONFIG_DELAY);
    }
    else if (strcmp(key, "sustain") == 0)
    {
        ret = GetRange(value, 0.0, 1.0, &CONFIG_RELEASE);
    }
    else if (strcmp(key, "release") == 0)
    {
        ret = GetRange(value, 1.0 / SAMPLING_RATE, DBL_MAX, 
                       &CONFIG_RELEASE);
    }
    else if (strcmp(key, "gate") == 0)
    {
        bool gate;
        ret = GetState(value, "off", "on", &gate);
        if (gate == true && CONFIG_GATE == false)
        {
            STAGE = 0;
            STAGE_PROGRESS = 0.0;
        }
        else if (gate == false && CONFIG_GATE == true)
        {
            STAGE = 3;
            STAGE_RELEASE = STAGE_PROGRESS;
        }
        CONFIG_GATE = gate;
    }
    else
    {
        ret = 1;
        ParserError("Invalid key", line.raw);
    }

    return ret;
}

static inline void
Play(void)
{
    if (STAGE == 0)
    {
        if (STAGE_PROGRESS >= 1.0)
        {
            STAGE = 1;
            STAGE_PROGRESS = 1.0;
        }
        else
        {
            STAGE_PROGRESS += 1.0/(CONFIG_ATTACK * SAMPLING_RATE);
        }
    }
    else if (STAGE == 1)
    {
        if (STAGE_PROGRESS <= CONFIG_SUSTAIN)
        {
            STAGE = 2;
            STAGE_PROGRESS = CONFIG_SUSTAIN;
        }
        else
        {
            STAGE_PROGRESS -= CONFIG_SUSTAIN/(CONFIG_DELAY *
                                              SAMPLING_RATE);
        }
    }
    else if (STAGE == 3)
    {
        if (STAGE_PROGRESS <= 0.0)
        {
            STAGE_PROGRESS = 0.0;
        }
        else
        {
            STAGE_PROGRESS -= STAGE_RELEASE/(CONFIG_RELEASE *
                                             SAMPLING_RATE);
        }
    }
    printf("%f\n", STAGE_PROGRESS);
}

extern int 
Env(int argc, char* argv[])
{
    uint8_t ret = 0;

    if (argc == 2)
    {
        CONFIG_ATTACK = 0.5;
        CONFIG_DELAY = 0.5;
        CONFIG_SUSTAIN = 1.0;
        CONFIG_RELEASE = 0.5;
        CONFIG_GATE = false;
        ret = Generator(argv[1], Play, Config);
    }
    else
    {
        fprintf(stderr, "Usage: env NAME\n");
        fprintf(stderr, "Generates ADSR envelopes\n\n");
        fprintf(stderr, "Variables:\n");
        fprintf(stderr, "    attack: 0.0 - infinity (0.5)\n");
        fprintf(stderr, "    delay: 0.0 - infinity (0.5)\n");
        fprintf(stderr, "    sustain: 0.0 - 1.0 (1.0)\n");
        fprintf(stderr, "    release: 0.0 - infinity (0.5)\n");
        fprintf(stderr, "    gate: on/off (off)\n");
        ret = 1;
    }

    return ret;
}
