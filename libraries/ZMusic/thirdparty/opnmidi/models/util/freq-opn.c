#include <math.h>
#include <stdio.h>
#include <stdint.h>

#define BEND_COEFFICIENT                321.88557

static double toneToHerz(double tone)
{
    return BEND_COEFFICIENT * exp(0.057762265 * tone);
}

static const size_t s_freqs_size = 12000;
static uint16_t s_freqs[12000];

int main()
{
    double i, res, prev;
    uint16_t freq_val = 0, freq_val_prev, diff = 0;
    uint32_t octave = 0;
    size_t idx = 0, size = 0;
    const double shift = 31.933413 / 3762.0;

    // 30.823808

//    for(i = 0.0; i < 35.0; i += 0.0000001)
//    {
//        res = toneToHerz(i);
//        if((uint16_t)res != freq_val)
//        {
//            printf("Coefficient= %25f, tone = %15f, res=%15f [idx=%zu]\n", i - prev, i, res, idx);
//            s_freqs[idx++] = freq_val = (uint16_t)res;
//            prev = i;
//        }
//
//        if(res >= 2036.0001)
//            break;
//    }

    printf("Shift value is = %25.10f\n", shift);

    idx = 0;
    freq_val = 0;

    for(i = 0.0; i < 32.0; i += shift)
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
        freq_val_prev = freq_val;
        diff = (uint16_t)res - freq_val;
//        printf("Coefficient= %25f, tone = %15f, Hz = 0x%03X\n", i - prev, i, (uint16_t)res);
        s_freqs[idx++] = freq_val = (uint16_t)res;
        prev = i;

        if(idx >= s_freqs_size)
        {
            printf("OUCH! IDX Overflow\n\n");
            return 1;
        }

        if(diff > 1)
        {
            printf("OUCH! (Diff=%u: %g (%u) - %u, %u, i=%15f)\n\n", diff, res, (uint16_t)res, freq_val_prev, (uint16_t)res - freq_val_prev, i);
            return 1;
        }

        if(freq_val >= 0x7FF)
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

    // 2047 is the maximum OPN frequency!

//    for(i = 0.0; i < 30.823808; i += 0.000001)
//    {
//
//        printf("Tone %15.6f -> %10.5f Hz\n", i, toneToHerz(i));
//
//        if(toneToHerz(i) >= 2047.0001)
//            break;
//    }
//
    printf("\n};\n\n");

    return 0;
}
