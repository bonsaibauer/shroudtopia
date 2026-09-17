#pragma once

#include "shroudtopia/api/world.h"
#include <math.h>
#include <limits.h>

/* Pure API utilities: these calculations do not require a loaded game world. */
typedef struct GridRegion {
    size_t struct_size;
    Aabb bounds;
    int32_t minimum[3];
    uint32_t dimensions[3];
    size_t cell_count;
} GridRegion;

/* Both marked cells are included. The resulting world bounds are half-open.
   Buffer order for world grid operations is x + width * (y + height * z). */
static inline Result GridRegionFromPoints(const Grid* grid,
    Vec3d a, Vec3d b, size_t maximum_cells, GridRegion* output) {
    if (!grid || !output || grid->struct_size < sizeof(*grid) ||
        output->struct_size < sizeof(*output) || maximum_cells == 0) return RESULT_INVALID_ARGUMENT;
    const double origins[3] = {grid->origin.x, grid->origin.y, grid->origin.z};
    const double steps[3] = {grid->cell_size.x, grid->cell_size.y, grid->cell_size.z};
    const double first[3] = {a.x, a.y, a.z}, second[3] = {b.x, b.y, b.z};
    double lo[3], hi[3];
    GridRegion result = {0};
    result.struct_size = sizeof(result);
    result.bounds.struct_size = sizeof(result.bounds);
    result.cell_count = 1;
    for (int axis = 0; axis < 3; ++axis) {
        if (!isfinite(origins[axis]) || !isfinite(steps[axis]) || steps[axis] <= 0 ||
            !isfinite(first[axis]) || !isfinite(second[axis])) return RESULT_INVALID_ARGUMENT;
        const double x = floor((first[axis] - origins[axis]) / steps[axis]);
        const double y = floor((second[axis] - origins[axis]) / steps[axis]);
        const double minimum = fmin(x, y), maximum = fmax(x, y);
        if (!isfinite(minimum) || !isfinite(maximum) || minimum < INT32_MIN || maximum >= INT32_MAX)
            return RESULT_INVALID_ARGUMENT;
        const double count = maximum - minimum + 1;
        if (count > INT32_MAX || count > (double)(maximum_cells / result.cell_count))
            return RESULT_INVALID_ARGUMENT;
        result.minimum[axis] = (int32_t)minimum;
        result.dimensions[axis] = (uint32_t)count;
        result.cell_count *= result.dimensions[axis];
        lo[axis] = origins[axis] + minimum * steps[axis];
        hi[axis] = origins[axis] + (maximum + 1) * steps[axis];
        if (!isfinite(lo[axis]) || !isfinite(hi[axis]) || hi[axis] <= lo[axis]) return RESULT_INVALID_ARGUMENT;
    }
    result.bounds.minimum.x = lo[0]; result.bounds.minimum.y = lo[1]; result.bounds.minimum.z = lo[2];
    result.bounds.maximum.x = hi[0]; result.bounds.maximum.y = hi[1]; result.bounds.maximum.z = hi[2];
    *output = result;
    return RESULT_OK;
}
