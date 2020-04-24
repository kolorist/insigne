#pragma once

#include <floral/stdaliases.h>
#include <floral/containers/fast_array.h>

#include "../commons.h"
#include "../memory.h"
#include "insigne/detail/types.h"

namespace insigne
{
namespace detail
{
// ------------------------------------------------------------------

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
void											set_polygon_mode(const polygon_mode_e i_mode);

void 											enable_vertex_attrib(const u32 i_location);
void 											disable_vertex_attrib(const u32 i_location);
void 											describe_vertex_data(const u32 i_location, const s32 i_size,
													const data_type_e i_type, const bool i_normalized, const s32 i_stride, const voidptr offset);

// ------------------------------------------------------------------
using framebuffers_pool_t = floral::fast_fixed_array<framebuffer_desc_t, linear_allocator_t>;
extern framebuffers_pool_t						g_framebuffers_pool;

const framebuffer_handle_t						create_framebuffer(const insigne::framebuffer_desc_t& i_desc);	/* ut */
const framebuffer_handle_t						get_last_framebuffer();											/* ut */
const texture_handle_t							extract_color_attachment(const framebuffer_handle_t i_fb, const u32 i_idx); /* ut */
const texture_handle_t							extract_depth_stencil_attachment(const framebuffer_handle_t i_fb); /* ut */

// ------------------------------------------------------------------
void											initialize_render_module();
void											cleanup_render_module();
const bool										process_render_command_buffer(const size i_cmdBuffId);
void											process_draw_command_buffer(const size i_cmdBuffId);
void											process_post_draw_command_buffer(const size i_cmdBuffId);

// ------------------------------------------------------------------
}
}
