#include <stdio.h> /* fprintf, stderr */

#include "utils.h" /* GetOption */
#include "components.h" /* * */

static inline uint8_t
Usage(void)
{
    fprintf(stderr, "Usage: sunset COMPONENT ARGS\n");
    fprintf(stderr, "Components:\n");
    fprintf(stderr, "    Generation: [Generates values on @play]\n");
    fprintf(stderr, "                osc    (Oscillator)\n");
    fprintf(stderr, "                noise  (Noise Generator)\n");
    fprintf(stderr, "                env    (Envelope Generator)\n");
    fprintf(stderr, "    Processing: [Process audio values]\n");
    fprintf(stderr, "                vol    (Amplifier/Attenuator)\n");
    fprintf(stderr, "                range  (Compressor/Expander)\n");
    fprintf(stderr, "                flt    (Filters)\n");
    fprintf(stderr, "    Macros:     [Edits stream lines]\n");
    fprintf(stderr, "                cfg    (Configurator)\n");
    fprintf(stderr, "    Tools:      [Miscellaneous]\n");
    fprintf(stderr, "                mix    (Mixer)\n");
    fprintf(stderr, "                out    (WAV output)\n");
    return 1;
}

int
main(int argc, char* argv[])
{
    int ret = 0;

    if (argc > 1)
    {
        const char* keys[9] = {"osc", "noise", "env", 
                               "vol", "range", "flt",
                               "cfg",
                               "mix", "out"};
        int (*const values[9])(int, char**) = {Osc, Noise, Env,
                                               Vol, Range, Flt,
                                               Cfg, 
                                               Mix, Out};
        uint8_t component;
        if (GetOption(argv[1], keys, 9, &component) == 0)
        {
            ret = values[component](argc - 1, &(argv[1]));
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
