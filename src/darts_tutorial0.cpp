/*
    This file is part of darts – the Dartmouth Academic Ray Tracing Skeleton.

    Copyright (c) 2017-2022 by Wojciech Jarosz
*/

/**
    \file
    \brief A tutorial stepping through basic darts functionality
*/

#include <darts/common.h>
#include <darts/image.h>
#include <darts/ray.h>
#include <darts/spherical.h>

// function declarations; definitions are below main
void test_vectors_and_matrices();
void test_color_and_image();

int main(int argc, char **argv)
{
    darts_init();

    test_vectors_and_matrices();
    test_color_and_image();

    return 0;
}

/// Working with darts vectors and matrices
void test_vectors_and_matrices()
{
    fmt::print("\n");
    fmt::print("--------------------------------------------------------\n"
               "PROGRAMMING ASSIGNMENT 0, PART 1: Vectors and Matrices  \n"
               "--------------------------------------------------------\n");

    // The darts basecode provides a number of useful classes and functions
    // We will walk through the basic functionality of the most important
    // ones.

    // Almost all graphics programs have some data structures for storing and
    // manipulating geometric vectors and colors. Darts already provides this for
    // you in darts/math.h. The Vec<N,T> struct is a template parametrized by the
    // dimensionality N, and the type T. This just means that we can use the
    // same generic code to represent vectors containing different types
    // (floats, doubles, integers, etc) and dimensionalities (2D: 2 floats, 3D:
    // 3 floats, etc).
    //
    // In addition to storing N values of type T, the Vec class provides a
    // number of useful operations to manipulate vectors in bulk or access their
    // elements.
    //
    // We can access the elements of a vector using the [] operator
    // (e.g. myvec[0]), but for 2D, 3D, and 4D vectors you can access the
    // individual elements directly as data *member variables* by name (e.g.
    // myvec.x). Note that this is different than in the Shirley book (Shirley
    // defines member *functions* e.g. x(), to access individual elements in his
    // vec3 class.
    //
    // Most arithmetic operations are overloaded so you can perform element-wise
    // addition, multiplication, etc. You can compute the length, dot product,
    // etc. Take a look at darts/math.h or the Doxygen documentation.
    //
    // Most of the time we will be dealing with the 3D float
    // version of this structure, and to avoid typing out the full template
    // typename "Vec<3,float>" we define a bunch of type aliases in arts/math.h,
    // so we can use the shorthand "Vec3f" for a 3D vector of floats, etc. There
    // are similar ones for 2D and 4D vectors. We will also sometimes use integer
    // versions of these (e.g. Vec2i)
    //
    // We will use the Vec<T,N> type to represent geometric points,
    // direction vectors, and normals.
    //

    // Let's create some 3D vectors
    // We can initialize them using:
    // 1) the 3-element constructor,
    // 2) from a single scalar, or
    // 3) using the array initializer
    Vec3f v1(-0.1f, 0.20f, -0.300f);
    Vec3f v2(1.f);                  // This is the same as Vec3f(1,1,1);
    Vec3f v3{0.5f, 0.25f, -0.123f}; // Same as Vec3f(0.5f, 0.25f, -0.123f)

    // we can also print out vectors to the console:
    // We can do this with fmt::print (a modern alternative to C's printf and C++ iostreams):
    fmt::print("v1 = {}\n", v1);

    // But darts also provides a number of output functions:
    //  spdlog::debug
    //  spdlog::info
    //  spdlog::warn
    //  spdlog::err
    //  spdlog::critical
    // These operate just like fmt::print, but colorize the output based on the type of message, and we can also
    // globally disable messages below a certain threshold to control the verbosity of the output.

    spdlog::info("v2 = {}.", v2);
    spdlog::info("v3 = {}.\n", v3);

    spdlog::info("You can access specific components using x, y, and z.");
    // TODO: Output the z coordinate of the normal
    spdlog::info("The z coordinate of v3 is {}.\n", "TODO");

    spdlog::info("We can also element-wise add, subtract, and multiply vectors:");
    spdlog::info("v1 + v2:\n   {}\n + {}\n = {}", v1, v2, v1 + v2);
    // TODO: divide vector 1 by vector 3
    spdlog::info("v1 / v3:\n   {}\n / {}\n = {}\n", v1, v3, "TODO");

    spdlog::info("or perform mixed vector-scalar arithmetic");
    spdlog::info("scalar * v2:\n   {}\n * {}\n = {}", 2.0f, v2, 2.0f * v2);

    spdlog::info("We can compute the length of a vector, or normalize it, or take "
                 "the dot product or cross product of two vectors:");

    spdlog::info("The length of v2 is: {}", length(v2));
    spdlog::info("The squared length of v2 is: {}", length2(v2));
    Vec3f normalized2 = normalize(v2);
    spdlog::info("A normalized copy of v2 is: {}", normalized2);
    spdlog::info("Let's confirm that its length is 1: {}\n", length(normalized2));

    // TODO: look in darts/math.h to find an appropriate function to call to compute
    // the dot product and cross product between two vectors.
    spdlog::info("The dot product of v1 and v3 is: {}", "TODO");
    spdlog::info("The cross product of v1 and v2 is: {}", "TODO");

    // TODO: compute the angle between v1 and v3 (in degrees) using
    // either the dot or cross product. Use the rad2deg function from common.h.
    float degrees = 0.0f;
    spdlog::info("The angle between v1 and v3 is: {}", degrees);
    if (std::abs(degrees - 80.0787f) < 1e-4f)
        spdlog::info("Result correct!\n");
    else
        spdlog::error("Result incorrect!\n");

    // We will also make use of rays, which represent an origin and a direction:
    Ray3f ray{{0.5f, 2.0f, -3.0f}, {-0.25f, -0.5f, 0.3f}};

    // Let's print some info about our ray
    spdlog::info("The origin of ray is    {}.", ray.o);
    spdlog::info("The direction of ray is {}.", ray.d);

    // We also provide a 4x4 matrix class Mat44<T>, again templated by type T.
    // The Mat44 class includes a number of constructors. One way to fill a
    // matrix is to pass in its four *column* vectors.
    // Note that because we pass in columns, visually the matrix below looks
    // transposed: the vector {4, 5, 6, 1} is the 4th column, not row.
    Mat44f matrix{{1, 0, 0, 0}, {0, 2, 0, 0}, {0, 0, 3, 0}, {4, 5, 6, 1}};

    // We also provide the ability to compute matrix products and inverses.
    spdlog::info("The matrix is\n{}.", matrix);
    spdlog::info("The inverse is\n{}.", inverse(matrix));
    spdlog::info("mat*inv should be the identity\n{}.", mul(matrix, inverse(matrix)));
}

/// Working with RGB colors (Color3f) and images (Image3) in darts
void test_color_and_image()
{
    fmt::print("\n");
    fmt::print("--------------------------------------------------------\n"
               "PROGRAMMING ASSIGNMENT 0, PART 2: Color & image tutorial\n"
               "--------------------------------------------------------\n");

    //
    // We will use the same Vec class to represent 3D locations, directions,
    // offsets, but also colors (since the space of colors humans can see are
    // well represented with 3 numbers). We introduce the convenience typedef
    // Color3f to represent RGB colors.
    //

    // A Color3f stores three floating-point values, one for red, green and
    // blue.
    Color3f red(1, 0, 0);
    Color3f blue(0, 0, 1);
    Color3f white = Color3f(1); // This is the same as Color3f(1,1,1);

    // We can perform basic element-wise arithmatic on Colors:
    Color3f magenta   = red + blue;
    Color3f still_red = red * white;

    // TODO: Initialize the color pinkish to the average of white and red
    Color3f pinkish;

    spdlog::info("white    = {}.", white);
    spdlog::info("red      = {}.", red);
    spdlog::info("blue     = {}.", blue);
    spdlog::info("magenta  = {}.", magenta);
    spdlog::info("pinkish  = {}.", pinkish);
    spdlog::info("still_red = {}.", still_red);

    // We can also access the individual elements of the color by channel index:
    spdlog::info("Red channel of pinkish is: {}", pinkish[0]);

    // sincle Color3f is just a typedef for Vec3f, you can also access the channels using pinkish.x, pinkish.y,
    // pinkish.z, but this may not be very informative

    // TODO: Print out the green channel of pinkish
    spdlog::info("Green channel of pinkish is: {}", 0.0f);

    spdlog::info("Blue channel of still_red is: {}", still_red[2]);

    pinkish[0] *= 2.f;

    spdlog::info("After scaling by 2, red channel of pinkish is: {}", pinkish[0]);

    // The Color3f class provides a few additional operations which are useful
    // specifically for manipulating colors, see the bottom of the darts/math.h file.

    // TODO: Compute and print the luminance of pinkish. Look at darts/math.h to see
    // what method you might need
    spdlog::info("The luminance of pinkish is: {}", 0.0f);

    // Darts also provides the Image3f class (see image.h|cpp) to load, store,
    // manipulate, and write images.

    // Image3f is just a dynamically allocated 2D array of pixels. It
    // derives from the Array2D class, which is a generic 2D array
    // container of arbitrary size.

    // Here we construct an empty image that is 200 pixels across, and
    // 100 pixels tall:
    auto image1 = Image3f(200, 100);

    // In the case of Image3f, each array element (pixel) is a Color3f, which,
    // as we saw before, is itself a 3-element array.

    // We can access individual pixels of an Image3f using the (x,y) operator:
    image1(5, 10) = white; // This sets the pixel to white

    // The file common.h defines a simple linear interpolation function: lerp
    // which allows us to specify two values, a and b, and an interpolation
    // parameter t. This function is a template, which means it will work with
    // any type as long as (in this case) we can add them and multiply by a
    // scalar. Just as we could interpolate between two scalar values, we can
    // also use it to interpolate between two colors:

    spdlog::info("25% of the way from blue to red is: {}.", lerp(blue, red, 0.25f));

    // Now, let's populate the colors of an entire image, and write it to a PNG
    // file.

    Image3f gradient(200, 100);

    // TODO: Populate and output the gradient image
    // First, loop over all rows, and then columns of an image.
    // Set the red component of a pixel's color to vary linearly from 0 at the
    // leftmost pixel to 1 at the rightmost pixel; and the green component to
    // vary from 0 at the topmost pixel to 1 at the bottommost pixel. The blue
    // component should be 0 for all pixels.

    // After populating the pixel colors, look at the member functions of
    // Image3f, and call a function to save the gradient image out to the file
    // "gradient.png".

    spdlog::info("Creating gradient image.");

    put_your_code_here("Populate an image with a color gradient and save to \"scenes/assignment0/gradient.png\"");
    spdlog::info("Saving image \"gradient.png\" ...");

    // Now, we will load an image, modify it, and save it back to disk.
    Image3f image;

    // TODO: Load the image scenes/assignment0/cornellbox.png into the
    // ``image'' variable
    spdlog::info("Loading image cornellbox.png ...");
    put_your_code_here("Load the image \"scenes/assignment0/cornellbox.png\".");
    // Hint: Take a look at Image3f::load
    // Keep in mind filenames are interpreted relative to your current
    // working directory

    // TODO: Convert the image to grayscale. Loop over every pixel and convert
    // it to grayscale by replacing every pixel with its luminance
    spdlog::info("Converting image to grayscale....");
    put_your_code_here("Convert the image to grayscale.");

    // TODO: Save the image to scenes/assignment0/cornell_grayscale.png
    // Hint: Take a look at Image3f::save
    spdlog::info("Saving image cornell_grayscale.png....");
    put_your_code_here("Save the image to \"scenes/assignment0/cornell_grayscale.png\".");

    spdlog::info("Done!\n");
}
