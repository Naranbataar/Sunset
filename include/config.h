#ifndef SUNSET_CONFIG_H
#define SUNSET_CONFIG_H

/* Don't use unsigned or 64bit types */
#define SAMPLING_RATE 48000
#define DEPTH_TYPE float
#define DEPTH_ISFLOAT

#include <math.h>
#define SOFT_CLIPPER tanh

#endif
