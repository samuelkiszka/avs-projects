/**
 * @file    tree_mesh_builder.h
 *
 * @author  Samuel Kiszka <xkiszk00@stud.fit.vutbr.cz>
 *
 * @brief   Parallel Marching Cubes implementation using OpenMP tasks + octree early elimination
 *
 * @date    1 December 2025
 **/

#ifndef TREE_MESH_BUILDER_H
#define TREE_MESH_BUILDER_H

#include "base_mesh_builder.h"

class TreeMeshBuilder : public BaseMeshBuilder
{
public:
    TreeMeshBuilder(unsigned gridEdgeSize);
    using Triangle_t = BaseMeshBuilder::Triangle_t;
    float mIsoLevel = BaseMeshBuilder::mIsoLevel;

private:
    static thread_local std::vector<Triangle_t> *thread_triangles;

protected:
    unsigned marchCubes(const ParametricScalarField &field);
    void processCube(const Vec3_t<float> &cubeOffset, unsigned size, const ParametricScalarField &field);
    bool intersectsIsosurface(const Vec3_t<float> &cubeOffset, unsigned size, const ParametricScalarField &field);
    float evaluateFieldAt(const Vec3_t<float> &pos, const ParametricScalarField &field);
    void emitTriangle(const Triangle_t &triangle);
    const Triangle_t *getTrianglesArray() const { return nullptr; }

    std::vector<Triangle_t> mTriangles; ///< Temporary array of triangles
};

#endif // TREE_MESH_BUILDER_H
