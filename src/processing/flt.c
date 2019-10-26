#include <math.h> /* sqrt, fabs, pow */
#include <stdio.h> /* fprintf, printf, stderr */
#include <string.h> /* strcmp */
#include <stdint.h> /* uint8_t */

#include "config.h" /* SAMPLING_RATE, SOFT_CLIPPER */
#include "parser.h" /* LINE, ParserError */
#include "utils.h" /* GetRange, GetOption */
#include "consts.h" /* M_PI, M_SQRT2 */

#include "components.h"

typedef enum { LOWPASS, HIGHPASS, BANDPASS, BANDSTOP, 
               GAUSSIAN, LOWSHELF, HIGHSHELF } FILTER_TYPE;

static double CONFIG_FREQUENCY;
static double CONFIG_DAMPING;
static double CONFIG_PEAK;
static FILTER_TYPE CONFIG_TYPE;

static double CACHE_A[3];
static double CACHE_B[3];

static double CACHE_Y[3];

static inline void
BasicFilters(const double Q, const double K)
{
    const double KK = K * K;
    const double N = 1 / (1 + (K / Q) + KK);
 
    CACHE_B[0] = 1;
    CACHE_B[1] = 2 * (KK - 1) * N;
    CACHE_B[2] = (1 - (K / Q) + KK) * N;
        
    if (CONFIG_TYPE == LOWPASS)
    {    
        CACHE_A[0] = KK * N;
        CACHE_A[1] = 2 * CACHE_A[0];
        CACHE_A[2] = CACHE_A[0];
    }
    else if (CONFIG_TYPE == HIGHPASS)
    {
        CACHE_A[0] = N;
        CACHE_A[1] = -2 * CACHE_A[0];
        CACHE_A[2] = CACHE_A[0];
    }
    else if (CONFIG_TYPE == BANDPASS)
    { 
        CACHE_A[0] = K / Q * N;
        CACHE_A[1] = 0;
        CACHE_A[2] = -CACHE_A[0];
    } 
    else if (CONFIG_TYPE == BANDSTOP)
    {
        CACHE_A[0] = (1 + KK) * N;
        CACHE_A[1] = CACHE_B[1];
        CACHE_A[2] = CACHE_A[0];
    }
}

static inline void
GaussianFilter(const double Q, const double K)
{
    const double V = pow(10, fabs(CONFIG_PEAK) / 20.0);
    const double KK = K * K;

    const double Q1 = (CONFIG_PEAK >= 0) ? (1 / Q) : (V / Q);
    const double Q2 = (CONFIG_PEAK >= 0) ? (V / Q) : (1 / Q);
    
    const double N = 1 / (1 + (Q1 * K) + KK);
    CACHE_A[0] = (1 + (Q2 * K) + KK) * N;
    CACHE_A[1] = 2 * (KK - 1) * N;
    CACHE_A[2] = (1 - (Q2 * K) + KK) * N;
    CACHE_B[0] = 1;
    CACHE_B[1] = CACHE_A[1];
    CACHE_B[2] = (1 - (Q1 * K) + KK) * N;
}

static inline void
ShelfFilters(const double K)
{
    const double V = pow(10, fabs(CONFIG_PEAK) / 20.0);
    const double KK = K * K;
    
    const double S1 = (CONFIG_PEAK >= 0) ? sqrt(2 * V) : M_SQRT2;
    const double S2 = (CONFIG_PEAK >= 0) ? M_SQRT2 : sqrt(2 * V);
    
    CACHE_B[0] = 1;
    if (CONFIG_TYPE == LOWSHELF)
    {        
        const double KK1 = (CONFIG_PEAK >= 0) ? KK : (V * KK);
        const double KK2 = (CONFIG_PEAK >= 0) ? (V * KK) : KK;
            
        const double NL = 1 / (1 + (S2 * K) + KK1);
        CACHE_A[0] = (1 + (S1 * K) + KK2) * NL;
        CACHE_A[1] = 2 * (KK2 - 1) * NL;
        CACHE_A[2] = (1 - (S1 * K) + KK2) * NL;
        CACHE_B[1] = 2 * (KK1 - 1) * NL;
        CACHE_B[2] = (1 - (S2 * K) + KK1) * NL;
    }
    else if (CONFIG_TYPE == HIGHSHELF)
    {
        const double V1 = (CONFIG_PEAK >= 0) ? V : 1;
        const double V2 = (CONFIG_PEAK >= 0) ? 1 : V;
        const double KKM1 = (CONFIG_PEAK >= 0) ? (KK - V) : (KK - 1);
        const double KKM2 = (CONFIG_PEAK >= 0) ? (KK - 1) : (KK - V);

        const double NH = 1 / (V2 + (S2 * K) + KK);
        CACHE_A[0] = (V1 + (S1 * K) + KK) * NH;
        CACHE_A[1] = 2 * KKM1 * NH;
        CACHE_A[2] = (V1 - (S1 * K) + KK) * NH;
        CACHE_B[1] = 2 * KKM2 * NH;
        CACHE_B[2] = (V2 - (S2 * K) + KK) * NH;
    }
}

static inline void
UpdateCoeffs(void)
{
    const double Q = 1 / (2 * CONFIG_DAMPING);
    const double K = (M_PI * (CONFIG_FREQUENCY / SAMPLING_RATE));
    if (CONFIG_TYPE < GAUSSIAN)
    {
        BasicFilters(Q, K);
    }
    else if (CONFIG_TYPE == GAUSSIAN)
    {
        GaussianFilter(Q, K);
    }
    else
    {
        ShelfFilters(K);
    }
}

static inline uint8_t
Config(const LINE line)
{
    uint8_t ret = 0;

    const char* key = line.data.config.key;
    const char* value = line.data.config.value;
    if (strcmp(key, "frequency") == 0)
    {
        ret = GetRange(value, 1.0, SAMPLING_RATE / 2.0, 
                          &CONFIG_FREQUENCY);
    }
    else if (strcmp(key, "damping") == 0)
    {
        ret = GetRange(value, 0.0, 1.0, &CONFIG_DAMPING);
    }
    else if (strcmp(key, "mode") == 0)
    {
        const char* keys[7] = {"lowpass", "highpass", "bandpass", "bandstop",
                               "gaussian", "lowshelf", "highshelf"};
        FILTER_TYPE values[7] = {LOWPASS, HIGHPASS, BANDPASS, BANDSTOP,
                                 GAUSSIAN, LOWSHELF, HIGHSHELF};

        uint8_t option;
        ret = GetOption(value, keys, 7, &option);
        if (ret == 0)
        {
            CONFIG_TYPE = values[option];
        }
    }
    else if (strcmp(key, "peak") == 0)
    {
        ret = GetRange(value, -30.0, 30.0, &CONFIG_PEAK);
    }
    else
    {
        ret = 1;
        ParserError("Invalid config key", line.raw);
    }

    if (ret == 0)
    {
        UpdateCoeffs();
    }

    return ret;
}

static inline void
Process(const double x)
{
    const double r = (CACHE_B[0] * x) - (CACHE_B[1] * CACHE_Y[0]) - 
                     (CACHE_B[2] * CACHE_Y[1]);
    const double y = (CACHE_A[0] * r) + (CACHE_A[1] * CACHE_Y[0]) +
                     (CACHE_A[2] * CACHE_Y[1]);

    CACHE_Y[1] = CACHE_Y[0];
    CACHE_Y[0] = r;

    printf("%f\n", SOFT_CLIPPER(y));
}

extern int
Flt(int argc, char* argv[])
{
    uint8_t ret = 0;

    if (argc == 2)
    {
        CONFIG_FREQUENCY = 1;
        CONFIG_DAMPING = 0.707;
        CONFIG_PEAK = 0;
        CONFIG_TYPE = HIGHPASS;
        UpdateCoeffs();

        ret = Processor(argv[1], Process, Config);
    }
    else
    {
        fprintf(stderr, "Usage: flt NAME\n");
        fprintf(stderr, "Filters signals\n");
        fprintf(stderr, "Variables:\n");
        fprintf(stderr, "    frequency: 1.0 - %.1fHz (1.0)\n", 
                SAMPLING_RATE / 2.0);
        fprintf(stderr, "    damping: 0.0 - 1.0 (0.707) [except shelves] \n");
        fprintf(stderr, "    mode: lowpass, highpass, bandpass, bandstop, \n");
        fprintf(stderr, "          gaussian, lowshelf, highshelf (highpass) \n");
        fprintf(stderr, "    peak: -30 - 30dB (0) [only for gaussian and");
        fprintf(stderr, " shelves]\n");
        ret = 1;
    }

    return ret;
}
