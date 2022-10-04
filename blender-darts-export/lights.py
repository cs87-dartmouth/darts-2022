from mathutils import Matrix
import numpy as np


def convert_area_light(ctx, b_light):
    params = {}

    scale_mat = Matrix.Scale(1, 4)

    # Compute area
    if b_light.data.shape == 'SQUARE' or b_light.data.shape == 'RECTANGLE':
        params['type'] = 'quad'
        ctx.report(
            {'INFO'}, f"Light scale {b_light.scale.x} x {b_light.scale.y}")
        x = b_light.data.size
        y = (b_light.data.size_y
             if b_light.data.shape == 'RECTANGLE'
             else b_light.data.size)
        params['size'] = [x, y]
        area = x*y * b_light.scale.x * b_light.scale.y

    elif b_light.data.shape == 'DISK' or b_light.data.shape == 'ELLIPSE':
        params['type'] = 'disk'
        x = b_light.data.size
        y = (b_light.data.size_y
             if b_light.data.shape == 'ELLIPSE'
             else b_light.data.size)
        area = np.pi * x*y * b_light.scale.x * b_light.scale.y / 4.0
        params['radius'] = x / 2.0

        scale = Matrix()
        scale[0][0] = 1
        scale[1][1] = y / x
        scale_mat = scale @ scale_mat
    else:
        raise NotImplementedError(
            f"Light shape: {b_light.data.shape} is not supported")

    # object transform
    flip_normal = Matrix(
        ((1, 0, 0, 0), (0, 1, 0, 0), (0, 0, -1, 0), (0, 0, 0, 1)))
    params['transform'] = ctx.transform_matrix(
        b_light.matrix_world @ scale_mat @ flip_normal)

    # Conversion factor used in Cycles, to convert to irradiance (don't ask me why)
    conv_fac = 1.0 / (area * 4.0)
    params['material'] = {
        'type': 'diffuse_light',
        'emit': ctx.color(conv_fac * b_light.data.energy * b_light.data.color)
    }

    return params


def convert_point_light(ctx, b_light):
    params = {'type': 'point light'}
    params['radius'] = b_light.data.shadow_soft_size
    params['transform'] = {'translate': list(b_light.location)}
    params['power'] = ctx.color(b_light.data.energy * b_light.data.color)
    return params


def convert_sun_light(ctx, b_light):
    params = {'type': 'sun light'}
    params['angle'] = np.rad2deg(b_light.data.angle / 2.0)
    params['irradiance'] = ctx.color(b_light.data.energy * b_light.data.color)
    params['transform'] = ctx.transform_matrix(b_light.matrix_world)
    return params


def convert_spot_light(ctx, b_light):
    params = {'type': 'spot light'}
    params['radius'] = b_light.data.shadow_soft_size
    params['power'] = ctx.color(b_light.data.energy * b_light.data.color)
    params['cutoff angle'] = np.rad2deg(b_light.data.spot_size / 2.0)
    params['cutoff blur'] = b_light.data.spot_blend
    params['transform'] = ctx.transform_matrix(b_light.matrix_world)
    return params


def export_light(ctx, b_lights):

    light_converters = {
        'AREA': convert_area_light,
        'POINT': convert_point_light,
        'SUN': convert_sun_light,
        'SPOT': convert_spot_light
    }

    params = []
    for l in b_lights:
        try:
            ctx.report({'INFO'}, f"light type {l.data.type}")
            params.append(light_converters[l.data.type](ctx, l))
        except KeyError:
            ctx.report(
                {'WARNING'}, f"Could not export '{l.name_full}', light type {l.data.type} is not supported.")
        except NotImplementedError as err:
            ctx.report(
                {'WARNING'}, f"Error while exporting light: '{l.name_full}': {err.args[0]}.")

    return params
