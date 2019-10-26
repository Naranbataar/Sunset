#include <stdio.h> /* fprintf, printf, stderr */
#include <string.h> /* strcmp */

#include "parser.h" /* LINE */
#include "utils.h" /* GetLineByType */

#include "components.h"

extern int
Cfg(int argc, char* argv[]) 
{
    uint8_t ret = 0;

    if (argc == 3)
    {
        FILE* input = fopen(argv[2], "r");
        if (input != NULL)
        {
            for (;;)
            {
                LINE line, line2;
                ret = GetLineByType(stdin, &line, LINE_TYPE_CONFIG, true);
                if (ret != 0)
                {
                    if (ret == 2)
                    {
                        ret = 0;
                    }
                    break;
                }
                else if (strcmp(line.data.config.component, argv[1]) == 0)
                {
                    ret = GetLineByType(input, &line2, LINE_TYPE_PCM, false);
                    if (ret != 0)
                    {
                        if (ret == 2)
                        {
                            ParserError("No value on FILE for config", 
                                        line.raw);
                            ret = 1;
                        }
                        break;
                    }
                    printf("#%s %s %f\n", line.data.config.key,
                                          line.data.config.value, 
                                          line2.data.value);
                }
                else
                {
                    printf("%s\n", line.raw);
                }
            }
        }
        else
        {
            ret = 1;
            ParserError("File couldn't be read", argv[2]);
        }
    }
    else
    {
        fprintf(stderr, "Usage: cfg NAME FILE \n");
        fprintf(stderr, "Replaces config lines with values from stream\n");
        fprintf(stderr, "#NAME x y -> #x y VALUE\n");
        ret = 1;
    }

    return ret;
}
