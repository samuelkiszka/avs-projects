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

#define ALIGNMENT 32


LineMandelCalculator::LineMandelCalculator (unsigned matrixBaseSize, unsigned limit) :
    BaseMandelCalculator(matrixBaseSize, limit, "LineMandelCalculator")
{
    data = (int *)(aligned_alloc(ALIGNMENT, height * width * sizeof(int)));
    temp = (float *)(aligned_alloc(ALIGNMENT, 2 * width * sizeof(float)));
}

LineMandelCalculator::~LineMandelCalculator() {
    free(data);
    free(temp);
    data = NULL;
    temp = NULL;
}

int * LineMandelCalculator::calculateMandelbrot () {
    int *pdata = data;
    float *ptemp = temp;
    float ii = 0.0f;

    for (int i = 0; i < height / 2; i++, ii++)
    {
        float y = y_start + ii * dy;
        int vals[width];
        #pragma omp simd aligned(ptemp:ALIGNMENT)
        for (int j = 0; j < width; j++) {
            ptemp[j] = x_start + j * dx;
            ptemp[j + width] = y;
            vals[j] = 0;
        }

        for (int n = 0; n < limit; n++) {
            #pragma omp simd aligned(ptemp:ALIGNMENT)
            for (int j = 0; j < width; j++) {
                float zReal = ptemp[j];
                float zImag = ptemp[j + width];

                float r2 = zReal * zReal;
                float i2 = zImag * zImag;

                if (r2 + i2 < 4.0f){
                    ptemp[j] = r2 - i2 + x_start + j * dx;
                    ptemp[j + width] = 2.0f * zReal * zImag + y;
                    vals[j]++;
                }
            }
        }

        #pragma omp simd aligned(pdata:ALIGNMENT)
        for (int j = 0; j < width; j++) {
            *(pdata++) = vals[j];
        }
    }

    int i = height / 2;
    while(i--) {
        #pragma omp simd aligned(data:ALIGNMENT) aligned(pdata:ALIGNMENT)
        for (int j = 0; j < width; j++)
            *(pdata++) = *(data + i * width + j);
    }

    return data;
}
