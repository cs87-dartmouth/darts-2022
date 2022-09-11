/*
    This file is part of darts – the Dartmouth Academic Ray Tracing Skeleton.

    Copyright (c) 2017-2022 by Wojciech Jarosz
*/

#include <cctype>
#include <darts/common.h>
#include <darts/image.h>
#include <darts/progress.h>
#include <iostream>
#include <math.h>
#include <sstream>
#include <thread>

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_STATIC

// these pragmas ignore warnings about unused static functions
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-function"
#elif defined(__GNUC__) || defined(__GNUG__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#elif defined(_MSC_VER)
#pragma warning(push, 0)
#endif

#include "stb_image.h"

#if defined(__clang__)
#pragma clang diagnostic pop
#elif defined(__GNUC__) || defined(__GNUG__)
#pragma GCC diagnostic pop
#elif defined(_MSC_VER)
#pragma warning(pop)
#endif

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define TINYEXR_IMPLEMENTATION
#include "tinyexr.h"

// local functions
namespace
{

string get_file_extension(const string &filename)
{
    if (filename.find_last_of(".") != string::npos)
        return filename.substr(filename.find_last_of(".") + 1);
    return "";
}

bool is_stb_image(const string &filename)
{
    FILE *f = stbi__fopen(filename.c_str(), "rb");
    if (!f)
        return false;

    stbi__context s;
    stbi__start_file(&s, f);

    // try stb library first
    if (stbi__jpeg_test(&s) || stbi__png_test(&s) || stbi__bmp_test(&s) || stbi__gif_test(&s) || stbi__psd_test(&s) ||
        stbi__pic_test(&s) || stbi__pnm_test(&s) || stbi__hdr_test(&s) || stbi__tga_test(&s))
    {
        fclose(f);
        return true;
    }

    fclose(f);
    return false;
}

} // namespace

bool Image3f::load(const string &filename)
{
    string errors_stb, errors_exr;

    Progress progress(fmt::format("Loading: {}", filename));

    // first try using stb_image
    // if (is_stb_image(filename))
    {
        // stbi doesn't do proper srgb, but uses gamma=2.2 instead, so override it.
        // we'll do our own srgb correction
        stbi_ldr_to_hdr_scale(1.0f);
        stbi_ldr_to_hdr_gamma(1.0f);

        int    n, w, h;
        float *float_data = stbi_loadf(filename.c_str(), &w, &h, &n, 3);
        bool   is_HDR     = stbi_is_hdr(filename.c_str());
        if (float_data)
        {
            resize(w, h);
            // copy pixels over to the Image
            for (auto y : range(h))
            {
                for (auto x : range(w))
                {
                    auto c = Color3f(float_data[3 * (x + y * w) + 0], float_data[3 * (x + y * w) + 1],
                                     float_data[3 * (x + y * w) + 2]);
                    if (!is_HDR)
                        c = to_linear_RGB(c);
                    (*this)(x, y) = c;
                }
            }
            stbi_image_free(float_data);
            return true;
        }

        errors_stb = stbi_failure_reason();
    }

    // next try exrs
    {
        float      *float_data; // width * height * RGBA
        int         w;
        int         h;
        const char *err = nullptr;

        int ret = LoadEXR(&float_data, &w, &h, filename.c_str(), &err);

        if (ret == TINYEXR_SUCCESS)
        {
            resize(w, h);

            // copy pixels over to the Image
            for (auto y : range(h))
            {
                for (auto x : range(w))
                    (*this)(x, y) = Color3f(float_data[4 * (x + y * w) + 0], float_data[4 * (x + y * w) + 1],
                                            float_data[4 * (x + y * w) + 2]);
            }
            free(float_data); // release memory of image data
            return true;
        }

        errors_exr = err;
        FreeEXRErrorMessage(err); // release memory of error message.
    }

    progress.set_done();

    spdlog::error("Unable to read image file \"{}\": {}; {}.", filename, errors_stb, errors_exr);

    return false;
}

bool Image3f::save(const string &filename, float gain)
{
    string extension = get_file_extension(filename);

    transform(extension.begin(), extension.end(), extension.begin(),
              [](char c) { return static_cast<char>(std::tolower(c)); });

    if (extension == "hdr")
        return stbi_write_hdr(filename.c_str(), width(), height(), 3, (const float *)&m_data[0]) != 0;
    else if (extension == "exr")
    {
        EXRHeader header;
        InitEXRHeader(&header);

        EXRImage image;
        InitEXRImage(&image);

        image.num_channels = 3;

        std::vector<float> images[3];
        images[0].resize(size());
        images[1].resize(size());
        images[2].resize(size());

        for (auto i : range(size()))
        {
            images[0][i] = (*this)(i)[0];
            images[1][i] = (*this)(i)[1];
            images[2][i] = (*this)(i)[2];
        }

        float *image_ptr[3];
        image_ptr[0] = &(images[2].at(0)); // B
        image_ptr[1] = &(images[1].at(0)); // G
        image_ptr[2] = &(images[0].at(0)); // R

        image.images = (unsigned char **)image_ptr;
        image.width  = width();
        image.height = height();

        header.num_channels = 3;
        header.channels     = (EXRChannelInfo *)malloc(sizeof(EXRChannelInfo) * header.num_channels);
        // Must be BGR(A) order, since most of EXR viewers expect this channel order.
        strncpy(header.channels[0].name, "B", 255);
        strncpy(header.channels[1].name, "G", 255);
        strncpy(header.channels[2].name, "R", 255);

        header.compression_type = TINYEXR_COMPRESSIONTYPE_PIZ;

        header.pixel_types           = (int *)malloc(sizeof(int) * header.num_channels);
        header.requested_pixel_types = (int *)malloc(sizeof(int) * header.num_channels);
        for (int i = 0; i < header.num_channels; i++)
        {
            header.pixel_types[i]           = TINYEXR_PIXELTYPE_FLOAT; // pixel type of input image
            header.requested_pixel_types[i] = TINYEXR_PIXELTYPE_HALF; // pixel type of output image to be stored in .EXR
        }

        // add custom comment attribute
        header.num_custom_attributes = 1;
        header.custom_attributes     = static_cast<EXRAttribute *>(malloc(sizeof(EXRAttribute)));

        strncpy(header.custom_attributes[0].name, "comments", 255);
        strncpy(header.custom_attributes[0].type, "string", 255);
        char comment[]                    = "Generated with darts";
        header.custom_attributes[0].value = reinterpret_cast<unsigned char *>(&comment);
        header.custom_attributes[0].size  = strlen(comment);

        const char *err;
        int         ret = SaveEXRImageToFile(&image, &header, filename.c_str(), &err);
        if (ret != TINYEXR_SUCCESS)
        {
            spdlog::error("Error saving EXR image: {}", err);
            return false;
        }

        free(header.channels);
        free(header.pixel_types);
        free(header.requested_pixel_types);
        free(header.custom_attributes);
        return true;
    }
    else
    {
        // convert floating-point image to 8-bit per channel
        vector<unsigned char> data(width() * height() * 3, 0);
        for (auto y : range(height()))
            for (auto x : range(width()))
            {
                Color3f cf = (*this)(x, y) * gain;

                // check for invalid colors and make them magenta
                cf = la::all(la::isfinite(cf)) ? cf : Color3f{1.f, 0.f, 1.f};

                Color3c cc{clamp(to_sRGB(cf), 0.f, 1.f) * 255};

                data[3 * x + 3 * y * width() + 0] = cc[0];
                data[3 * x + 3 * y * width() + 1] = cc[1];
                data[3 * x + 3 * y * width() + 2] = cc[2];
            }

        if (extension == "png")
            return stbi_write_png(filename.c_str(), width(), height(), 3, &data[0],
                                  sizeof(unsigned char) * width() * 3) != 0;
        else if (extension == "jpg" || extension == "jpeg")
            return stbi_write_jpg(filename.c_str(), width(), height(), 3, &data[0], 100) != 0;
        else if (extension == "bmp")
            return stbi_write_bmp(filename.c_str(), width(), height(), 3, &data[0]) != 0;
        else if (extension == "tga")
            return stbi_write_tga(filename.c_str(), width(), height(), 3, &data[0]) != 0;
        else
            throw DartsException("Could not determine desired file type from extension.");
    }
}
