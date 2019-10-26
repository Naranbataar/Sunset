#ifndef SUNSET_PARSER_H
#define SUNSET_PARSER_H

#define STR2(x) #x
#define STR(x) STR2(x)

#define LINE_L 80
#define IDENTIFIER_L 16
#define EV_VALUE_L 64
#define CFG_VALUE_L 48

#include <stdio.h> /* FILE */
#include <stdint.h> /* uint*_t, int*_t */
#include <stdbool.h> /* bool */

typedef struct {
    char component[IDENTIFIER_L];
    char key[IDENTIFIER_L];
    char value[CFG_VALUE_L];
} LINE_CONFIG;

typedef struct {
    char key[IDENTIFIER_L];
    char value[EV_VALUE_L];
} LINE_EVENT;

typedef enum {
    LINE_TYPE_NULL,
    LINE_TYPE_PCM,
    LINE_TYPE_EVENT,
    LINE_TYPE_CONFIG,
    LINE_TYPE_END
} LINE_TYPE;

typedef union {
    double value;
    LINE_EVENT event;
    LINE_CONFIG config;
} LINE_DATA;

typedef struct {
    uint8_t _pad[4];
    char raw[LINE_L];
    LINE_TYPE type;
    LINE_DATA data;
} LINE;

void ParserError(const char* err, const char* line);
LINE GetLine(FILE* file);
uint8_t SetCaller(const char* caller);

#endif
