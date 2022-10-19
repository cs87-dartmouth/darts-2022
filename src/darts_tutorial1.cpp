/*
    This file is part of darts – the Dartmouth Academic Ray Tracing Skeleton.

    Copyright (c) 2017-2022 by Wojciech Jarosz
*/

/**
    \file
    \brief A tutorial walking through writing your first path tracer in Darts
*/

#include <darts/camera.h>
#include <darts/common.h>
#include <darts/image.h>
#include <darts/progress.h>
#include <darts/sphere.h>
#include <darts/surface_group.h>
#include <darts/transform.h>

Color3f vec2color(const Vec3f &dir);
Color3f ray2color(const Ray3f &r);
Color3f intersection2color(const Ray3f &r, const Sphere &sphere);
Color3f recursive_color(const Ray3f &ray, const SurfaceGroup &scene, int depth);
void    function_with_JSON_parameters(const json &j);
void    test_manual_camera_image();
void    test_JSON();
void    test_camera_class_image();
void    test_transforms();
void    test_xformed_camera_image();
void    test_ray_sphere_intersection();
void    test_sphere_image();
void    test_materials();
void    test_recursive_raytracing();

int main(int argc, char **argv)
{
    darts_init();

    // test_manual_camera_image();
    // test_JSON();
    // test_camera_class_image();

    // test_transforms();
    // test_xformed_camera_image();

    // test_ray_sphere_intersection();
    // test_sphere_image();

    // test_materials();
    // test_recursive_raytracing();

    return 0;
}

/// Generate rays by hand
void test_manual_camera_image()
{
    fmt::print("\n");
    fmt::print("--------------------------------------------------------\n"
               "PROGRAMMING ASSIGNMENT, PART 1: Generating rays by hand \n"
               "--------------------------------------------------------\n");

    // Setup the output image
    Image3f ray_image(200, 100);

    const Vec3f camera_origin(0.f, 0.f, 0.f);
    const float image_plane_width  = 4.f;
    const float image_plane_height = 2.f;

    // loop over all pixels and generate a ray
    for (auto y : range(ray_image.height()))
    {
        for (auto x : range(ray_image.width()))
        {
            // TODO: Fill in ray_origin so that the ray starts at
            // camera_origin, and fill in ray_direction so that
            // 1) the x component of the direction varies from -image_plane_width/2 for the left-most pixel to
            //    +image_plane_width/2 for the right-most pixel
            // 2) the y component of the direction varies from +image_plane_height/2 for the top-most pixel to
            //    -image_plane_height/2 for the bottom-most pixel
            // 3) the z component is -1
            //
            // Make sure to calculate the ray directions to go through the center of each pixel
            Vec3f ray_origin;
            Vec3f ray_direction;
            auto  ray = Ray3f(ray_origin, ray_direction);

            // Generate a visual color for the ray so we can debug our ray directions
            ray_image(x, y) = ray2color(ray);
        }
    }

    string filename("scenes/assignment1/01_manual_ray_image.png");
    spdlog::info("Saving ray image to {}....", filename);
    ray_image.save(filename);
}

/// Learn about how darts uses JSON
void test_JSON()
{
    // Darts also includes a C++ library (https://github.com/nlohmann/json)
    // for parsing and manipulating JSON data.
    //
    // JSON is a human-readible data interchange format for expressing
    // attribute-value pairs. You can read more about it here:
    //      https://en.wikipedia.org/wiki/JSON
    //      https://www.json.org/
    //
    // In darts, we will use it for two purposes:
    //  1) As a generic way to pass named parameters to functions
    //  2) As a way to specify and load text-based scene files

    fmt::print("\n");
    fmt::print("--------------------------------------------------------\n"
               "PROGRAMMING ASSIGNMENT, PART 2: passing data using JSON \n"
               "--------------------------------------------------------\n");

    float   f(2.f);
    string  s("a text string");
    Color3f c3f(1.0f, .25f, .5f);
    Vec3f   v3f(2, 3, 4);
    Vec4f   v4f(2, 3, 4, 5);
    Vec3f   n3f(2, 3, 4);
    spdlog::info("Original darts data:\nf = {},\ns = {},\nc3f = {},\nv3f = {},\nv4f "
                 "= {},\nn3f = {}.",
                 f, s, c3f, v3f, v4f, n3f);

    // All the basic darts data-types can easily be stored in a JSON object
    json j;
    j["my float"]   = f;
    j["my string"]  = s;
    j["my color"]   = c3f;
    j["my vector3"] = v3f;
    j["my normal"]  = n3f;
    spdlog::info("The JSON object contains:\n{}.", j.dump(4));

    // We can also read these structures back out of the JSON object
    float  f2 = j["my float"];
    string s2 = j["my string"];
    Color3f c3f2 = j["my color3"];
    Vec3f v3f2 = j["my vector3"];
    Vec3f n3f2 = j["my normal"];

    spdlog::info("Retrieved darts data:\nf2 = {},\ns2 = {},\nc3f2 = {},\nv3f2 = "
                 "{},\nn3f2 = {}.",
                 f2, s2, c3f2, v3f2, n3f2);
    // TODO: There is a bug in the code above, and c3f2 doesn't have the same
    // value as the original c3f. Fix it.

    // Now we will pass a json object in place of explicit parameters to
    // a function. Go to the function below and implement the TODO.
    json parameters = {{"radius", 2.3f}};
    function_with_JSON_parameters(parameters);
}

void function_with_JSON_parameters(const json &j)
{
    // Many of the constructors for ray tracing in darts take a JSON object. This
    // allows us to have a uniform interface for creating these structures while
    // allowing the constructors to retrieve the necessary values from the JSON
    // object. This will simplify our code for writing a parser for reading
    // scene files from disk.

    // Sometimes we may want to make a parameter optional, and take on some
    // default value if it is not specified.
    // Unfortunately, checking for a missing parameter using e.g. j["radius"]
    // will throw an exception if the parameter doesn't exist (if j is const).
    // Instead, we can use j.value<type>("name", default_value) to extract it.
    // This is what the constructors to Camera, Sphere, Quad, and Materials do.

    // TODO: Replace the below two lines to extract the parameters radius (default=1.f),
    // and center (default={0,0,0}) from the JSON object j
    float radius = 1.f;
    Vec3f center = Vec3f(0.f);
    spdlog::info("The radius is: {}", radius);
    spdlog::info("The center is:\n{}", center);
}

/// Generating the same image as in #test_manual_camera_image(), but using the Camera class
void test_camera_class_image()
{
    fmt::print("\n");
    fmt::print("--------------------------------------------------------\n"
               "PROGRAMMING ASSIGNMENT, PART 3: Camera class generate_ray\n"
               "--------------------------------------------------------\n");

    // Setup the output image
    Image3f ray_image(200, 100);

    // Set up a camera with some reasonable parameters, using JSON
    // TODO: Look in camera.h|.cpp and implement the camera constructor
    Camera camera({{"vfov", 90.f}, {"resolution", Vec2i(ray_image.width(), ray_image.height())}, {"fdist", 1.f}});

    // loop over all pixels and ask the camera to generate a ray
    for (auto y : range(ray_image.height()))
    {
        for (auto x : range(ray_image.width()))
        {
            // TODO: Look in camera.h|.cpp and implement Camera::generate_ray

            // We add 0.5 to the pixel coordinate to center the ray within the pixel
            auto ray        = camera.generate_ray(Vec2f(x + 0.5f, y + 0.5f));
            ray_image(x, y) = ray2color(ray);
        }
    }

    string filename("scenes/assignment1/01_camera_ray_image.png");
    spdlog::info("Saving ray image to {}....", filename);
    ray_image.save(filename);
}

/// Working with darts #Transform
void test_transforms()
{
    fmt::print("\n");
    fmt::print("--------------------------------------------------------\n"
               "PROGRAMMING ASSIGNMENT, PART 4: Transforms              \n"
               "--------------------------------------------------------\n");

    // Darts also provides you with a Transform class.
    // Transform is a helper class that helps you transform geometric primitives
    // correctly. Internally, it keeps track of a transformation matrix and its
    // inverse

    // Let's create a random transformation matrix
    Mat44f transformation_matrix({-0.846852f, 0.107965f, -0.520755f, 0.0f}, {-0.492958f, -0.526819f, 0.692427f, 0.0f},
                                 {-0.199586f, 0.843093f, 0.499359f, 0.0f}, {-0.997497f, 0.127171f, -0.613392f, 1.0f});

    // Now that we have a matrix, we can create a transform from it:
    Transform transform{transformation_matrix};

    // TODO: Go to transform.h and implement all required methods there. If you
    // implement them correctly, the code below will work:

    // Let's create some random geometric objects...

    Vec3f vector{-0.997497f, 0.127171f, -0.6133920f};
    Vec3f point{0.617481f, 0.170019f, -0.0402539f};
    Vec3f normal{-0.281208f, 0.743764f, 0.6064130f};
    Ray3f ray{{-0.997497f, 0.127171f, -0.613392f}, {0.962222f, 0.264941f, -0.0627278f}};

    spdlog::info("vector = {}.", vector);
    spdlog::info("point  = {}.", point);
    spdlog::info("normal = {}.", normal);
    spdlog::info("ray.o  = {};\nray.d  = {}.", ray.o, ray.d);

    // ...and let's transform them!
    // We can transform things simply by multiplying it with the transform.
    // Let's check if you did it correctly:
    Vec3f transformed_vector = transform.vector(vector);
    Vec3f transformed_point  = transform.point(point);
    Vec3f transformed_normal = transform.normal(normal);
    Ray3f transformed_ray    = transform.ray(ray);

    Vec3f correct_transformed_vector(0.904467f, -0.6918370f, 0.301205f);
    Vec3f correct_transformed_point(-1.596190f, 0.0703303f, -0.837324f);
    Vec3f correct_transformed_normal(-0.249534f, 0.0890737f, 0.96426f);
    Vec3f correct_transformed_ray_position(-0.0930302f, -0.564666f, -0.312187f);
    Vec3f correct_transformed_ray_direction(-0.932945f, -0.088575f, -0.348953f);

    float vector_error = maxelem(abs(correct_transformed_vector - transformed_vector));
    float point_error  = maxelem(abs(correct_transformed_point - transformed_point));
    float normal_error = maxelem(abs(correct_transformed_normal - transformed_normal));
    float ray_error    = std::max(maxelem(abs(correct_transformed_ray_position - transformed_ray.o)),
                                  maxelem(abs(correct_transformed_ray_direction - transformed_ray.d)));

    spdlog::info("The forward transform matrix is\n{}.", transform.m);
    spdlog::info("The inverse transform matrix is\n{}.", transform.m_inv);

    spdlog::info("Result of transform*vector is:\n{}, and it should be:\n{}.", transformed_vector,
                 correct_transformed_vector);
    if (vector_error > 1e-5f)
        spdlog::error("Result incorrect!\n");
    else
        spdlog::info("Result correct!\n\n");

    spdlog::info("Result of transform*point is:\n{}, and it should be:\n{}.", transformed_point,
                 correct_transformed_point);
    if (point_error > 1e-5f)
        spdlog::error("Result incorrect!\n");
    else
        spdlog::info("Result correct!\n\n");

    spdlog::info("Result of transform*normal is:\n{}, and it should be:\n{}.", transformed_normal,
                 correct_transformed_normal);
    if (normal_error > 1e-5f)
        spdlog::error("Result incorrect!\n");
    else
        spdlog::info("Result correct!\n\n");

    spdlog::info("transform*ray: transformed_ray.o is:\n{}, and it should be:\n{}.", transformed_ray.o,
                 correct_transformed_ray_position);
    spdlog::info("transform*ray: transformed_ray.d is:\n{}, and it should be:\n{}.", transformed_ray.d,
                 correct_transformed_ray_direction);
    if (ray_error > 1e-5f)
        spdlog::error("Result incorrect!\n");
    else
        spdlog::info("Result correct!\n\n");
}

/// Finally, we render an image like before, but allow our camera to be positioned and oriented using a Transform
void test_xformed_camera_image()
{
    fmt::print("\n");
    fmt::print("--------------------------------------------------------\n"
               "PROGRAMMING ASSIGNMENT, PART 5: Transformed camera      \n"
               "--------------------------------------------------------\n");

    // Setup the output image
    Image3f ray_image(200, 100);

    // Set up a camera with some reasonable parameters
    // TODO: Look in camera.h|.cpp and implement the camera constructor
    Camera camera({
        {"vfov", 90.f},
        {"resolution", Vec2i(ray_image.width(), ray_image.height())},
        {"fdist", 1.f},
        {"transform", {{"from", {5.0f, 15.0f, -25.0f}}, {"to", {0.0f, 0.0f, 0.0f}}, {"up", {0.0f, 1.0f, 0.0f}}}},
    });

    // Generate a ray for each pixel in the ray image
    for (auto y : range(ray_image.height()))
    {
        for (auto x : range(ray_image.width()))
        {
            // TODO: Look in camera.h|.cpp and implement camera.generate_ray

            // Make sure to take the camera transform into account!

            // We add 0.5 to the pixel coordinate to center the ray within the pixel
            auto ray        = camera.generate_ray(Vec2f(x + 0.5f, y + 0.5f));
            ray_image(x, y) = ray2color(ray);
        }
    }

    string filename("scenes/assignment1/01_xformed_camera_ray_image.png");
    spdlog::info("Saving ray image to {}....", filename);
    ray_image.save(filename);
}

/// Now, we will implement ray-sphere intersection
void test_ray_sphere_intersection()
{
    fmt::print("\n");
    fmt::print("--------------------------------------------------------\n"
               "PROGRAMMING ASSIGNMENT, PART 6: Ray-Sphere intersection \n"
               "--------------------------------------------------------\n");

    // TODO: Go to sphere.cpp and implement Sphere::intersect

    // Let's check if your implementation was correct:
    // Don't worry about the complex DartsFactory syntax for now, we will explain it in PART 8
    auto   material = DartsFactory<Material>::create(json{{"type", "lambertian"}, {"albedo", 1.f}});
    Sphere test_sphere(1.f, material);

    spdlog::info("Testing untransformed sphere intersection");
    Ray3f   test_ray(Vec3f(-0.25f, 0.5f, 4.0f), Vec3f(0.0f, 0.0f, -1.0f));
    HitInfo hit;
    if (test_sphere.intersect(test_ray, hit))
    {
        float correct_t = 3.170844f;
        Vec3f correct_p(-0.25f, 0.5f, 0.829156f);
        Vec3f correct_n(-0.25f, 0.5f, 0.829156f);

        spdlog::info("Hit sphere! Distance is:\n{}, and it should be:\n{}.", hit.t, correct_t);
        spdlog::info("Intersection point is:\n{}, and it should be:\n{}.", hit.p, correct_p);
        spdlog::info("Intersection normal is:\n{}, and it should be:\n{}.", hit.sn, correct_n);

        float sphere_error =
            std::max({maxelem(abs(correct_p - hit.p)), maxelem(abs(correct_n - hit.sn)), std::abs(correct_t - hit.t)});

        if (sphere_error > 1e-5f)
            spdlog::error("Result incorrect!");
        else
            spdlog::info("Result correct!\n");
    }
    else
        spdlog::error("Sphere intersection incorrect! Should hit sphere");

    // Now, let's check if you implemented sphere transforms correctly!
    auto   transform = Transform::axisOffset(Vec3f(2.0f, 0.0f, 0.0f), // x-axis
                                             Vec3f(0.0f, 1.0f, 0.0f), // y-axis
                                             Vec3f(0.0f, 0.0f, 0.5f), // z-axis
                                             Vec3f(0.0f, 0.25f, 5.0f) // translation
      );
    Sphere transformed_sphere(1.0f, nullptr, transform);
    test_ray = Ray3f(Vec3f(1.0f, 0.5f, 8.0f), Vec3f(0.0f, 0.0f, -1.0f));
    hit      = HitInfo();

    spdlog::info("Testing transformed sphere intersection");
    if (transformed_sphere.intersect(test_ray, hit))
    {
        float correct_t = 2.585422f;
        Vec3f correct_p(1.0f, 0.5f, 5.41458f);
        Vec3f correct_n(0.147442f, 0.147442f, 0.978019f);

        spdlog::info("Hit sphere! Distance is:\n{}, and it should be:\n{}.", hit.t, correct_t);
        spdlog::info("Intersection point is:\n{}, and it should be:\n{}.", hit.p, correct_p);
        spdlog::info("Intersection normal is:\n{}, and it should be:\n{}.", hit.sn, correct_n);

        float sphere_error =
            std::max({maxelem(abs(correct_p - hit.p)), maxelem(abs(correct_n - hit.sn)), std::abs(correct_t - hit.t)});

        if (sphere_error > 1e-5f)
            spdlog::error("Result incorrect!");
        else
            spdlog::info("Result correct!\n");
    }
    else
        spdlog::error("Transformed sphere intersection incorrect! Should hit sphere");
}

/// Now: Let's allow our camera to be positioned and oriented using a Transform, and will use it to raytrace a Sphere
void test_sphere_image()
{
    fmt::print("\n");
    fmt::print("--------------------------------------------------------\n"
               "PROGRAMMING ASSIGNMENT, PART 7: False-color sphere image\n"
               "--------------------------------------------------------\n");

    // Setup the output image
    Image3f ray_image(200, 100);

    // Set up a camera with some reasonable parameters
    Camera camera({
        {"vfov", 90.f},
        {"resolution", Vec2i(ray_image.size_x(), ray_image.size_y())},
        {"fdist", 1.f},
        {"transform", {{"from", {5.0f, 15.0f, -25.0f}}, {"to", {0.0f, 0.0f, 0.0f}}, {"up", {0.0f, 1.0f, 0.0f}}}},
    });

    auto   material = DartsFactory<Material>::create(json{{"type", "lambertian"}, {"albedo", 1.f}});
    Sphere sphere(20.f, material);

    // Generate a ray for each pixel in the ray image
    for (auto y : range(ray_image.height()))
    {
        for (auto x : range(ray_image.width()))
        {
            // TODO: Look in camera.h|.cpp and implement camera.generate_ray

            // Make sure to take the camera transform into account!

            // We add 0.5 to the pixel coordinate to center the ray within the
            // pixel
            auto ray = camera.generate_ray(Vec2f(x + 0.5f, y + 0.5f));

            // If we hit the sphere, output the sphere normal; otherwise,
            // convert the ray direction into a color so we can have some visual
            // debugging
            ray_image(x, y) = intersection2color(ray, sphere);
        }
    }

    string filename("scenes/assignment1/01_xformed_camera_sphere_image.png");
    spdlog::info("Saving ray image to {}....", filename);
    ray_image.save(filename);
}

/// It's time to test materials!
void test_materials()
{
    fmt::print("\n");
    fmt::print("--------------------------------------------------------\n"
               "PROGRAMMING ASSIGNMENT, PART 8: Materials               \n"
               "--------------------------------------------------------\n");

    // TODO: Look at material.h|cpp and then go implement the Lambertian and Metal materials in lambertian.cpp and
    // metal.cpp, respectively

    // Note the line at the end of both files that looks something like this:
    // DARTS_REGISTER_CLASS_IN_FACTORY(Material, Lambertian, "lambertian")
    // This macro creates some code that informs a "Factory" how to create Materials of type Lambertian, and allows us
    // to later create them by providing the key "lambertian".

    // Let's see how this works by creating a red Lambertian material.
    //
    // The DartsFactory<Material>::create() function accepts a json object as a parameter, and returns a shared_ptr to a
    // Material. It determines the specific Material class to use by looking for a "type" field in the json object. In
    // this case it is "lambertian", which matches the third parameter in the DARTS_REGISTER_CLASS_IN_FACTORY macro
    // above
    Color3f surface_color    = Color3f(1.0f, 0.25f, 0.25f);
    auto    lambert_material = DartsFactory<Material>::create(json{{"type", "lambertian"}, {"albedo", surface_color}});

    // And now let's create a slightly shiny metal surface
    auto metal_material =
        DartsFactory<Material>::create(json{{"type", "metal"}, {"albedo", surface_color}, {"roughness", 0.3f}});

    // Later we will also use DartsFactory<Type>::create() with different classes for the Type, such as Surface,
    // Integrator, etc.

    // Let's create a fictitious hitpoint
    Vec3f   surface_point(1.0f, 2.0f, 0.0f);
    Vec3f   normal = normalize(Vec3f(1.0f, 2.0f, -1.0f));
    HitInfo hit;
    hit.t  = 0.0f;
    hit.p  = surface_point;
    hit.uv = Vec2f(0.0f, 0.0f);
    hit.gn = hit.sn = normal;

    // And a fictitious ray
    Ray3f ray(Vec3f(2.0f, 3.0f, -1.0f), Vec3f(-1.0f, -1.0f, 1.0f));
    Vec3f reflected = normalize(reflect(ray.d, normal));

    Vec3f   correct_origin      = surface_point;
    Color3f correct_attenuation = surface_color;

    // Now, let's test your implementation!
    spdlog::info("Testing Lambertian::scatter");
    float lambert_avg_cos   = 0.f;
    float max_lambert_error = 0.f;
    bool  scattering_failed = false;
    int   num_samples       = 100000;
    for (auto i : range(num_samples))
    {
        Ray3f   lambert_scattered;
        Color3f lambert_attenuation;
        if (lambert_material->scatter(ray, hit, lambert_attenuation, lambert_scattered))
        {
            lambert_avg_cos += dot(normal, normalize(lambert_scattered.d));
            max_lambert_error = std::max({maxelem(abs(correct_origin - lambert_scattered.o)),
                                          maxelem(abs(lambert_attenuation - correct_attenuation)), max_lambert_error});
        }
        else
            scattering_failed = true;
    }

    if (max_lambert_error > 1e-5f)
        spdlog::error("Lambertian scattered origin and attenuation error too high. Result might be incorrect!");

    if (scattering_failed)
        spdlog::error("Lambertian incorrect! Scatter function returned false\n");

    lambert_avg_cos /= num_samples;

    // The true average cosine can be computed as:
    // Integrate[(Cos[t]^2 Sin[t])/Pi, {p, 0, 2 Pi}, {t, 0, Pi/2}] = 2/3
    spdlog::info("Average cosine of samples is: {}; This should be close to 2/3.", lambert_avg_cos);
    if (std::abs(lambert_avg_cos - 2 / 3.f) > 2e-3f)
        spdlog::error("Result might be incorrect!");
    else
        spdlog::info("Result correct!\n");

    spdlog::info("Testing Metal::scatter");
    float metal_min_cos   = 100.f;
    float max_metal_error = 0.f;
    scattering_failed     = false;
    for (auto i : range(num_samples))
    {
        Ray3f   metal_scattered;
        Color3f metal_attenuation;
        if (metal_material->scatter(ray, hit, metal_attenuation, metal_scattered))
        {
            metal_min_cos   = std::min(dot(reflected, normalize(metal_scattered.d)), metal_min_cos);
            max_metal_error = std::max({maxelem(abs(correct_origin - metal_scattered.o)),
                                        maxelem(abs(metal_attenuation - correct_attenuation)), max_metal_error});
        }
        else
            scattering_failed = true;
    }

    if (max_metal_error > 1e-5f)
        spdlog::error("Metal scattered origin and attenuation error too high. Result might be incorrect!");

    if (scattering_failed)
        spdlog::error("Metal incorrect! Scatter function returned false\n");

    spdlog::info("Minimum cosine of samples is: {}; This should be close to 0.9539391.", metal_min_cos);
    if (std::abs(metal_min_cos - 0.9539391f) > 1e-2f)
        spdlog::error("Result might be incorrect!");
    else
        spdlog::info("Result correct!\n");
}

/// Now that we can scatter off of surfaces, let's try this out and render a scene with different materials
void test_recursive_raytracing()
{
    fmt::print("\n");
    fmt::print("--------------------------------------------------------\n"
               "PROGRAMMING ASSIGNMENT, PART 9: Recursive Ray Tracing   \n"
               "--------------------------------------------------------\n");

    // Setup the output image
    Image3f ray_image(300, 150);

    // We want to average over several rays to get a more pleasing result
    const int num_samples = 64;

    // Set up a camera with some reasonable parameters
    Camera camera({
        {"vfov", 45.f},
        {"resolution", Vec2i(ray_image.size_x(), ray_image.size_y())},
        {"fdist", 1.f},
        {"transform", {{"from", {1.9f, 0.8f, -3.5f}}, {"to", {1.9f, 0.8f, 0.0f}}, {"up", {0.0f, 1.0f, 0.0f}}}},
    });

    auto ground = DartsFactory<Material>::create(json{{"type", "lambertian"}, {"albedo", 0.5f}});
    auto matte  = DartsFactory<Material>::create(json{{"type", "lambertian"}, {"albedo", Vec3f(1.0f, 0.25f, 0.25f)}});
    auto shiny  = DartsFactory<Material>::create(json{{"type", "metal"}, {"albedo", 1.f}, {"roughness", 0.3f}});

    auto matte_sphere  = make_shared<Sphere>(1.f, matte, Transform::translate({3.f, 1.f, 0.f}));
    auto shiny_sphere  = make_shared<Sphere>(1.f, shiny, Transform::translate({0.f, 1.f, 1.f}));
    auto ground_sphere = make_shared<Sphere>(1000.f, ground, Transform::translate({0.f, -1000.f, 0.f}));

    // To raytrace more than one object at a time, we can put them into a group
    SurfaceGroup scene;
    scene.add_child(matte_sphere);
    scene.add_child(shiny_sphere);
    scene.add_child(ground_sphere);

    {
        Progress progress("Rendering", ray_image.size());
        // Generate a ray for each pixel in the ray image
        for (auto y : range(ray_image.height()))
        {
            for (auto x : range(ray_image.width()))
            {
                auto ray = camera.generate_ray(Vec2f(x + 0.5f, y + 0.5f));

                Color3f color = Color3f(0.0f);

                // TODO: Call recursive_color ``num_samples'' times and average the
                // results. Assign the average color to ``color''

                ray_image(x, y) = color;
                ++progress;
            }
        }
    } // progress reporter goes out of scope here

    string filename("scenes/assignment1/01_recursive_raytracing.png");
    spdlog::info("Saving rendered image to {} ...", filename);
    ray_image.save(filename);
}

Color3f recursive_color(const Ray3f &ray, const SurfaceGroup &scene, int depth)
{
    const int max_depth = 64;

    // TODO: Implement this function
    // Pseudo-code:
    //
    // if scene.intersect:
    // 		if depth < max_depth and hit_material.scatter(....) is successful:
    //			recursive_color = call this function recursively with the scattered ray and increased depth
    //          return attenuation * recursive_color
    //		else
    //			return black;
    // else:
    // 		return white

    return Color3f(0.0f);

}

Color3f vec2color(const Vec3f &dir)
{
    return 0.5f * (dir + 1.f);
}

Color3f ray2color(const Ray3f &r)
{
    return vec2color(normalize(r.d));
}

Color3f intersection2color(const Ray3f &r, const Sphere &sphere)
{
    HitInfo hit;
    if (sphere.intersect(r, hit))
        return vec2color(normalize(hit.sn));
    else
        return vec2color(normalize(r.d));
}
