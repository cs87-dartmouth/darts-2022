/*
    This file is part of darts – the Dartmouth Academic Ray Tracing Skeleton.

    Copyright (c) 2017-2022 by Wojciech Jarosz
*/
#pragma once

#include <darts/factory.h>
#include <darts/fwd.h>
#include <darts/material.h>
#include <darts/stats.h>
#include <darts/transform.h>

/** \addtogroup Surfaces
    @{
*/

/**
    Contains information about a ray intersection hit point.

    Used by surface intersection routines to return more than just a single value. Includes the position, traveled ray
    distance, uv coordinates, the geometric and interpolated shading normals, and a pointer to the intersected surface
    and underlying material.
*/
struct HitInfo
{
    float t;  ///< Ray parameter for the hit
    Vec3f p;  ///< World-space hit position
    Vec3f gn; ///< Geometric normal
    Vec3f sn; ///< Interpolated shading normal
    Vec2f uv; ///< UV texture coordinates

    const Material *mat = nullptr; ///< Material at the hit point

    /// Default constructor that leaves all members uninitialized
    HitInfo() = default;
};


/**
    This is the abstract superclass for all surfaces.

    Surfaces represent the geometry of the scene. A Surface could be an individual primitive like a #Sphere, or it could
    be composed of many smaller primitives, e.g. the triangles composing a #Mesh.
*/
class Surface
{
public:
    virtual ~Surface() = default;

    /**
        Perform any necessary precomputation before ray tracing.

        If a surface requires some precomputation (e.g. building an acceleration structure) overload this function. This
        will be called after the last child has been added and before any call to intersect().

        The base class implementation just does nothing.
    */
    virtual void build(){};

    /**
        Add a child surface.

        This function will become useful once we create groups of objects. The base class implementation just throws an
        error.

        This function should only be used before #build() is called.
    */
    virtual void add_child(shared_ptr<Surface> surface)
    {
        throw DartsException("This surface does not support children.");
    }

    /**
        Add this surface to the \p parent surface.

        By default this function just calls #add_child() on the parent.

        This function is used by aggregate surfaces that shouldn't themselves be added to the scene (like a mesh), but
        instead need to create other surfaces (individual triangles) that get added to the scene.

        \param [in,out] parent  The parent Surface (typically the scene) to add to.
        \param [in]     self    A shared_ptr version of this
        \param [in]     j       The #json parameters used to create \p self
     */
    virtual void add_to_parent(Surface *parent, shared_ptr<Surface> self, const json &j)
    {
        parent->add_child(self);
    }

    /**
        Ray-Surface intersection test.

        Intersect a ray against this surface and return detailed intersection information.

        \param [in] ray     A 3-dimensional ray data structure with minimum/maximum extent information
        \param [out] hit    A detailed intersection record, which will be filled by the intersection query
        \return             True if an intersection was found
     */
    virtual bool intersect(const Ray3f &ray, HitInfo &hit) const
    {
        throw DartsException("Surface intersection method not implemented.");
    }
    /// Return the surface's world-space AABB.
    virtual Box3f bounds() const = 0;


};

/**
    A convenience subclass of #Surface for surfaces with a #Transform.

    Explicitly stores a #Transform which positions/orients the surface in the scene.
*/
class XformedSurface : public Surface
{
public:
    XformedSurface(const Transform &xform = Transform()) : m_xform(xform)
    {
    }

    XformedSurface(const json &j = json::object());
    virtual ~XformedSurface() = default;

    /// The world-space bounds: obtained by simply applying #m_xform to the result of #local_bounds()
    virtual Box3f bounds() const override;

    /// Return the surface's local-space AABB.
    virtual Box3f local_bounds() const = 0;

protected:
    Transform m_xform = Transform(); ///< Local-to-world Transformation
};

/**
    Adds a #Material to #XformedSurface.

    Explicitly stores a #Transform which positions/orients the surface in the scene and pointer to a single #Material
    which specifies the light reflectance properties.
*/
class XformedSurfaceWithMaterial : public XformedSurface
{
public:
    XformedSurfaceWithMaterial(shared_ptr<const Material> material, const Transform &xform = Transform()) :
        XformedSurface(xform), m_material(material)
    {
    }

    XformedSurfaceWithMaterial(const json &j = json::object());
    virtual ~XformedSurfaceWithMaterial() = default;


protected:
    shared_ptr<const Material> m_material;
};

STAT_RATIO("Intersections/Total intersection tests per ray", g_num_total_intersection_tests, g_num_traced_rays);

/** @}*/

/**
    \file
    \brief Class #HitInfo, #Surface, and #XformedSurfaceWithMaterial.
*/
