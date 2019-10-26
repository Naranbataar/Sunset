#include <math.h> /* sin */
#include <float.h> /* DBL_MAX */
#include <stdio.h> /* fprintf, printf, stderr */
#include <string.h> /* strcmp */
#include <stdint.h> /* uint8_t */
#include <stdbool.h> /* bool, true, false */

#include "parser.h" /* ParserError */
#include "utils.h" /* GetOption */
#include "consts.h" /* M_PI */

#include "components.h"

static inline void
Add(const double x, const double y)
{
    printf("%f\n", x + y);
}

static inline void
Sub(const double x, const double y)
{
    printf("%f\n", x - y);
}

static inline void
Mul(const double x, const double y)
{
    printf("%f\n", x * y);
}

static inline void
Div(double x, double y)
{
    printf("%f\n", x / y);
}

static inline void
FM(const double x, const double y)
{
    printf("%f\n", sin((2 * M_PI * x) + sin(2 * M_PI * y)));
}

static inline uint8_t
RunMix(const char* file, void (*const mix)(const double, const double))
{
    uint8_t ret = 0;

    FILE* input = fopen(file, "r");
    if (input != NULL)
    {
        bool active = true;
        for (;;)
        {
            double y = 0;
            LINE line, line2;

            ret = GetLineByType(stdin, &line, LINE_TYPE_PCM, true);
            if (ret != 0)
            {
                break;
            }

            if (active)
            {
                ret = GetLineByType(input, &line2, LINE_TYPE_PCM, false);
                if (ret == 0)
                {
                    y = line2.data.value;
                }
                else if (ret == 1)
                {
                    break;
                }
                else
                {
                    active = false;
                }
            }

            mix(line.data.value, y);
        }
    }
    else
    {
        ret = 1;
        ParserError("File couldn't be read", file);
    }
    return (ret == 1) ? 1 : 0;
}

static inline uint8_t
RunMixC(const char* value, void (*const mix)(const double, const double))
{
    uint8_t ret = 0;

    double y;
    ret = GetRange(value, -DBL_MAX, DBL_MAX, &y);
    if (ret == 0)
    {
        for (;;)
        {
            LINE line;
            ret = GetLineByType(stdin, &line, LINE_TYPE_PCM, NULL);
            if (ret != 0)
            {
                break;
            }
            mix(line.data.value, y);
        }
    }

    return (ret == 1) ? 1 : 0;
}
static inline uint8_t
Usage(void)
{
    fprintf(stderr, "Usage: mix [-c] OP ARG\n");
    fprintf(stderr, "Mixing operations (no clipping)\n\n");
    fprintf(stderr, "OPs: add, sub, mul, div, fm\n\n");
    fprintf(stderr, "If -c provided, ARG is a constant\n");
    fprintf(stderr, "    otherwise is a file\n");
    return 1;
}

extern int
Mix(int argc, char* argv[])
{
    uint8_t ret = 0;
    
    if (argc >= 3)
    {
        const char* keys[5] = {"add", "sub", "mul", "div", "fm"};
        void (*const values[5])(const double, const double) = {
                Add, Sub, Mul, Div, FM};
        uint8_t mode;

        if (strcmp(argv[1], "-c") == 0)
        {
            if (argc == 4)
            {
                ret = GetOption(argv[2], keys, 5, &mode);
                if (ret == 0)
                {
                    ret = RunMixC(argv[3], values[mode]);
                }
            }
            else
            {
                ret = Usage();
            }
        }
        else if (argc == 3)
        {
            ret = GetOption(argv[1], keys, 5, &mode);
            if (ret == 0)
            {
                ret = RunMix(argv[2], values[mode]);
            }
        }
        else
        {
            ret = Usage();
        }
    }
    else
    {
        ret = Usage();
    }

    return ret;
}
