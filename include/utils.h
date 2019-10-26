#ifndef SUNSET_UTILS_H
#define SUNSET_UTILS_H

#include <stdio.h> /* FILE */
#include <stdbool.h> /* bool */
#include "parser.h" /* LINE, LINE_TYPE */

uint8_t GetState(const char* string, const char* low, 
                 const char* high, bool* out);
uint8_t GetRange(const char* string, double min, double max, double* out);
uint8_t GetOption(const char* string, const char* keys[], 
                  uint8_t length, uint8_t* out);
uint8_t GetLineByType(FILE* input, LINE* out, LINE_TYPE type, 
                      bool print);

uint8_t Generator(const char* name, void (*play)(void), 
                  uint8_t (*config)(const LINE*));
uint8_t Processor(const char* name, void (*process)(double), 
                  uint8_t (*config)(const LINE*));
uint8_t Mixer(const char* file, void (*mix)(double, double)); 
#endif
