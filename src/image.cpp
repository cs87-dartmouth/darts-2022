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
#define TINYEXR_USE_THREAD 1
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

template <int N>
bool load(const string &filename, bool raw, Image<Color<N, float>> &image)
{
    string errors_stb, errors_exr;

    Progress progress(fmt::format("Loading: {}", filename));

    // first try using stb_image
    if (is_stb_image(filename))
    {
        // stbi doesn't do proper srgb, but uses gamma=2.2 instead, so override it.
        // we'll do our own srgb correction
        stbi_ldr_to_hdr_scale(1.0f);
        stbi_ldr_to_hdr_gamma(1.0f);

        int    w, h, n;
        float *float_data = stbi_loadf(filename.c_str(), &w, &h, &n, N);
        bool   is_HDR     = stbi_is_hdr(filename.c_str());
        if (float_data)
        {
            image.resize(w, h);
            // copy pixels over to the Image
            for (auto y : range(h))
            {
                for (auto x : range(w))
                {
                    image(x, y) = Color<N, float>{float_data + N * (x + y * w)};
                    if (!is_HDR && !raw)
                    {
                        auto rgb       = to_linear_RGB(*reinterpret_cast<Color3f *>(&image(x, y)));
                        image(x, y)[0] = rgb[0];
                        image(x, y)[1] = rgb[1];
                        image(x, y)[2] = rgb[2];
                    }
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
            image.resize(w, h);

            // copy pixels over to the Image
            for (auto y : range(h))
                for (auto x : range(w))
                    image(x, y) = Color<N, float>{float_data + 4 * (x + y * w)};
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

template <int N>
bool save(const string &filename, float gain, const Image<Color<N, float>> &buffer)
{
    string extension = get_file_extension(filename);

    transform(extension.begin(), extension.end(), extension.begin(),
              [](char c) { return static_cast<char>(std::tolower(c)); });

    if (extension == "hdr")
        return stbi_write_hdr(filename.c_str(), buffer.width(), buffer.height(), N,
                              reinterpret_cast<const float *>(&buffer(0))) != 0;
    else if (extension == "exr")
    {
        EXRHeader header;
        InitEXRHeader(&header);

        EXRImage image;
        InitEXRImage(&image);

        image.num_channels = N;

        std::vector<float> images[N];
        for (auto i : range(N))
        {
            images[i].resize(buffer.size());
            for (auto j : range(images[i].size()))
                images[i][j] = buffer(j)[i];
        }

        float *image_ptr[N];
        // first 3 channels are in BGR order, then A
        for (auto i : range(3))
            image_ptr[i] = &(images[2 - i].at(0));
        if constexpr (N == 4)
            image_ptr[3] = &(images[3].at(0)); // A

        image.images = (uint8_t **)image_ptr;
        image.width  = buffer.width();
        image.height = buffer.height();

        header.num_channels = N;
        header.channels     = (EXRChannelInfo *)malloc(sizeof(EXRChannelInfo) * header.num_channels);
        // Must be BGR(A) order, since most of EXR viewers expect this channel order.
        const char *chan_names[4] = {"B", "G", "R", "A"};
        for (auto i : range(N))
            strncpy(header.channels[i].name, chan_names[i], 255);

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
        header.custom_attributes[0].value = reinterpret_cast<uint8_t *>(&comment);
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
        vector<uint8_t> data(buffer.size() * N, 0);
        for (auto y : range(buffer.height()))
            for (auto x : range(buffer.width()))
            {
                int pixel_offset = N * (x + y * buffer.width());

                Color3f cf{reinterpret_cast<const float *>(&buffer(x, y))};
                cf *= gain;

                // check for invalid colors and make them magenta
                cf = la::all(la::isfinite(cf)) ? cf : Color3f{1.f, 0.f, 1.f};

                Color3c cc{clamp(to_sRGB(cf), 0.f, 1.f) * 255};

                data[pixel_offset + 0] = cc[0];
                data[pixel_offset + 1] = cc[1];
                data[pixel_offset + 2] = cc[2];

                if constexpr (N == 4)
                    data[pixel_offset + 3] = clamp(buffer(x, y)[3], 0.f, 1.f) * 255;
            }

        if (extension == "png")
            return stbi_write_png(filename.c_str(), buffer.width(), buffer.height(), N, &data[0],
                                  sizeof(uint8_t) * buffer.width() * N) != 0;
        else if (extension == "jpg" || extension == "jpeg")
            return stbi_write_jpg(filename.c_str(), buffer.width(), buffer.height(), N, &data[0], 100) != 0;
        else if (extension == "bmp")
            return stbi_write_bmp(filename.c_str(), buffer.width(), buffer.height(), N, &data[0]) != 0;
        else if (extension == "tga")
            return stbi_write_tga(filename.c_str(), buffer.width(), buffer.height(), N, &data[0]) != 0;
        else
            throw DartsException("Could not determine desired file type from extension.");
    }
}

} // namespace

template <>
bool Image<Color3f>::load(const string &filename, bool raw)
{
    return ::load(filename, raw, *this);
}

template <>
bool Image<Color3f>::save(const string &filename, float gain)
{
    return ::save(filename, gain, *this);
}

template <>
bool Image<Color4f>::load(const string &filename, bool raw)
{
    return ::load(filename, raw, *this);
}

template <>
bool Image<Color4f>::save(const string &filename, float gain)
{
    return ::save(filename, gain, *this);
}
