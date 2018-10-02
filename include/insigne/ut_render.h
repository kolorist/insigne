#pragma once

#include <floral.h>

#include "commons.h"

namespace insigne {

// -----------------------------------------
template <typename t_surface>
struct renderable_surface_t {
	static void									render();
	static void									init_buffer(insigne::linear_allocator_t* i_allocator);
};

// render entrypoint----------------------------
void											begin_frame();
void											end_frame();

void											begin_render_pass(const framebuffer_handle_t i_fb);
void											begin_render_pass(const framebuffer_handle_t i_fb, const s32 i_x, const s32 i_y, const s32 i_width, const s32 i_height);
void											end_render_pass(const framebuffer_handle_t i_fb);
void											mark_present_render();
void											dispatch_render_pass();
// framebuffer----------------------------------
color_attachment_list_t*						allocate_color_attachment_list(const u32 i_attachCount); // deprecated
framebuffer_descriptor_t						create_framebuffer_descriptor(const u32 i_colorAttachCount);
const framebuffer_handle_t						create_framebuffer(const s32 i_width, const s32 i_height,
													const f32 i_scale, const bool i_hasDepth, const color_attachment_list_t* i_colorAttachs); // deprecated
const framebuffer_handle_t						create_framebuffer(const framebuffer_descriptor_t& i_desc);
const framebuffer_handle_t						create_mega_framebuffer(const s32 i_width, const s32 i_height,
													const bool i_hasDepth, const color_attachment_list_t* i_colorAttachs);
const texture_handle_t							extract_color_attachment(const framebuffer_handle_t i_fbHdl, const s32 i_idx);
const texture_handle_t							extract_depth_stencil_attachment(const framebuffer_handle_t i_fbHdl);


// drawcall-------------------------------------
template <typename TSurface>
void											draw_surface(const surface_handle_t i_surfaceHdl, const material_handle_t i_matHdl);
template <typename t_surface>
void											draw_surface(const vb_handle_t i_vb, const ib_handle_t i_ib, const material_handle_t i_mat);
template <typename TSurface>
void											draw_surface_segmented(const surface_handle_t i_surfaceHdl, const material_handle_t i_matHdl,
													const s32 i_segSize, const voidptr i_segOffset);
}

#include "ut_render.hpp"
