/**
 * @file LineMandelCalculator.cc
 * @author FULL NAME <xkiszk00@stud.fit.vutbr.cz>
 * @brief Implementation of Mandelbrot calculator that uses SIMD paralelization over lines
 * @date DATE
 */
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <string.h>

#include <stdlib.h>


#include "LineMandelCalculator.h"

#define ALIGNMENT 512
#define SIMD_WIDTH 32


LineMandelCalculator::LineMandelCalculator (unsigned matrixBaseSize, unsigned limit) :
    BaseMandelCalculator(matrixBaseSize, limit, "LineMandelCalculator")
{
    data = static_cast<int *>(aligned_alloc(ALIGNMENT, height * width * sizeof(int)));
    tempr = static_cast<float *>(aligned_alloc(ALIGNMENT, width * sizeof(float)));
    tempi = static_cast<float *>(aligned_alloc(ALIGNMENT, width * sizeof(float)));
}

LineMandelCalculator::~LineMandelCalculator() {
    free(data);
    free(tempr);
    free(tempi);
    data = NULL;
    tempr = NULL;
    tempi = NULL;
}

int * LineMandelCalculator::calculateMandelbrot () {
    int *__restrict__ pdata = data;
    float *__restrict__ ptempr = tempr;
    float *__restrict__ ptempi = tempi;
    float ii = 0.0f;

    alignas(ALIGNMENT) float zReal0[width];

    for (int i = 0; i < height / 2; i++, ii++)
    {
        const float y = y_start + ii * dy;
        int * __restrict__ row = pdata + i * width;

        #pragma omp simd aligned(ptempr:ALIGNMENT) aligned(ptempi:ALIGNMENT) aligned(row:ALIGNMENT) simdlen(SIMD_WIDTH) aligned(zReal0:ALIGNMENT)
        for (int j = 0; j < width; j++) {
            ptempi[j] = y;
            ptempr[j] = zReal0[j] = x_start + j * dx;
            row[j] = 0;
        }

        int finished = 0;
        for (int n = 0; (n < limit) && (finished != width); n++) {
            finished = 0;
            #pragma omp simd aligned(ptempr:ALIGNMENT) aligned(ptempi:ALIGNMENT) aligned(row:ALIGNMENT) reduction(+:finished) simdlen(SIMD_WIDTH) aligned(zReal0:ALIGNMENT)
            for (int j = 0; j < width; j++) {
                float zReal = ptempr[j];
                float zImag = ptempi[j];

                float r2 = zReal * zReal;
                float i2 = zImag * zImag;

                float active = (r2 + i2 < 4.0f) ? 1.0f : 0.0f;

                float newZReal = r2 - i2 + zReal0[j];
                float newZImag = 2.0f * zReal * zImag + y;

                ptempr[j] = active * newZReal + (1.0f - active) * 10;
                ptempi[j] = active * newZImag + (1.0f - active) * 10;

                finished += int(1.0f - active);
                row[j] += int(active);
            }
        }

        memcpy(pdata + (height - 1 - i) * width, row, width * sizeof(int));
    }

    return data;
}
