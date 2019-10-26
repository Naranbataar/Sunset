#include <math.h> /* sin */
#include <float.h> /* DBL_MAX */
#include <stdio.h> /* fprintf, printf, stderr */
#include <string.h> /* strcmp */
#include <stdint.h> /* uint8_t, uint32_t */

#include "config.h" /* SAMPLING_RATE */
#include "parser.h" /* LINE, ParserError */
#include "utils.h" /* GetRange, GetOption, Generator */
#include "consts.h" /* M_PI */

#include "components.h"

static double PI2_LUT[SAMPLING_RATE];

static double CONFIG_FREQUENCY;
static double CONFIG_DETUNING;
static double (*CONFIG_WAVEFORM)(double);

static double WAVE_PROGRESS;

static inline void
InitLUT(void)
{
    uint32_t i;
    for (i = 0; i < SAMPLING_RATE; i++)
    {
        PI2_LUT[i] = 2 * M_PI * ((double) i / SAMPLING_RATE);
    }
}

static inline double
SineWave(const double x)
{
    return sin(x);
}

static inline double
SquareWave(const double x)
{
    return (x < M_PI) ? 1.0 : -1.0;
}

static inline double
TriangleWave(const double x)
{
    return (x < M_PI) ? -1.0 + (2.0 / M_PI) * x:
                         3.0 - (2.0 / M_PI) * x;
}

static inline double
SawtoothWave(const double x)
{
    return 1.0 - ((1.0 / M_PI) * x);
}

static inline uint8_t
Config(const LINE* line)
{
    uint8_t ret = 0;
    
    const char* key = line->data.config.key;
    const char* value = line->data.config.value;
    if (strcmp(key, "frequency") == 0 || strcmp(key, "detune") == 0)
    {
        if (key[0] == 'f')
        {
            ret = GetRange(value, 1.0, SAMPLING_RATE / 2.0, 
                           &CONFIG_FREQUENCY);
        }
        else
        {
            ret = GetRange(value, -DBL_MAX, DBL_MAX, &CONFIG_DETUNING);
        }
        
        double freq = CONFIG_FREQUENCY * pow(2, CONFIG_DETUNING / 1200.0);
        if (freq > SAMPLING_RATE / 2.0 || freq < 1)
        {
            ret = 1;
            ParserError("Detuning beyond frequency range", line->raw); 
        }
    }
    else if (strcmp(key, "waveform") == 0)
    {
        const char* keys[4] = {"sin", "square", "triangle", "sawtooth"};
        double (*const values[4])(const double) = {
                SineWave, SquareWave, TriangleWave, SawtoothWave};

        uint8_t waveform;
        ret = GetOption(value, keys, 4, &waveform); 
        if (ret == 0)
        {
            CONFIG_WAVEFORM = values[waveform];
        }
    }
    else if (strcmp(key, "phase") == 0)
    {
        double phase;
        ret = GetRange(value, 0.0, 360.0, &phase);
        WAVE_PROGRESS = phase / 360.0;
    }
    else
    {
        ret = 1;
        ParserError("Invalid config key", line->raw); 
    }

    return ret;
}

static inline void
Play(void)
{
    const double freq = CONFIG_FREQUENCY * pow(2.0, CONFIG_DETUNING / 1200.0);
    const double step = freq / ((double) SAMPLING_RATE);
    const uint32_t j = (uint32_t) (WAVE_PROGRESS * SAMPLING_RATE);
    printf("%f\n", CONFIG_WAVEFORM(PI2_LUT[j]));

    const double progress = (WAVE_PROGRESS + step);
    WAVE_PROGRESS = (progress >= 1) ? progress - 1: progress;
}

extern int 
Osc(int argc, char* argv[])
{
    uint8_t ret = 0;

    if (argc == 2)
    {
        InitLUT();
        CONFIG_FREQUENCY = 440;
        CONFIG_DETUNING = 0;
        CONFIG_WAVEFORM = SineWave;

        WAVE_PROGRESS = 0;
        ret = Generator(argv[1], Play, Config);
    }
    else
    {
        double nyquist = SAMPLING_RATE / 2.0;

        fprintf(stderr, "Usage: osc NAME\n");
        fprintf(stderr, "Generates waves\n\n");
        fprintf(stderr, "Variables:\n");
        fprintf(stderr, "    frequency: 0.0 - %.1f (440 Hz)\n", nyquist); 
        fprintf(stderr, "    detune: -infinity - infinity (0 cents)\n"); 
        fprintf(stderr, "    waveform: sin, square, triangle,\n");
        fprintf(stderr, "              sawtooth (sin)\n");
        fprintf(stderr, "    phase: 0.0 - 360.0 (0 deg)\n");
        ret = 1;
    }

    return ret;
}
