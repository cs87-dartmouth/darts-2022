# Darts Blender Add-on

This add-on provides support for exporting a Blender scene as a Darts `.json` scene file with meshes saved as OBJ files in a `meshes` subdirectory and textures in a `textures` subdirectory. The plugin is primarily useful for converting blender objects and positioning the camera, though it does provide rudimentary support for converting some Blender materials to Darts materials.

## Features:
* Camera:
  * Exports the camera position, resolution, field-of-view, and depth-of-field settings (only perspective cameras are supported)
* Background:
  * Blender's background color or a (optionally rotated) environment map `"envmap"` gets exported to the darts `"background"` field
* Integrator:
  * Optionally sets an `"integrator"` depending on export settings
* Sampler:
  * Optionally sets an `"sampler"` depending on export settings. Sets the number of samples according to the Cycles Render "Max samples" field
* Materials:
  * Can either export a single default `"lambertian"` material for all surfaces, a placeholder `"lambertian"` material for each Blender surface, or it can convert each Blender material into a corresponding Darts material as follows:
    * Diffuse BSDF -> `"lambertian"` ("Roughness" parameter is ignored)
    * Emission -> `"diffuse_light"`
    * Glass BSDF -> `"dielectric"` or `"rough dielectric"` depending whether distribution is set to "Sharp"
    * Glossy BSDF -> `"(rough) conductor"`, `"blinn-phong"`, `"phong"`, or `"metal"` depending on export settings
    * Mix Shader -> `"blend"`, supports scalar blend factors as well as Fresnel blends
  * Additionally, for all of the above materials, if a normal map is specified in the shader, the exporter will wrap the material inside a darts `"normal map"` material
  * Textures are supported for many of the parameters of the above shaders, and written to a `textures/` subdirectory
* Surfaces:
  * All meshes, fonts, metaballs, and NURBS surfaces are exported as OBJ meshes into a `meshes/` subdirectory
  * Options to write out UVs, normals, and apply modifiers (like subdivision surfaces)
  * Objects can be exported as a single OBJ file for the whole scene, or split into separate OBJ files per Blender object. Darts supports multiple materials for different faces of a single mesh (for each OBJ material, Darts looks for a Darts material with the same name, otherwise it uses the material set for the entire mesh).
  * Can disable mesh and/or texture output. If nothing about the geometry changes (e.g. only camera or material parameters), this can considerably speed up export.
* Lights:
  * All Blender lights are converted to corresponding Darts lights:
    * Point -> `"point light"`
    * Sun -> `"sun light"`
    * Spot -> `"spot light"`
    * Area -> `"quad"`, `"sphere"`, or `"disk"` (depending on the light shape) with an emissive `"diffuse_light"` material (note that these will be visible by rays in Darts, even though they are invisible "geometry" in Blender)

## Installation

- Create a `.zip` file of this directory
- In Blender, go to **Edit** -> **Preferences** -> **Add-ons** -> **Install**.
- Select the ZIP archive
- After the plugin is installed, it has to be activated by clicking the checkbox next to it in the Add-ons menu.

### Requirements

* `Blender >= 2.93`
