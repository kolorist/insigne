#pragma once

#include <floral.h>

#include "commons.h"
#include "buffers.h"
#include "memory.h"

namespace insigne {
	// -----------------------------------------
	struct renderer_settings_t {
		u32										frame_allocator_size_mb;
		u32										frame_shader_allocator_size_mb;
		u32										frame_buffers_allocator_size_mb;
		u32										draw_command_buffer_size;
		u32										generic_command_buffer_size;
	};
	extern renderer_settings_t					g_renderer_settings;

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
	void										begin_render_pass(const framebuffer_handle_t i_fb, const s32 i_x, const s32 i_y, const s32 i_width, const s32 i_height);
	void										end_render_pass(const framebuffer_handle_t i_fb);
	void										mark_present_render();
	void										dispatch_render_pass();

	void										set_clear_color(f32 i_red, f32 i_green, f32 i_blue, f32 i_alpha);

	// framebuffer
	color_attachment_list_t*					allocate_color_attachment_list(const u32 i_attachCount); // deprecated
	framebuffer_descriptor_t					create_framebuffer_descriptor(const u32 i_colorAttachCount);
	const framebuffer_handle_t					create_framebuffer(const s32 i_width, const s32 i_height,
													const f32 i_scale, const bool i_hasDepth, const color_attachment_list_t* i_colorAttachs); // deprecated
	const framebuffer_handle_t					create_framebuffer(const framebuffer_descriptor_t& i_desc);
	const framebuffer_handle_t					create_mega_framebuffer(const s32 i_width, const s32 i_height,
													const bool i_hasDepth, const color_attachment_list_t* i_colorAttachs);
	const texture_handle_t						extract_color_attachment(const framebuffer_handle_t i_fbHdl, const s32 i_idx);
	const texture_handle_t						extract_depth_stencil_attachment(const framebuffer_handle_t i_fbHdl);

	// normal upload
	const texture_handle_t						create_texture2d(const s32 i_width, const s32 i_height,
													const texture_format_e i_format,
													const filtering_e i_minFilter, const filtering_e i_magFilter,
													const size i_dataSize, voidptr& o_placeholderData, const bool i_hasMM = false); // deprecated
	const texture_handle_t						create_texture2d(const texture2d_descriptor_t& i_desc, voidptr& o_placeholderData);
	const texture_handle_t						upload_texture2d(const s32 i_width, const s32 i_height,
													const texture_format_e i_format,
													const filtering_e i_minFilter, const filtering_e i_magFilter,
													voidptr i_data, const bool i_hasMM = false); // deprecated
	const texture_handle_t						upload_texture2d(const texture2d_descriptor_t& i_desc, voidptr i_data);

	const texture_handle_t						create_texturecube(const s32 i_width, const s32 i_height,
													const texture_format_e i_format,
													const filtering_e i_minFilter, const filtering_e i_magFilter,
													const size i_dataSize, voidptr& o_placeholderData, const bool i_hasMM = false);
	const texture_handle_t						upload_texturecube(const s32 i_width, const s32 i_height,
													const texture_format_e i_format,
													const filtering_e i_minFilter, const filtering_e i_magFilter,
													voidptr i_data, const bool i_hasMM = false);

	// streaming
	voidptr										create_stream_texture2d(texture_format_e i_format);
	void										update_stream_texture2d();

	const surface_handle_t						upload_surface(voidptr i_vertices, const size i_vsize, voidptr i_indices, const size i_isize,
													const s32 i_stride, const u32 i_vcount, const u32 i_icount);
	const surface_handle_t						create_streamed_surface(const s32 i_stride);
	const surface_handle_t						create_streamed_surface(const surface_descriptor_t& i_desc);
	void										update_streamed_surface(const surface_handle_t& i_hdl,
													voidptr i_vertices, const size i_vsize, voidptr i_indices, const size i_isize,
													const u32 i_vcount, const u32 i_icount);
	void										update_streamed_surface_vertices(const surface_handle_t i_hdl, voidptr i_vertices, const u32 i_vcount);
	void										update_streamed_surface_indices(const surface_handle_t i_hdl, voidptr i_indices, const u32 i_icount);

	shader_param_list_t*						allocate_shader_param_list(const u32 i_paramCount);
	const shader_handle_t						compile_shader(const_cstr i_vertStr, const_cstr i_fragStr, const shader_param_list_t* i_paramList);

	const material_handle_t						create_material(const shader_handle_t i_fromShader);
	template <typename t_param_type>
	const param_id								get_material_param(const material_handle_t i_hdl, const_cstr i_name);
	const param_id								get_material_param_texcube(const material_handle_t i_hdl, const_cstr i_name);
	const param_id								get_material_param_ub(const material_handle_t i_hdl, const_cstr i_name);
	template <typename t_param_type>
	void										set_material_param(const material_handle_t i_hdl, const param_id i_paramId, const t_param_type& i_value);
	void										set_material_param_texcube(const material_handle_t i_hdl, const param_id i_paramId, const texture_handle_t i_tex);
	void										set_material_param_ub(const material_handle_t i_hdl, const param_id i_paramId, const ub_handle_t i_ub);

	template <typename t_surface>
	void										set_scissor_test(const bool i_enable = false, const s32 i_x = 0, const s32 i_y = 0, const s32 i_width = 0, const s32 i_height = 0);

	template <typename TSurface>
	void										draw_surface(const surface_handle_t i_surfaceHdl, const material_handle_t i_matHdl);
	template <typename t_surface>
	void										draw_surface(const vb_handle_t i_vb, const ib_handle_t i_ib, const material_handle_t i_mat);
	template <typename TSurface>
	void										draw_surface_segmented(const surface_handle_t i_surfaceHdl, const material_handle_t i_matHdl,
													const s32 i_segSize, const voidptr i_segOffset);
}

#include "render.hpp"
