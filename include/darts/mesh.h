/*
    This file is part of darts – the Dartmouth Academic Ray Tracing Skeleton.

    Copyright (c) 2017-2022 by Wojciech Jarosz
*/
#pragma once

#include <darts/surface.h>

/**
    A triangle mesh.

    This class stores a triangle mesh object and provides numerous functions
    for querying the individual triangles. Subclasses of #Mesh implement
    the specifics of how to create its contents (e.g. by loading from an
    external file)

    \ingroup Surfaces
    \ingroup Helpers
*/
struct Mesh : public Surface
{
public:
    /// Create an empty mesh
    Mesh()
    {
    }

    /// Try to load a mesh from an OBJ file
    Mesh(const json &j);

    Box3f bounds() const override
    {
        return bbox;
    }

    bool empty() const
    {
        return Fv.empty() || vs.empty();
    }

    /// Report the approximate size (in bytes) of the mesh
    size_t size() const;

    vector<Vec3f>                      vs;        ///< Vertex positions
    vector<Vec3f>                      ns;        ///< Vertex normals
    vector<Vec2f>                      uvs;       ///< Vertex texture coordinates
    vector<Vec3i>                      Fv;        ///< Vertex indices per face (triangle)
    vector<Vec3i>                      Fn;        ///< Normal indices per face (triangle)
    vector<Vec3i>                      Ft;        ///< Texture indices per face (triangle)
    vector<uint32_t>                   Fm;        ///< One material index per face (triangle)
    vector<shared_ptr<const Material>> materials; ///< All materials in the mesh
    Transform xform = Transform();                ///< Transformation that the data has already been transformed by
    Box3f     bbox;                               ///< The bounds, after transformation

    virtual void add_to_parent(Surface *parent, shared_ptr<Surface> self, const json &j) override;
};

/**
    \file
    \brief Class #Mesh
*/
