/**
 * @file LineMandelCalculator.cc
 * @author FULL NAME <xlogin00@stud.fit.vutbr.cz>
 * @brief Implementation of Mandelbrot calculator that uses SIMD paralelization over lines
 * @date DATE
 */
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

#include <stdlib.h>


#include "LineMandelCalculator.h"


LineMandelCalculator::LineMandelCalculator (unsigned matrixBaseSize, unsigned limit) :
    BaseMandelCalculator(matrixBaseSize, limit, "LineMandelCalculator")
{
    data = (int *)(malloc(height * width * sizeof(int)));
}

LineMandelCalculator::~LineMandelCalculator() {
    free(data);
    data = NULL;
}

template <typename T>
static inline int mandelbrot(T real, T imag, int limit)
{
    T zReal = real;
    T zImag = imag;

    for (int i = 0; i < limit; ++i)
    {
        T r2 = zReal * zReal;
        T i2 = zImag * zImag;

        if (r2 + i2 > 4.0f)
            return i;

        zImag = 2.0f * zReal * zImag + imag;
        zReal = r2 - i2 + real;
    }
    return limit;
}


int * LineMandelCalculator::calculateMandelbrot () {
    int *pdata = data;
    float ii = 0.0f;

    for (int i = 0; i < height / 2; i++, ii++)
    {
        float jj = 0.0f;
        for (int j = 0; j < width; j++, jj++)
        {
            float x = x_start + jj * dx; // current real value
            float y = y_start + ii * dy; // current imaginary value

            int value = mandelbrot(x, y, limit);

            *(pdata++) = value;
        }
    }

    int line = height / 2;
    while(line--) {
        for (int line_pos = 0; line_pos < width; line_pos++)
            *(pdata++) = *(data + line * width + line_pos);
    }

    return data;
}
