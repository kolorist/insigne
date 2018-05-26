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
	void										dispatch_frame();

	void										set_depth_test(const bool i_enable);
	void										set_depth_write(const bool i_enable);
	void										set_depth_func(const compare_func_e i_func);
	void										set_cull_face(const bool i_enable);
	void										set_front_face(const front_face_e i_frontFace);
	//TODO: add stencil
	void										set_blend(const bool i_enable);
	void										set_blend_equation(const blend_equation_e i_blendEqu);
	void										set_blend_function(const factor_e i_sfactor, const factor_e i_dfactor);
	void										set_scissor(const bool i_enable);
	void										set_scissor_rect(const s32 i_x, const s32 i_y, const s32 i_width, const s32 i_height);

	void										set_clear_color(f32 i_red, f32 i_green, f32 i_blue, f32 i_alpha);

	// normal upload
	const texture_handle_t						upload_texture2d(const s32 i_width, const s32 i_height, const texture_format_e i_format, voidptr i_data);
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
