#pragma once

#include <floral.h>

#include "commons.h"

namespace insigne {

// ---------------------------------------------
template <typename t_surface>
struct renderable_surface_t {
	static void									render();
	static void									init_buffer(insigne::linear_allocator_t* i_allocator);
};

// render entrypoint----------------------------
void											begin_frame();
/* GPU synchronization point
 * 	wait for SwapBuffer()
 */
void											end_frame();

// renderpass entrypoint------------------------
/* render at full resolution of a framebuffer */
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

// drawcall-------------------------------------
template <typename t_surface>
void											draw_surface(const vb_handle_t i_vb, const ib_handle_t i_ib, const material_handle_t i_mat);

// ---------------------------------------------
void											cleanup_render_module();
}

#include "ut_render.hpp"
