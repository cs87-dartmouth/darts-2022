/*
    This file is part of darts – the Dartmouth Academic Ray Tracing Skeleton.

    Copyright (c) 2017-2022 by Wojciech Jarosz
*/

#pragma once

#include <darts/array2d.h>
#include <darts/common.h>
#include <darts/math.h>

/// An image with a generic pixel type
template <typename T>
class Image : public Array2d<T>
{
    using Base = Array2d<T>;

public:
    /// Default constructor (empty image)
    Image() : Base()
    {
    }
    /**
        Size Constructor (sets width and height)

        \param w     The width of the image
        \param h     The height of the image
     */
    Image(int w, int h) : Base(w, h)
    {
    }

    /**
        Construct an image of a fixed size and initialize all pixels

        \param w     The width of the image
        \param h     The height of the image
        \param v     The Color to set all pixels
     */
    Image(int w, int h, const T &v) : Base(w, h)
    {
        reset(v);
    }

    /**
        Load an image from file

        \param filename	The filename
        \param raw      If set to true, this will bypass the sRGB to linear conversion
        \return 		True if the file loaded successfully
    */
    bool load(const std::string &filename, bool raw = false);

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

/// A floating-point RGB image
using Image3f = Image<Color3f>;
/// A floating-point RGBA image
using Image4f = Image<Color4f>;

template <>
bool Image3f::load(const std::string &filename, bool raw);
template <>
bool Image4f::load(const std::string &filename, bool raw);

template <>
bool Image3f::save(const std::string &filename, float gain);
template <>
bool Image4f::save(const std::string &filename, float gain);

/**
    \file
    \brief Class #Image, #Image3f, and #Image4f
*/
