#include <math.h> /* pow, round */
#include <float.h> /* DBL_MAX */
#include <stdio.h> /* printf */
#include <string.h> /* strcmp */
#include <stdlib.h> /* strtod */
#include <stdint.h> /* uint8_t, uint16_t */
#include <stdbool.h> /* bool, true, false */

#include "config.h" /* SAMPLING_RATE */
#include "parser.h" /* LINE, LINE_TYPE_*, GetLine, ParserError */

#include "utils.h"

extern uint8_t
GetState(const char* string, const char* low, const char* high, bool* out)
{
    uint8_t ret = 0;

    if (strcmp(string, high) == 0)
    {
        *out = true;
    }
    else if (strcmp(string, low) == 0)
    {
        *out = false;
    }
    else
    {
        ret = 1;
        ParserError("Invalid state", string);
    }

    return ret;
}

extern uint8_t
GetRange(const char* string, const double min, const double max, double* out)
{
    uint8_t ret = 1;

    char* end = NULL;
    const double value = strtod(string, &end);

    if (end != NULL)
    {
        if (*end == '\0')
        {
            if (value >= min && value <= max)
            {
                ret = 0;
                *out = value;
            }
            else
            {
                ret = 2;
                ParserError("Invalid range for this key", string);
            }
        }
    }

    if (ret == 1)
    {
        ParserError("Invalid number", string);
    }
    
    return (ret >= 1) ? 1 : 0;
}

extern uint8_t
GetOption(const char* string, const char* keys[], 
          const uint8_t length, uint8_t* out)
{
    uint8_t ret = 0;

    uint16_t i;
    bool success = false;
    for (i = 0; i < length; i++)
    {
        if (strcmp(string, keys[i]) == 0)
        {
            *out = (uint8_t) i;
            success = true;
            break;
        }
    }

    if (success == false)
    {
        ret = 1;
        ParserError("Invalid option", string);
    }

    return ret;
}

extern uint8_t
GetLineByType(FILE* input, LINE* out, LINE_TYPE type, bool print)
{
    uint8_t ret = 0;

    if (input != NULL)
    {
        out->type = LINE_TYPE_NULL;
        while (out->type != type)
        {
            *out = GetLine(input);
            if (out->type == LINE_TYPE_NULL)
            {
                ret = 1;
                break;
            }
            else if (out->type == LINE_TYPE_END)
            {
                ret = 2;
                break;
            }
            else if (out->type != type)
            {
                if (print)
                {
                    printf("%s\n", out->raw);
                }
            }
        }
     }
    else
    {
        ret = 1;
    }
    
    return ret;
}

extern uint8_t
Generator(const char* name, void (*const play)(void), 
          uint8_t (*const config)(const LINE*))
{
    uint8_t ret = SetCaller(name);

    if (ret == 0)
    {
        for (;;)
        {
            const LINE line = GetLine(stdin);

            if (line.type == LINE_TYPE_EVENT)
            {
                printf("%s\n", line.raw);

                const char* key = line.data.event.key;
                const char* value = line.data.event.value;
                if (strcmp(key, "play") == 0)
                {
                    double time = 0;
                    if (strcmp(value, "-") == 0)
                    {
                        time = 1;
                    }
                    else 
                    {
                        const double step = 1.0 / SAMPLING_RATE;
                        ret = GetRange(value, step, DBL_MAX, &time);
                        time *= SAMPLING_RATE;
                        time = round(time);
                    }
                        
                    double i;
                    for (i = 0; i < time; i++)
                    {
                        play();
                    }
                }
            }
            else if (line.type == LINE_TYPE_CONFIG)
            {
                printf("%s\n", line.raw);
                if (strcmp(line.data.config.component, name) == 0)
                {
                    ret = config(&line);
                }
            }
            else if (line.type == LINE_TYPE_PCM)
            {
                ret = 1;
                ParserError("Generators can't handle audio values", 
                            line.raw);
            }
            else if (line.type == LINE_TYPE_NULL)
            {
                ret = 1;
            }
            else if (line.type == LINE_TYPE_END)
            {
                break;
            }

            if (ret != 0)
            {
                break;
            }
        }
    }
    
    return ret;
}

extern uint8_t
Processor(const char* name, void (*const process)(const double), 
          uint8_t (*const config)(const LINE*))
{
    uint8_t ret = SetCaller(name);

    if (ret == 0)
    {
        for (;;)
        {
            const LINE line = GetLine(stdin);

            if (line.type == LINE_TYPE_EVENT)
            {
                printf("%s\n", line.raw);
            }
            else if (line.type == LINE_TYPE_CONFIG)
            {
                printf("%s\n", line.raw);
                if (strcmp(line.data.config.component, name) == 0)
                {
                    ret = config(&line);
                }
            }
            else if (line.type == LINE_TYPE_PCM)
            {
                process(line.data.value);
            }
            else if (line.type == LINE_TYPE_NULL)
            {
                ret = 1;
            }
            else if (line.type == LINE_TYPE_END)
            {
                break;
            }

            if (ret != 0)
            {
                break;
            }
        }
    }

    return ret;
}

