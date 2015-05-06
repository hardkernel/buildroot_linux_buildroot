/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2014 ARM Limited
 *     ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

#include "culling.hpp"

void CullingInterface::compute_frustum_from_view_projection(vec4 *planes, const mat4 &view_projection)
{
    mat4 inv_view_proj = mat_inverse(view_projection);

    // Get world-space coordinates for clip-space bounds.
    vec4 lbn = inv_view_proj * vec4(-1, -1, -1, 1);
    vec4 ltn = inv_view_proj * vec4(-1, 1, -1, 1);
    vec4 lbf = inv_view_proj * vec4(-1, -1, 1, 1);
    vec4 rbn = inv_view_proj * vec4( 1, -1, -1, 1);
    vec4 rtn = inv_view_proj * vec4( 1, 1, -1, 1);
    vec4 rbf = inv_view_proj * vec4( 1, -1, 1, 1);
    vec4 rtf = inv_view_proj * vec4( 1, 1, 1, 1);

    vec3 lbn_pos = vec_project(lbn);
    vec3 ltn_pos = vec_project(ltn);
    vec3 lbf_pos = vec_project(lbf);
    vec3 rbn_pos = vec_project(rbn);
    vec3 rtn_pos = vec_project(rtn);
    vec3 rbf_pos = vec_project(rbf);
    vec3 rtf_pos = vec_project(rtf);

    // Get plane equations for all sides of frustum.
    vec3 left_normal = vec_normalize(vec_cross(lbf_pos - lbn_pos, ltn_pos - lbn_pos));
    vec3 right_normal = vec_normalize(vec_cross(rtn_pos - rbn_pos, rbf_pos - rbn_pos));
    vec3 top_normal = vec_normalize(vec_cross(ltn_pos - rtn_pos, rtf_pos - rtn_pos));
    vec3 bottom_normal = vec_normalize(vec_cross(rbf_pos - rbn_pos, lbn_pos - rbn_pos));
    vec3 near_normal = vec_normalize(vec_cross(ltn_pos - lbn_pos, rbn_pos - lbn_pos));
    vec3 far_normal = vec_normalize(vec_cross(rtf_pos - rbf_pos, lbf_pos - rbf_pos));

    planes[0] = vec4(left_normal, -vec_dot(left_normal, lbn_pos)); // Left
    planes[1] = vec4(right_normal, -vec_dot(right_normal, rbn_pos)); // Right
    planes[2] = vec4(near_normal, -vec_dot(near_normal, lbn_pos)); // Near
    planes[3] = vec4(far_normal, -vec_dot(far_normal, lbf_pos)); // Far
    planes[4] = vec4(top_normal, -vec_dot(top_normal, ltn_pos)); // Top
    planes[5] = vec4(bottom_normal, -vec_dot(bottom_normal, lbn_pos)); // Bottom
}

