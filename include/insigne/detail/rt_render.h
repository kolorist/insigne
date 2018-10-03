#pragma once

#include <floral.h>

#include "../commons.h"

namespace insigne {
namespace detail {

template <typename t_switch>
void											set_depth_test(const compare_func_e i_depthFunc);
template <typename t_switch>
void											set_depth_write();
template <typename t_switch>
void											set_cull_face(const face_side_e i_faceSide, const front_face_e i_frontFace);
template <typename t_switch>
void											set_blending(const blend_equation_e i_blendEqu, const factor_e i_sfactor, const factor_e i_dfactor);
template <typename t_switch>
void											set_scissor_test(const s32 i_x, const s32 i_y, const s32 i_width, const s32 i_height);
template <typename t_switch>
void											set_stencil_test(const compare_func_e i_func, const u32 i_mask, const s32 i_ref, const operation_e i_sfail, const operation_e i_dpfail, const operation_e i_dppass);

void 											enable_vertex_attrib(const u32 i_location);
void 											describe_vertex_data(const u32 i_location, const s32 i_size,
													const data_type_e i_type, const bool i_normalized, const s32 i_stride, const voidptr offset);

// ---------------------------------------------
template <typename t_surface_list>
void											initialize_render_module();
const bool										process_render_command_buffer();
template <typename t_surface_list>
void											process_draw_command_buffer();

}
}

#include "rt_render.hpp"
