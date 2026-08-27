#include <math.h>
#include <stdio.h>
#include <stdint.h>

#define BEND_COEFFICIENT                172.4387

static double toneToHerz(double tone)
{
    return BEND_COEFFICIENT * exp(0.057762265 * tone);
}

static uint16_t s_freqs[3000];

int main()
{
    double i, res, prev;
    uint16_t freq_val = 0, diff = 0;
    uint32_t octave = 0;
    size_t idx = 0, size = 0;
    const double shift = 30.823808 / 1821.0;

    // 30.823808

//    for(i = 0.0; i < 30.823808; i += 0.000001)
//    {
//        res = toneToHerz(i);
//        if((uint16_t)res != freq_val)
//        {
//            printf("Coefficient= %25f, tone = %15f\n", i - prev, i);
//            s_freqs[idx++] = freq_val = (uint16_t)res;
//            prev = i;
//        }
//
//        if(res >= 1023.0001)
//            break;
//    }

    printf("Shift value is = %25.10f\n", shift);

    for(i = 0.0; i < 31.0; i += shift)
    {
        res = toneToHerz(i);

        if(freq_val == 0) // First entry!
            freq_val = (uint16_t)res;
//        if((uint16_t)res != freq_val)
//        {
//            printf("Coefficient= %25f, tone = %15f, Hz = 0x%03X\n", i - prev, i, (uint16_t)res);
//            s_freqs[idx++] = freq_val = (uint16_t)res;
//            prev = i;
//        }
        diff = (uint16_t)res - freq_val;
//        printf("Coefficient= %25f, tone = %15f, Hz = 0x%03X\n", i - prev, i, (uint16_t)res);
        s_freqs[idx++] = freq_val = (uint16_t)res;
        prev = i;

        if(diff > 1)
        {
            printf("OUCH! (Diff=%u)\n\n", diff);
            return 1;
        }

        if(freq_val >= 0x3FF)
        {
            printf("Biggest tone value = %25f\n", i);
            break;
        }
    }

    size = idx;

    printf("uint16_t xxx[%zu] = \n{", size);

    for(idx = 0; idx < size; ++idx)
    {
        if(idx % 10 == 0)
            printf("\n    ");

        printf("0x%03X, ", s_freqs[idx]);
    }

    // 1023 is the maximum OPL frequency!

//    for(i = 0.0; i < 30.823808; i += 0.000001)
//    {
//        /*

//
//        printf("Tone %15.6f -> %10.5f Hz\n", i, toneToHerz(i));
//
//        if(toneToHerz(i) >= 1023.0001)
//            break;
//    }
//
    printf("\n};\n\n");

    return 0;
}
