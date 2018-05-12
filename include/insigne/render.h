#pragma once

#include <floral.h>

#include "commons.h"
#include "generated_code/proxy.h"

namespace insigne {

	void										initialize_render_thread();
	void										wait_for_initialization();

	void										begin_frame();
	void										end_frame();
	void										dispatch_frame();

	void										set_depth_test(const bool i_enable);
	void										set_depth_write(const bool i_enable);
	void										set_depth_func(const compare_func_e i_func);

	void										set_clear_color(f32 i_red, f32 i_green, f32 i_blue, f32 i_alpha);

	// normal upload
	const texture_handle_t						upload_texture2d(const s32 i_width, const s32 i_height, const texture_format_e i_format, voidptr i_data);
	// streaming
	voidptr										create_stream_texture2d(texture_format_e i_format);
	void										update_stream_texture2d();

	const surface_handle_t						upload_surface(voidptr i_vertices, voidptr i_indices,
													s32 i_stride, const u32 i_vcount, const u32 i_icount);
	const surface_handle_t						upload_surface(voidptr i_vertices, voidptr i_indices,
													s32 i_stride, const u32 i_vcount, const u32 i_icount, const draw_type_e i_drawType);
	void										update_surface(const surface_handle_t& i_hdl,
													voidptr i_vertices, voidptr i_indices,
													const u32 i_vcount, const u32 i_icount);

	shader_param_list_t*						allocate_shader_param_list(const u32 i_paramCount);
	const shader_handle_t						compile_shader(const_cstr i_vertStr, const_cstr i_fragStr, const shader_param_list_t* i_paramList);

	const material_handle_t						create_material(const shader_handle_t i_fromShader);
	template <typename t_param_type>
	const param_id								get_material_param(const material_handle_t i_hdl, const_cstr i_name);
	template <typename t_param_type>
	void										set_material_param(const material_handle_t i_hdl, const param_id i_paramId, const t_param_type& i_value);

	/*
	 * 3 cases:
	 * 	- the material is identical with previous draw call: just draw the geometry
	 * 	- the material is different with previous draw call: re-bind it
	 * 	- the material parameters are different with previous draw call: update them
	 */
	void										draw_surface(const surface_handle_t i_surfaceHdl, const material_handle_t i_matHdl);
}

//#include "render.hpp"
