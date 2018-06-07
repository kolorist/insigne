#pragma once

#include <floral.h>

#include "commons.h"
#include "buffers.h"
#include "memory.h"

namespace insigne {

	// -----------------------------------------
	template <typename t_surface>
	struct renderable_surface_t {
		static void								render();
		static void								init_buffer(insigne::linear_allocator_t* i_allocator);
	};

	// -----------------------------------------
	template <typename t_surface_list>
	void										initialize_render_thread();

	void										wait_for_initialization();

	void										begin_frame();
	void										end_frame();

	void										begin_render_pass(const framebuffer_handle_t i_fb);
	void										end_render_pass(const framebuffer_handle_t i_fb);
	void										mark_present_render();
	void										dispatch_render_pass();

	void										set_clear_color(f32 i_red, f32 i_green, f32 i_blue, f32 i_alpha);

	// framebuffer
	color_attachment_list_t*					allocate_color_attachment_list(const u32 i_attachCount);
	const framebuffer_handle_t					create_framebuffer(const s32 i_width, const s32 i_height,
													const f32 i_scale, const bool i_hasDepth, const color_attachment_list_t* i_colorAttachs);
	const texture_handle_t						extract_color_attachment(const framebuffer_handle_t i_fbHdl, const s32 i_idx);

	// normal upload
	const texture_handle_t						create_texture2d(const s32 i_width, const s32 i_height,
													const texture_format_e i_format,
													const filtering_e i_minFilter, const filtering_e i_magFilter,
													const size i_dataSize, voidptr& o_placeholderData, const bool i_hasMM = false);
	const texture_handle_t						upload_texture2d(const s32 i_width, const s32 i_height,
													const texture_format_e i_format,
													const filtering_e i_minFilter, const filtering_e i_magFilter,
													voidptr i_data, const bool i_hasMM = false);

	// streaming
	voidptr										create_stream_texture2d(texture_format_e i_format);
	void										update_stream_texture2d();

	const surface_handle_t						upload_surface(voidptr i_vertices, const size i_vsize, voidptr i_indices, const size i_isize,
													const s32 i_stride, const u32 i_vcount, const u32 i_icount);
	const surface_handle_t						create_streamed_surface(const s32 i_stride);
	void										update_streamed_surface(const surface_handle_t& i_hdl,
													voidptr i_vertices, const size i_vsize, voidptr i_indices, const size i_isize,
													const u32 i_vcount, const u32 i_icount);

	shader_param_list_t*						allocate_shader_param_list(const u32 i_paramCount);
	const shader_handle_t						compile_shader(const_cstr i_vertStr, const_cstr i_fragStr, const shader_param_list_t* i_paramList);

	const material_handle_t						create_material(const shader_handle_t i_fromShader);
	template <typename t_param_type>
	const param_id								get_material_param(const material_handle_t i_hdl, const_cstr i_name);
	template <typename t_param_type>
	void										set_material_param(const material_handle_t i_hdl, const param_id i_paramId, const t_param_type& i_value);

	template <typename TSurface>
	void										draw_surface(const surface_handle_t i_surfaceHdl, const material_handle_t i_matHdl);
	template <typename TSurface>
	void										draw_surface_segmented(const surface_handle_t i_surfaceHdl, const material_handle_t i_matHdl,
													const s32 i_segSize, const voidptr i_segOffset);
}

#include "render.hpp"
