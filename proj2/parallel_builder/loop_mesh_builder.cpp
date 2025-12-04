/**
 * @file    loop_mesh_builder.cpp
 *
 * @author  Samuel Kiszka <xkiszk00@stud.fit.vutbr.cz>
 *
 * @brief   Parallel Marching Cubes implementation using OpenMP loops
 *
 * @date    1 December 2025
 **/

#include <iostream>
#include <math.h>
#include <limits>
#include <omp.h>

#include "loop_mesh_builder.h"

thread_local std::vector<LoopMeshBuilder::Triangle_t> *LoopMeshBuilder::thread_triangles = nullptr;

LoopMeshBuilder::LoopMeshBuilder(unsigned gridEdgeSize)
        : BaseMeshBuilder(gridEdgeSize, "OpenMP Loop")
{

}

unsigned LoopMeshBuilder::marchCubes(const ParametricScalarField &field)
{
    size_t totalCubesCount = mGridSize*mGridSize*mGridSize;

    int maxThreads = omp_get_max_threads();

    // Create a vector of triangle vectors, one for each thread
    std::vector<std::vector<LoopMeshBuilder::Triangle_t>> threadTriangles(maxThreads);

    // Parallel region - every thread gets its own vector to store triangles
    #pragma omp parallel
    {
        int threadId = omp_get_thread_num();
        // Assign the thread's triangle vector to the thread-local pointer
        thread_triangles = &threadTriangles[threadId];

        const size_t chunkSize = 8;
        // Parallel for loop to process all cubes
        #pragma omp for schedule(guided, chunkSize)
        for (size_t i = 0; i < totalCubesCount; ++i) {
            Vec3_t<float> cubeOffset(i % mGridSize,
                                     (i / mGridSize) % mGridSize,
                                     i / (mGridSize * mGridSize));

            // Evaluate "Marching Cube" at given position in the grid
            buildCube(cubeOffset, field);
        }
        thread_triangles = nullptr;
    } // End of parallel region

    // Combine triangles from all threads into the main triangle vector
    size_t totalTrianglesCount = 0;
    for (const auto &triangles : threadTriangles) {
        totalTrianglesCount += triangles.size();
    }
    mTriangles.clear();
    mTriangles.reserve(totalTrianglesCount);
    for (const auto &triangles : threadTriangles) {
        mTriangles.insert(mTriangles.end(), triangles.begin(), triangles.end());
    }

    return totalTrianglesCount;
}

float LoopMeshBuilder::evaluateFieldAt(const Vec3_t<float> &pos, const ParametricScalarField &field)
{
    const Vec3_t<float> *pPoints = field.getPoints().data();
    const unsigned count = unsigned(field.getPoints().size());

    float value = std::numeric_limits<float>::max();

    for(unsigned i = 0; i < count; ++i)
    {
        float distanceSquared  = (pos.x - pPoints[i].x) * (pos.x - pPoints[i].x);
        distanceSquared       += (pos.y - pPoints[i].y) * (pos.y - pPoints[i].y);
        distanceSquared       += (pos.z - pPoints[i].z) * (pos.z - pPoints[i].z);

        value = std::min(value, distanceSquared);
    }

    return sqrt(value);
}

void LoopMeshBuilder::emitTriangle(const LoopMeshBuilder::Triangle_t &triangle)
{
    if (thread_triangles) {
        // Use thread-local storage to avoid contention
        thread_triangles->push_back(triangle);
        return;
    }

    // Fallback to global vector with critical section (should not happen in normal execution)
    #pragma omp critical
    {
        mTriangles.push_back(triangle);
    }
}
