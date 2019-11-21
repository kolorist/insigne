#pragma once

// ut = user thread

#include <floral.h>

#include "commons.h"
#include "commands.h"
#include "detail/rt_render.h"

namespace insigne
{

// render entrypoint----------------------------
/* GPU synchronization point
 * 	wait for SwapBuffer()
 */
void											begin_frame();

void											end_frame();

// renderpass entrypoint------------------------
void											begin_render_pass(const framebuffer_handle_t i_fb);
/* render only a specific region of a framebuffer
 * origin:
 * 	x: 0 -> framebuffer_width from left to right
 * 	y: 0 -> framebuffer_height from top to bottom
 *
 * NOTE: OpenGL screen coordinate:
 * 	x: 0 -> framebuffer_width from left to right
 * 	y: 0 -> framebuffer_height from bottom to top
 */
void											begin_render_pass(const framebuffer_handle_t i_fb, const s32 i_x, const s32 i_y, const s32 i_width, const s32 i_height);
void											begin_render_pass(const framebuffer_handle_t i_fb, const cubemap_face_e i_face);
void											begin_render_pass(const framebuffer_handle_t i_fb, const cubemap_face_e i_face, const s32 i_mip);
/* do nothing, LUL */
void											end_render_pass(const framebuffer_handle_t i_fb);
/* mark that this render pass will be the last one,
 * SwapBuffer() will be call in this render pass
 * CPU waiting is done by end_frame() function
 */
void											mark_present_render();
/* trigger command buffer process of render thread 
 * and swap to a new command buffer for user thread
 */
void											dispatch_render_pass();

// framebuffer----------------------------------
framebuffer_desc_t								create_framebuffer_desc();
const framebuffer_handle_t						create_framebuffer(const framebuffer_desc_t& i_desc);
const texture_handle_t							extract_color_attachment(const framebuffer_handle_t i_fb, const u32 i_idx);
const texture_handle_t							extract_depth_stencil_attachment(const framebuffer_handle_t i_fb);
const u64										schedule_framebuffer_capture(const framebuffer_handle_t i_fb, voidptr o_data);

// drawcall-------------------------------------
template <typename t_surface>
void											draw_surface(const vb_handle_t i_vb, const ib_handle_t i_ib, const material_desc_t& i_mat);
template <typename t_surface>
void											draw_surface(const vb_handle_t i_vb, const ib_handle_t i_ib, const material_desc_t& i_mat, const s32 i_idxOffset, const s32 i_elemCount);

// origin (0; 0) locates at the lower-left corner
template <typename t_surface>
void											setup_scissor(const bool i_enabled, const s32 i_x = 0, const s32 i_y = 0, const s32 i_width = 0, const s32 i_height = 0);

// ---------------------------------------------
void											cleanup_render_module();
void											push_draw_command(const ssize i_surfaceTypeIdx, const draw_command_t& i_cmd);
ssize											get_render_resource_state();
void											cleanup_render_resource(const ssize i_stateId);

}

#include "ut_render.hpp"
