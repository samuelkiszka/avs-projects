/**
 * @file    tree_mesh_builder.cpp
 *
 * @author  Samuel Kiszka <xkiszk00@stud.fit.vutbr.cz>
 *
 * @brief   Parallel Marching Cubes implementation using OpenMP tasks + octree early elimination
 *
 * @date    4 December 2025
 **/

#include <iostream>
#include <math.h>
#include <limits>
#include <omp.h>

#include "tree_mesh_builder.h"

thread_local std::vector<TreeMeshBuilder::Triangle_t> *TreeMeshBuilder::thread_triangles = nullptr;

TreeMeshBuilder::TreeMeshBuilder(unsigned gridEdgeSize)
    : BaseMeshBuilder(gridEdgeSize, "Octree")
{

}

unsigned TreeMeshBuilder::marchCubes(const ParametricScalarField &field)
{
    int maxThreads = omp_get_max_threads();
    std::vector<std::vector<TreeMeshBuilder::Triangle_t>> threadTriangles(maxThreads);

    #pragma omp parallel
    {
        int threadId = omp_get_thread_num();
        thread_triangles = &threadTriangles[threadId];

        #pragma omp single nowait
        {
            TreeMeshBuilder::processCube(Vec3_t<float>(0, 0, 0), mGridSize, field);
        }
    }

    size_t totalTrianglesCount = 0;
    for (const auto &triangles : threadTriangles) {
        totalTrianglesCount += triangles.size();
    }
    mTriangles.clear();
    mTriangles.reserve(totalTrianglesCount);
    for (const auto &triangles : threadTriangles) {
        mTriangles.insert(mTriangles.end(), triangles.begin(), triangles.end());
    }

    return mTriangles.size();
}

void TreeMeshBuilder::processCube(const Vec3_t<float> &cubeOffset, unsigned size, const ParametricScalarField &field)
{
    if (!intersectsIsosurface(cubeOffset, size, field)) {
        return;
    }

    if (size == 1) {
        #pragma omp task firstprivate(cubeOffset)
        buildCube(cubeOffset, field);
        return;
    }

    const unsigned TASK_SIZE_THRESHOLD = 16;

    unsigned half = size * 0.5f;

    for (unsigned x = 0; x <= 1; ++x) {
        for (unsigned y = 0; y <= 1; ++y) {
            for (unsigned z = 0; z <= 1; ++z) {
                Vec3_t<float> newOffset = Vec3_t<float>(cubeOffset.x + x * half, cubeOffset.y + y * half, cubeOffset.z + z * half);
                if (size >= TASK_SIZE_THRESHOLD) {
                    #pragma omp task firstprivate(newOffset, half)
                    processCube(newOffset, half, field);
                } else {
                    processCube(newOffset, half, field);
                }
            }
        }
    }
    #pragma omp taskwait
}

bool TreeMeshBuilder::intersectsIsosurface(const Vec3_t<float> &cubeOffset, unsigned size, const ParametricScalarField &field)
{
    const float gridRes = mGridResolution;
    const float minX = cubeOffset.x * gridRes;
    const float minY = cubeOffset.y * gridRes;
    const float minZ = cubeOffset.z * gridRes;
    const float maxX = (cubeOffset.x + float(size)) * gridRes;
    const float maxY = (cubeOffset.y + float(size)) * gridRes;
    const float maxZ = (cubeOffset.z + float(size)) * gridRes;

    const float iso2 = BaseMeshBuilder::mIsoLevel * BaseMeshBuilder::mIsoLevel;

    const auto &points = field.getPoints();
    float value = std::numeric_limits<float>::max();

    for (unsigned i = 0; i < points.size(); ++i)
    {
        const Vec3_t<float> &p = points[i];

        float dx = 0.0f;
        if (p.x < minX)         dx = minX - p.x;
        else if (p.x > maxX)    dx = p.x - maxX;

        float dy = 0.0f;
        if (p.y < minY)         dy = minY - p.y;
        else if (p.y > maxY)    dy = p.y - maxY;

        float dz = 0.0f;
        if (p.z < minZ)         dz = minZ - p.z;
        else if (p.z > maxZ)    dz = p.z - maxZ;

        float distanceSquared = dx * dx + dy * dy + dz * dz;

        if (distanceSquared < value){
            value = distanceSquared;
            if (distanceSquared <= iso2) {
                return true;
            }
        }
    }

    return value <= iso2;
}

float TreeMeshBuilder::evaluateFieldAt(const Vec3_t<float> &pos, const ParametricScalarField &field)
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

void TreeMeshBuilder::emitTriangle(const BaseMeshBuilder::Triangle_t &triangle)
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
