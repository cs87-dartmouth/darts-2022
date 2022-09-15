/*
    This file is part of darts – the Dartmouth Academic Ray Tracing Skeleton.

    Copyright (c) 2017-2022 by Wojciech Jarosz
*/

#include <darts/scene.h>
#include <darts/surface.h>

XformedSurface::XformedSurface(const json &j)
{
    m_xform = j.value("transform", m_xform);
}



XformedSurfaceWithMaterial::XformedSurfaceWithMaterial(const json &j) : XformedSurface(j)
{
    m_material = DartsFactory<Material>::find(j);
}


