/**
 * @file BatchMandelCalculator.cc
 * @author FULL NAME <xlogin00@stud.fit.vutbr.cz>
 * @brief Implementation of Mandelbrot calculator that uses SIMD paralelization over small batches
 * @date DATE
 */

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

#include <stdlib.h>
#include <stdexcept>

#include "BatchMandelCalculator.h"

#define ALIGNMENT 32
#define BATCH_SIZE 32
#define IMAG_OFFSET (BATCH_SIZE * BATCH_SIZE)

BatchMandelCalculator::BatchMandelCalculator (unsigned matrixBaseSize, unsigned limit) :
    BaseMandelCalculator(matrixBaseSize, limit, "BatchMandelCalculator")
{
    data = (int *)(aligned_alloc(ALIGNMENT, height * width * sizeof(int)));
    temp = (float *)(aligned_alloc(ALIGNMENT, BATCH_SIZE * BATCH_SIZE * 2 * sizeof(float)));
}

BatchMandelCalculator::~BatchMandelCalculator() {
    free(data);
    free(temp);
    data = NULL;
    temp = NULL;
}


int * BatchMandelCalculator::calculateMandelbrot () {
    int *pdata = data;
    float *ptemp = temp;
    int index, batch_index;
    int vals[BATCH_SIZE * BATCH_SIZE];
    float zReal, zImag, r2, i2, y;

    for (int i = 0; i < height / (BATCH_SIZE * 2); i++)
    {
        for (int j = 0; j < width / BATCH_SIZE; j++)
        {
            // Initialize batch
            for (int bi = 0; bi < BATCH_SIZE; bi++) {
                y = y_start + (i * BATCH_SIZE + bi) * dy;
                #pragma omp simd aligned(ptemp:ALIGNMENT) simdlen(32)
                for (int bj = 0; bj < BATCH_SIZE; bj++) {
                    ptemp[bi * BATCH_SIZE + bj] = x_start + (j * BATCH_SIZE + bj) * dx;
                    ptemp[IMAG_OFFSET + bi * BATCH_SIZE + bj] = y;
                    vals[bi * BATCH_SIZE + bj] = 0;
                }
            }

            // Compute batch into temporary storage
            for (int n = 0; n < limit; n++) {
                for (int bi = 0; bi < BATCH_SIZE; bi++) {
                    index = bi * BATCH_SIZE;
                    #pragma omp simd aligned(ptemp:ALIGNMENT) linear(index:1)
                    for (int bj = 0; bj < BATCH_SIZE; bj++) {
                        zReal = ptemp[index];
                        zImag = ptemp[IMAG_OFFSET + index];

                        r2 = zReal * zReal;
                        i2 = zImag * zImag;
                        if (r2 + i2 < 4.0f){
                            ptemp[index] = r2 - i2 + x_start + (j * BATCH_SIZE + bj) * dx;
                            ptemp[IMAG_OFFSET + index] = 2.0f * zReal * zImag + y_start + (i * BATCH_SIZE + bi) * dy;
                            vals[index]++;
                        }
                        index++;
                    }
                }
            }

            // Store results
            int *current = pdata + (i * BATCH_SIZE * width) + (j * BATCH_SIZE);
            for (int bi = 0; bi < BATCH_SIZE; bi++) {
                int *current_row = current + bi * width;
                #pragma omp simd aligned(pdata:ALIGNMENT) simdlen(32)
                for (int bj = 0; bj < BATCH_SIZE; bj++) {
                    *(current_row + bj) = vals[bi * BATCH_SIZE + bj];
                }
            }
        }
    }

    // Copy the second half of the set as it is symmetric
    int i = height / 2;
    pdata += (width * height) / 2;
    while(i--) {
        #pragma omp simd aligned(data:ALIGNMENT) aligned(pdata:ALIGNMENT) linear(i: -1) simdlen(32)
        for (int j = 0; j < width; j++)
            *(pdata++) = *(data + i * width + j);
    }

    return data;
}
