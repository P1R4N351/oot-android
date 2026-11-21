// SPDX-License-Identifier: MIT
// Zero-allocation 3D spatial breadcrumb trail, elevation-band classifier,
// and 8-slot session waypoint radar projection for second-screen companions.
// Synced from P1R4N351/nx-second-screen (include/nx_companion/companion_trail.h).
#ifndef NX_COMPANION_TRAIL_H
#define NX_COMPANION_TRAIL_H

#include <assert.h>
#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#define NX_TRAIL_CAP 128U
#define NX_WAYPOINT_CAP 8U
#define NX_SCENE_MAX 64U
#define NX_PIN_LABEL_MAX 24U
#define NX_TRAIL_PI_F 3.14159265358979323846f

enum {
    NX_BAND_BELOW = -1,
    NX_BAND_LEVEL = 0,
    NX_BAND_ABOVE = 1
};

typedef struct NxTrailPoint {
    float x;
    float y;
    float z;
    uint32_t valid;
} NxTrailPoint;

typedef struct NxWaypoint {
    float x;
    float y;
    float z;
    char label[NX_PIN_LABEL_MAX];
    uint32_t active;
} NxWaypoint;

typedef struct NxCompanionTrail {
    char scene[NX_SCENE_MAX];
    NxTrailPoint ring[NX_TRAIL_CAP];
    uint32_t head;
    uint32_t count;
    float min_step_sq;
    float floor_band_m;
    NxWaypoint pins[NX_WAYPOINT_CAP];
} NxCompanionTrail;

static inline void nx_trail_clear(NxCompanionTrail *t) {
    assert(t != NULL);
    memset(t->ring, 0, sizeof(t->ring));
    memset(t->pins, 0, sizeof(t->pins));
    t->head = 0U;
    t->count = 0U;
}

static inline void nx_trail_init(
    NxCompanionTrail *t, float min_step_m, float floor_band_m) {
    assert(t != NULL);
    memset(t, 0, sizeof(*t));
    float step = (isfinite(min_step_m) && min_step_m > 0.0f) ? min_step_m : 1.5f;
    float band = (isfinite(floor_band_m) && floor_band_m > 0.0f) ? floor_band_m : 3.0f;
    t->min_step_sq = step * step;
    t->floor_band_m = band;
}

static inline int nx_trail_coord_ok(float x, float y, float z) {
    return isfinite(x) && isfinite(y) && isfinite(z) &&
           fabsf(x) < 1e5f && fabsf(y) < 1e5f && fabsf(z) < 1e5f;
}

static inline int nx_trail_push(
    NxCompanionTrail *t, const char *scene, float x, float y, float z) {
    if (!t || !scene || scene[0] == '\0' || !nx_trail_coord_ok(x, y, z)) return 0;
    if (strncmp(t->scene, scene, NX_SCENE_MAX) != 0) {
        nx_trail_clear(t);
        snprintf(t->scene, NX_SCENE_MAX, "%s", scene);
    }
    if (t->count > 0U) {
        uint32_t prev = (t->head + NX_TRAIL_CAP - 1U) % NX_TRAIL_CAP;
        float dx = x - t->ring[prev].x;
        float dy = y - t->ring[prev].y;
        float dz = z - t->ring[prev].z;
        if ((dx * dx + dy * dy + dz * dz) < t->min_step_sq) return 0;
    }
    t->ring[t->head].x = x;
    t->ring[t->head].y = y;
    t->ring[t->head].z = z;
    t->ring[t->head].valid = 1U;
    t->head = (t->head + 1U) % NX_TRAIL_CAP;
    if (t->count < NX_TRAIL_CAP) t->count++;
    return 1;
}

static inline int nx_trail_get_oldest(
    const NxCompanionTrail *t, uint32_t idx, NxTrailPoint *out) {
    if (!t || !out || idx >= t->count) return 0;
    uint32_t start = (t->head + NX_TRAIL_CAP - t->count) % NX_TRAIL_CAP;
    uint32_t slot = (start + idx) % NX_TRAIL_CAP;
    *out = t->ring[slot];
    return out->valid != 0U;
}

static inline int nx_trail_floor_band(
    const NxCompanionTrail *t, float ref_y, float pt_y) {
    float band = (t && t->floor_band_m > 0.0f) ? t->floor_band_m : 3.0f;
    float dy = pt_y - ref_y;
    if (dy > band) return NX_BAND_ABOVE;
    if (dy < -band) return NX_BAND_BELOW;
    return NX_BAND_LEVEL;
}

static inline int nx_trail_project_xz(
    float ref_x, float ref_z, float yaw_deg, int heading_up, float radius_m,
    float pt_x, float pt_z, float *out_nx, float *out_ny) {
    if (!out_nx || !out_ny || !isfinite(radius_m) || radius_m <= 1e-3f) return 0;
    if (!isfinite(ref_x) || !isfinite(ref_z) || !isfinite(pt_x) || !isfinite(pt_z)) return 0;
    float dx = pt_x - ref_x;
    float dz = pt_z - ref_z;
    float rx = dx;
    float rz = dz;
    if (heading_up) {
        float y = isfinite(yaw_deg) ? yaw_deg : 0.0f;
        float rad = y * (NX_TRAIL_PI_F / 180.0f);
        float c = cosf(rad);
        float s = sinf(rad);
        rx = dx * c + dz * s;
        rz = -dx * s + dz * c;
    }
    float nx = rx / radius_m;
    float ny = rz / radius_m;
    float mag = hypotf(nx, ny);
    if (mag > 1.0f) {
        *out_nx = nx / mag;
        *out_ny = ny / mag;
        return 2;
    }
    *out_nx = nx;
    *out_ny = ny;
    return 1;
}

static inline int nx_trail_pin_add(
    NxCompanionTrail *t, float x, float y, float z, const char *label) {
    if (!t || !nx_trail_coord_ok(x, y, z)) return -1;
    for (uint32_t i = 0U; i < NX_WAYPOINT_CAP; i++) {
        if (t->pins[i].active) continue;
        t->pins[i].x = x;
        t->pins[i].y = y;
        t->pins[i].z = z;
        t->pins[i].active = 1U;
        if (label && label[0] != '\0') {
            snprintf(t->pins[i].label, NX_PIN_LABEL_MAX, "%s", label);
        } else {
            snprintf(t->pins[i].label, NX_PIN_LABEL_MAX, "Pin %u", i + 1U);
        }
        return (int)i;
    }
    return -1;
}

static inline int nx_trail_pin_remove(NxCompanionTrail *t, uint32_t slot) {
    if (!t || slot >= NX_WAYPOINT_CAP || !t->pins[slot].active) return 0;
    memset(&t->pins[slot], 0, sizeof(t->pins[slot]));
    return 1;
}

static inline void nx_trail_pins_clear(NxCompanionTrail *t) {
    if (!t) return;
    memset(t->pins, 0, sizeof(t->pins));
}

#ifdef __cplusplus
}
#endif

#endif
