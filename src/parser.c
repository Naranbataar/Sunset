#include <stdio.h> /* fprintf, stderr, fgets, sscanf, stderr */
#include <string.h> /* strcspn, strlen, memcpy */
#include <stdint.h> /* uint8_t, uint32_t */
#include <stdlib.h> /* strtod */
#include <stdbool.h> /* bool, true, false */

#include "parser.h" /* LINE, LINE_TYPE_*, LINE_L, IDENTIFIER_L, 
                       EV_SSCANF, CFG_SSCANF */

static char CALLER[IDENTIFIER_L] = "";

extern void
ParserError(const char* err, const char* line)
{
    if (CALLER[0] == '\0')
    {
        fprintf(stderr, "sunset: error: %s\n", err);
    }
    else
    {
        fprintf(stderr, "sunset(%s): error: %s\n", CALLER, err);
    }
    fprintf(stderr, "    %s\n\n", line);
}

static inline LINE
GetPCM(const char line[static LINE_L])
{
    LINE ret;
    ret.type = LINE_TYPE_NULL;

    char* end = NULL;
    ret.data.value = strtod(line, &end);

    if (end != NULL)
    {
        if (end[0] == '\0')
        {
            ret.type = LINE_TYPE_PCM;
        }
    }

    if (ret.type == LINE_TYPE_NULL)
    {
        ParserError("Invalid value", line);
    }

    return ret;
}

static inline LINE
GetEvent(const char line[static LINE_L])
{
    LINE ret = {{0}, {0}, 0, {0}};
    ret.type = LINE_TYPE_NULL;

    #ifndef EV_SSCANF
    #define EV_SSCANF "@ %"STR(IDENTIFIER_L)"s%c%"STR(EV_VALUE_L)"s%c%"\
                       STR(LINE_L)"s"
    #endif

    char sep = '\0';
    char end = '\0';
    char trailing[LINE_L] = "";
    sscanf(line, EV_SSCANF, ret.data.event.key,
           &sep, ret.data.event.value, &end, trailing);
    
    if (ret.data.event.key[0] == '\0')
    {
        ParserError("Event without key", line);
    }
    else if (ret.data.event.value[0] == '\0')
    {
        ParserError("Event without value", line);
    }
    else if (sep != ' ' && sep != '\t')
    {
        ParserError("Event key is longer than " STR(IDENTIFIER_L)
                    " chars", line);
    }
    else if (end != ' ' && end != '\t' && end != '\0')
    {
        ParserError("Event value is longer than " STR(EV_VALUE_L)
                    " chars", line);
    }
    else if (strlen(trailing) > 0)
    {
        ParserError("Event with too many args", line);
    }
    else
    {
        ret.type = LINE_TYPE_EVENT;
    }
    
    return ret;
}

static inline LINE
GetConfig(const char line[static LINE_L]) 
{
    LINE ret = {{0}, {0}, 0, {0}};
    ret.type = LINE_TYPE_NULL;

    #ifndef CFG_SSCANF
    #define CFG_SSCANF "# %"STR(IDENTIFIER_L)"s%c%"STR(IDENTIFIER_L)"s%c%" \
                       STR(CFG_VALUE_L)"s%c%"STR(LINE_L)"s"
    #endif

    char sep = '\0';
    char sep2 = '\0';
    char end = '\0';
    char trailing[LINE_L] = "";
    sscanf(line, CFG_SSCANF, ret.data.config.component,
           &sep, ret.data.config.key, &sep2, ret.data.config.value,
           &end, trailing);
 
    if (ret.data.config.component[0] == '\0')
    {
        ParserError("Config without component", line);
    }
    else if (ret.data.config.key[0] == '\0')
    {
        ParserError("Config without key", line);
    }
    else if (ret.data.config.value[0] == '\0')
    {
        ParserError("Config without value", line);
    }
    else if (sep != ' ' && sep != '\t')
    {
        ParserError("Config component is longer than " STR(IDENTIFIER_L)
                    " chars", line);
    }
    else if (sep2 != ' ' && sep2 != '\t')
    {
        ParserError("Config key is longer than " STR(IDENTIFIER_L)
                    " chars", line);
    }
    else if (end != ' ' && end != '\t' && end != '\0')
    {
        ParserError("Config value is longer than " STR(CFG_VALUE_L)
                    " chars", line);
    }
    else if (strlen(trailing) > 0)
    {
        ParserError("Config with too many args", line);
    }
    else
    {
        ret.type = LINE_TYPE_CONFIG;
    }
    
    return ret;
}

extern uint8_t
SetCaller(const char* caller)
{
    uint8_t ret = 1;

    if (strlen(caller) <= IDENTIFIER_L &&
        caller[0] != '/' && caller[0] != '#' && caller[0] != '@')
    {
        ret = 0;
        memcpy(CALLER, caller, IDENTIFIER_L * sizeof(char));
    }
    else
    {
        ParserError("Invalid Name", caller);
    }

    return ret;
}

extern LINE
GetLine(FILE* file)
{
    LINE ret;
    ret.type = LINE_TYPE_NULL;
   
    bool broken = false; 
    char buffer[LINE_L];
    while (fgets(buffer, LINE_L, file) != NULL)
    {
        if (buffer[0] != '/' && buffer[0] != '\n' && buffer[0] != '\0')
        {
            if (buffer[strlen(buffer) - 1] == '\n')
            {
                buffer[strlen(buffer) - 1] = '\0';
                if (buffer[0] == '#')
                {
                    ret = GetConfig(buffer);
                }
                else if (buffer[0] == '@')
                {
                    ret = GetEvent(buffer);
                }
                else
                {
                    ret = GetPCM(buffer);
                }
                memcpy(ret.raw, buffer, LINE_L * sizeof(char));
            }
            else
            {
                ParserError("Line is too long", buffer);
            }
            broken = true;
            break;
        }
    }

    if (!broken)
    {
        ret.type = LINE_TYPE_END;
    }

    return ret;
}
