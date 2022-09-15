/*
    This file is part of darts – the Dartmouth Academic Ray Tracing Skeleton.

    Copyright (c) 2017-2022 by Wojciech Jarosz
*/
#pragma once

#include <darts/math.h>
#include <darts/sampling.h>
#include <darts/spherical.h>
#include <darts/transform.h>
#include <darts/json.h>

/**
    A virtual (pinhole) camera.

    The camera is responsible for generating primary rays. It is positioned
    using a Transform and points along the -z axis of the local coordinate
    system. It has an image plane positioned a z = -dist with size
    (width, height).

    We currently only support pinhole perspective cameras. This class could
    be made into a virtual base class to support other types of cameras
    (e.g. an orthographic camera, or omni-directional camera).

    The camera setup looks something like this, where the
    up vector points out of the screen:

    \verbatim
            top view                         side view
               ^                    up
               |                     ^
               |                     |             _,-'
             width                   |         _,-'   |
          +----|----+     +          |     _,-'       | h
           \   |   /    d |        e | _,-'           | e
            \  |  /     i |        y +'---------------+-i----->
             \ | /      s |        e  '-,_   dist     | g
              \|/       t |               '-,_        | h
               +          +                   '-,_    | t
              eye                                 '-,_|
    \endverbatim

 */
class Camera
{
public:
    /// Construct a camera from json parameters.
    Camera(const json &j = json());

    /// Return the camera's image resolution
    Vec2i resolution() const
    {
        return m_resolution;
    }

    /**
        Generate a ray going through image-plane location \p pixel.

        \p pixel ranges from (0,0) to (m_resolution.x, m_resolution.y) along the x- and y-axis of the rendered image,
        respectively.

        \param pixel 	        The pixel position within the image.
        \return 	            The #Ray data structure filled with the appropriate position and direction.
     */
    Ray3f generate_ray(const Vec2f &pixel) const;


private:
    Transform m_xform           = Transform();     ///< Local coordinate system
    Vec2f     m_size            = Vec2f(1, 1);     ///< Physical size of the image plane
    float     m_focal_distance  = 1.f;             ///< Distance to image plane along local z axis
    Vec2i     m_resolution      = Vec2i(512, 512); ///< Image resolution
    float     m_aperture_radius = 0.f;             ///< The size of the aperture for depth of field
};

/**
    \file
    \brief Class #Camera
*/
