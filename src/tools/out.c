#include <math.h> /* pow */
#include <stdio.h> /* FILE, fopen, fseek, fwrite, fprintf, stderr */
#include <string.h> /* memcpy */
#include <stdint.h> /* uint8_t, uint16_t, uint32_t */

#include "config.h" /* SAMPLING_RATE */
#include "parser.h" /* LINE, LINE_TYPE_*, GetLine, ParserError */

#include "components.h" 

typedef struct {
    char id[4];
    uint32_t size;
    char format[4];
} RIFF_HDR;

typedef struct {
    char id[4];
    uint32_t size;
    uint16_t format;
    uint16_t channels;
    uint32_t sample_rate;
    uint32_t byte_rate;
    uint16_t block_align;
    uint16_t bits_per_sample;
    uint16_t ext_size;
    char _pad[2];
} FMT_CHUNK;

#ifdef DEPTH_ISFLOAT
typedef struct {
    char id[4];
    uint32_t size;
    uint32_t samples;
} FACT_CHUNK;
#endif

typedef struct {
    char id[4];
    uint32_t size;
} DATA_CHUNK;

static inline void
WriteRiffHeader(FILE* output)
{
    RIFF_HDR header;
    memcpy(header.id, "RIFF", 4);
    header.size = 0;
    memcpy(header.format, "WAVE", 4);
    fwrite(&header, sizeof(RIFF_HDR), 1, output);
}

static inline void
WriteFormatChunk(FILE* output)
{
    FMT_CHUNK fmt;
    memcpy(fmt.id, "fmt ", 4);
    fmt.channels = 1;
    fmt.sample_rate = SAMPLING_RATE;
    fmt.byte_rate = SAMPLING_RATE * sizeof(DEPTH_TYPE);
    fmt.block_align = sizeof(DEPTH_TYPE);
    fmt.bits_per_sample = 8 * sizeof(DEPTH_TYPE);
    
    #ifdef DEPTH_ISFLOAT
    fmt.size = 18;
    fmt.format = 3;
    fmt.ext_size = 0;
    fwrite(&fmt, sizeof(FMT_CHUNK) - 2, 1, output);
    #else
    fmt.size = 16;
    fmt.format = 1;
    fwrite(&fmt, sizeof(FMT_CHUNK) - 4, 1, output);
    #endif
}

static inline uint8_t
WriteDataChunk(FILE* output, uint32_t* samples)
{
    uint8_t ret = 0;

    DATA_CHUNK data;
    memcpy(data.id, "data", 4);
    data.size = 0;
    fwrite(&data, sizeof(DATA_CHUNK), 1, output);

    uint32_t count = 0;
    for (;;)
    {
        const LINE line = GetLine(stdin);
        #ifndef DEPTH_ISFLOAT
        const double range = pow(2, sizeof(DEPTH_TYPE) * 8.0) - 1;
        #endif
        if (line.type == LINE_TYPE_PCM)
        {
            #ifdef DEPTH_ISFLOAT
            DEPTH_TYPE value = (DEPTH_TYPE) line.data.value;
            #else
            DEPTH_TYPE value = (DEPTH_TYPE) (line.data.value * range);
            #endif
            fwrite(&value, sizeof(DEPTH_TYPE), 1, output);
            count++;
        }
        else if (line.type == LINE_TYPE_NULL)
        {
            ret = 1;
            break;
        }
        else if (line.type == LINE_TYPE_END)
        {
            break;
        }
    }
   
    *samples = count; 
    return ret;
}

static inline void
FillSizes(FILE* output, const uint32_t samples)
{
    #ifdef DEPTH_ISFLOAT
    FACT_CHUNK fact;
    memcpy(fact.id, "fact", 4);
    fact.size = 4;
    fact.samples = samples;
    fwrite(&fact, sizeof(FACT_CHUNK), 1, output);

    const uint32_t total = 42 + (8 + (sizeof(DEPTH_TYPE) * samples));
    #else
    const uint32_t total = 28 + (8 + (sizeof(DEPTH_TYPE) * samples));
    #endif

    fseek(output, 4, SEEK_SET);
    fwrite(&total, 4, 1, output);
   
    #ifdef DEPTH_ISFLOAT 
    fseek(output, 42, SEEK_SET);  
    #else
    fseek(output, 40, SEEK_SET); 
    #endif
    
    const uint32_t dsize = samples * sizeof(DEPTH_TYPE);
    fwrite(&dsize, 4, 1, output);
}

extern int
Out(int argc, char* argv[])
{
    uint8_t ret = 0;

    if (argc == 2)
    {
        ret = SetCaller("writer");
        if (ret == 0)
        {
            FILE* output = fopen(argv[1], "wb");
            if (output != NULL)
            {
                WriteRiffHeader(output);
                WriteFormatChunk(output);

                uint32_t samples;
                ret = WriteDataChunk(output, &samples);

                FillSizes(output, samples);
                fclose(output);
            }
            else
            {
                ParserError("Couldn't open file for writing", argv[1]);
            }
        }
    }
    else
    {
        fprintf(stderr, "Usage: out FILE\n");
        fprintf(stderr, "Writes data to a %dhz %zu-bit ", 
                SAMPLING_RATE, sizeof(DEPTH_TYPE) * 8);
        #ifdef DEPTH_ISFLOAT
        fprintf(stderr, "float ");
        #endif
        fprintf(stderr, "WAV file\n");
        ret = 1;
    }
    
    return ret;
}
