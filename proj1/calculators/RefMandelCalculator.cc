/**
 * @file RefMandelCalculator.cc
 * @author Vojtech Mrazek (mrazek@fit.vutbr.cz)
 * @author Marta Jaros (jarosmarta@fit.vutbr.cz)
 * @brief Naive implementation
 * @date 2025-10-09
 */
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

#include "RefMandelCalculator.h"

RefMandelCalculator::RefMandelCalculator(unsigned matrixBaseSize, unsigned limit) : BaseMandelCalculator(matrixBaseSize, limit, "RefMandelCalculator")
{
    data = (int *)(malloc(height * width * sizeof(int)));
}

RefMandelCalculator::~RefMandelCalculator()
{
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

int *RefMandelCalculator::calculateMandelbrot()
{
    int *pdata = data;
    float ii = 0.0f;
    
    for (int i = 0; i < height; i++, ii++)
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
    return data;
}

