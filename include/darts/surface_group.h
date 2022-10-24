/*
    This file is part of darts – the Dartmouth Academic Ray Tracing Skeleton.

    Copyright (c) 2017-2022 by Wojciech Jarosz
*/
#pragma once

#include <darts/surface.h>

/**
    A collection of Surfaces grouped together.

    Provides an interface for treating a collection of Surfaces as a single Surface. This base class implements a naive
    linear-time intersection routine which simply intersects each ray with every child Surface.

    This class also serves as the superclass for acceleration structures (such as BVHs, KD-Trees) which are responsible
    for performing ray-surface intersection tests against a collection of Surfaces.

    We derive SurfaceGroup from XformedSurface so that nested SurfaceGroups can be individually
    positioned/oriented with respect to their parent.

    \ingroup Surfaces
*/
class SurfaceGroup : public XformedSurface
{
public:
    /// Create a new naive accelerator, add children if they are specified in \p j
    SurfaceGroup(const json &j = json::object());

    virtual void add_child(shared_ptr<Surface> surface) override;

    /**
        Intersect a ray against all surfaces registered with the Accelerator.

        \copydetails Surface::intersect()
    */
    bool intersect(const Ray3f &ray, HitInfo &hit) const override;

    Box3f local_bounds() const override;

    pair<const Surface *, float> sample_child(float &rv1) const override;
    float                        child_prob() const override;
    float                        pdf(const Vec3f &o, const Vec3f &v) const override;

protected:
    vector<shared_ptr<Surface>> m_surfaces; ///< All children
    Box3f m_bounds;
};

/**
    \file
    \brief Class #SurfaceGroup
*/
