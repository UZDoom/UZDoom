#include <math.h>
#include <stdio.h>
#include <stdint.h>


static inline uint_fast32_t brightnessToOPN(uint_fast32_t brightness)
{
    double b = (double)(brightness);
    double ret = round(127.0 * sqrt(b * (1.0 / 127.0)));
    return (uint_fast32_t)(ret);
}

int main()
{
    unsigned int i;

    printf("{");

    for(i = 0; i < 128; ++i)
    {
        if(i % 10 == 0)
            printf("\n    ");

        printf("%3lu,", brightnessToOPN(i));
    }

    printf("\n};\n\n");

    return 0;
}
