/*
    This file is part of darts – the Dartmouth Academic Ray Tracing Skeleton.

    Copyright (c) 2017-2022 by Wojciech Jarosz
*/

#pragma once

#include <darts/array2d.h>
#include <darts/common.h>
#include <darts/math.h>

/// A floating-point RGB image
class Image3f : public Array2d<Color3f>
{
    using Base = Array2d<Color3f>;

public:
    enum class WrapMode
    {
        Repeat, ///< Assume that the image repeats periodically
        Clamp,  ///< Clamp to the outermost pixel value
        Zero,   ///< Return zero (black) outside the defined domain
        One     ///< Return one (white) outside the defined domain
    };

    enum class FilterType
    {
        /// Nearest neighbor lookups
        Nearest = 0,
        /// Bilinear interpolation
        Bilinear = 1,
    };

    /// Default constructor (empty image)
    Image3f() : Base()
    {
    }
    /**
        Size Constructor (sets width and height)

        \param w     The width of the image
        \param h     The height of the image
     */
    Image3f(int w, int h) : Base(w, h)
    {
    }

    /**
        Construct an image of a fixed size and initialize all pixels

        \param w     The width of the image
        \param h     The height of the image
        \param v     The Color to set all pixels
     */
    Image3f(int w, int h, const Color3f &v) : Base(w, h)
    {
        reset(v);
    }

    const Color3f &texel(int x, int y, WrapMode xmode = WrapMode::Repeat, WrapMode ymode = WrapMode::Repeat) const
    {
        static const Color3f black = Color3f{0.f};
        static const Color3f white = Color3f{1.f};
        int                  s = x, t = y;
        if (s < 0 || s >= width())
        {
            switch (xmode)
            {
            case WrapMode::Repeat: s = mod(s, width()); break;
            case WrapMode::Clamp: s = std::clamp(s, 0, width() - 1); break;
            case WrapMode::Zero:
                if (s < 0 || s >= width())
                    return black;
                break;
            case WrapMode::One:
                if (s < 0 || s >= width())
                    return black;
                break;
            }
        }

        if (t < 0 || t >= height())
        {
            switch (ymode)
            {
            case WrapMode::Repeat: t = mod(t, height()); break;
            case WrapMode::Clamp: t = std::clamp(t, 0, height() - 1); break;
            case WrapMode::Zero:
                if (t < 0 || t >= height())
                    return black;
                break;
            case WrapMode::One:
                if (t < 0 || t >= height())
                    return black;
                break;
            }
        }

        return this->operator()(s, t);
    }

    Color3f bilinear(float x, float y, WrapMode xmode = WrapMode::Repeat, WrapMode ymode = WrapMode::Repeat) const
    {
        int   ix = int(std::floor(x)), iy = int(std::floor(y));
        float dx1 = x - ix, dx2 = 1.f - dx1, dy1 = y - iy, dy2 = 1.f - dy1;

        return texel(ix, iy, xmode, ymode) * dx2 * dy2 + texel(ix, iy + 1, xmode, ymode) * dx2 * dy1 +
               texel(ix + 1, iy, xmode, ymode) * dx1 * dy2 + texel(ix + 1, iy + 1, xmode, ymode) * dx1 * dy1;
    }

    /**
        Load an image from file

        \param filename	The filename
        \return 		True if the file loaded successfully
     */
    bool load(const std::string &filename);

    /**
        Save an image to the specified filename
        \param filename The filename to save to
        \param gain 	The multiplicative gain to apply to pixel values before saving
        \return 		True if the file saved successfully
     */
    bool save(const std::string &filename, float gain = 1.0f);

    /// Set of supported formats for image loading
    static std::set<std::string> loadable_formats()
    {
        return {"jpg",
                "jpeg"
                "png",
                "bmp",
                "psd",
                "tga",
                "gif",
                "hdr",
                "pic",
                "ppm",
                "pgm",
                "exr"};
    }

    /// Set of supported formats for image saving
    static std::set<std::string> savable_formats()
    {
        return {"bmp", "exr", "hdr", "jpg", "png", "tga"};
    }
};

/**
    \file
    \brief Class #Image3f
*/
