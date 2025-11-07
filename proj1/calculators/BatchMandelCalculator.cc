/**
 * @file BatchMandelCalculator.cc
 * @author FULL NAME <xkiszk00@stud.fit.vutbr.cz>
 * @brief Implementation of Mandelbrot calculator that uses SIMD paralelization over small batches
 * @date DATE
 */

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <string.h>

#include <stdlib.h>
#include <stdexcept>

#include "BatchMandelCalculator.h"

#define ALIGNMENT 512
#define SIMD_WIDTH 64
#define BATCH_SIZE 64
#define TOTAL_BATCH_SIZE (BATCH_SIZE * BATCH_SIZE)

BatchMandelCalculator::BatchMandelCalculator (unsigned matrixBaseSize, unsigned limit) :
    BaseMandelCalculator(matrixBaseSize, limit, "BatchMandelCalculator")
{
    data = (int *)(aligned_alloc(ALIGNMENT, height * width * sizeof(int)));
    tempr = (float *)(aligned_alloc(ALIGNMENT, BATCH_SIZE * BATCH_SIZE * sizeof(float)));
    tempi = (float *)(aligned_alloc(ALIGNMENT, BATCH_SIZE * BATCH_SIZE * sizeof(float)));
}

BatchMandelCalculator::~BatchMandelCalculator() {
    free(data);
    free(tempr);
    free(tempi);
    data = NULL;
    tempr = NULL;
    tempi = NULL;
}


int * BatchMandelCalculator::calculateMandelbrot () {
    alignas(ALIGNMENT) float zReal0[TOTAL_BATCH_SIZE];
    int *__restrict__ pdata = data;
    float *__restrict__ ptempr = tempr;
    float *__restrict__ ptempi = tempi;
    float *__restrict__ pzReal0 = zReal0;

    for (int i = 0; i < height / 2; i += BATCH_SIZE)
    {
        for (int j = 0; j < width; j += BATCH_SIZE)
        {
            // Initialize batch
            for (int bi = 0; bi < BATCH_SIZE; bi++) {
                float y = y_start + (i + bi) * dy;
                int index = bi * BATCH_SIZE;
                #pragma omp simd aligned(ptempr:ALIGNMENT) aligned(ptempi:ALIGNMENT) aligned(pzReal0:ALIGNMENT)  simdlen(SIMD_WIDTH)
                for (int bj = 0; bj < BATCH_SIZE; bj++) {
                    ptempr[index + bj] = pzReal0[index + bj] = x_start + (j + bj) * dx;
                    ptempi[index + bj] = y;
                }
            }

            // Compute batch into temporary storage
            int *__restrict__ current = pdata + (i * width) + j;
            int finished = 0;
            for (int n = 0; (n < limit) && (finished != TOTAL_BATCH_SIZE); n++) {
                finished = 0;
                for (int bi = 0; bi < BATCH_SIZE; bi++) {
                    int index = bi * BATCH_SIZE;
                    int *__restrict__ current_row = current + bi * width;
                    float y = y_start + (i + bi) * dy;
                    #pragma omp simd aligned(ptempr:ALIGNMENT) aligned(ptempi:ALIGNMENT) aligned(current_row:ALIGNMENT) aligned(pzReal0:ALIGNMENT) reduction(+:finished) linear(index:1) simdlen(SIMD_WIDTH)
                    for (int bj = 0; bj < BATCH_SIZE; bj++) {
                        float zReal = ptempr[index];
                        float zImag = ptempi[index];

                        float r2 = zReal * zReal;
                        float i2 = zImag * zImag;

                        float active = (r2 + i2 < 4.0f) ? 1.0f : 0.0f;
                        finished = active ? finished : finished + 1;

                        current_row[bj] = active ? current_row[bj] + 1 : current_row[bj];
                        ptempr[index] = active ? (r2 - i2 + pzReal0[index]) : 10.0f;
                        ptempi[index] = active ? (2.0f * zReal * zImag + y) : 10.0f;

                        index++;
                    }
                }
            }
        }
    }

    for (int i = 0; i < height / 2; i++) {
        memcpy(pdata + (height - 1 - i) * width, pdata + i * width, width * sizeof(int));
    }

    return data;
}
