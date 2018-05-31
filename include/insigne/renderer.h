#pragma once

#include <floral.h>

#include "memory.h"
#include "commons.h"
#include "internal_commons.h"

namespace insigne {
namespace renderer {

	void										initialize_renderer();

	// -----------------------------------------
	template <typename t_switch>
	void										set_depth_test(const compare_func_e i_depthFunc);
	template <typename t_switch>
	void										set_depth_write();
	template <typename t_switch>
	void										set_cull_face(const front_face_e i_frontFace);
	template <typename t_switch>
	void										set_blending(const blend_equation_e i_blendEqu, const factor_e i_sfactor, const factor_e i_dfactor);
	template <typename t_switch>
	void										set_scissor_test(const s32 i_x, const s32 i_y, const s32 i_width, const s32 i_height);
	template <typename t_switch>
	void										set_stencil_test(const compare_func_e i_func, const u32 i_mask, const s32 i_ref, const operation_e i_sfail, const operation_e i_dpfail, const operation_e i_dppass);
	void										enable_vertex_attrib(const u32 i_location);
	void										describe_vertex_data(const u32 i_location, const s32 i_size,
													const data_type_e i_type, const bool i_normalized,
													const s32 i_stride, const voidptr offset);

	// -----------------------------------------
	void										clear_color(const floral::vec4f& i_color);
	void										clear_framebuffer(const bool i_clearcolor, const bool i_cleardepth);

	texture_handle_t							create_texture();
	void										upload_texture2d(const texture_handle_t& i_hdl, const s32 i_width,
													const s32 i_height, const texture_format_e i_format,
													const texture_internal_format_e i_internalFormat,
													const data_type_e i_dataType, voidptr i_data,
													const filtering_e i_minFil = filtering_e::nearest,
													const filtering_e i_magFil = filtering_e::nearest);

	// with built-in mipmaps
	void										upload_texture2d_mm(const texture_handle_t& i_hdl, const s32 i_width,
													const s32 i_height, const texture_format_e i_format,
													const texture_internal_format_e i_internalFormat,
													const data_type_e i_dataType, voidptr i_data,
													const filtering_e i_minFil = filtering_e::nearest,
													const filtering_e i_magFil = filtering_e::nearest);
	
	shader_handle_t								create_shader(const shader_param_list_t* i_paramList);
	void										compile_shader(shader_handle_t& i_hdl, const_cstr i_vertstr, const_cstr i_fragstr);
	void										compile_shader(const shader_handle_t& i_hdl, const_cstr i_vertStr, const_cstr i_fragStr, const shader_param_list_t* i_paramList);
	const material_template_t&					get_material_template(const shader_handle_t& i_shaderHdl);

	surface_handle_t							create_surface();
	void										upload_surface(surface_handle_t& i_hdl, voidptr i_vertices, voidptr i_indices,
													const u32 i_vcount, const u32 i_icount, s32 i_stride,
													const draw_type_e i_drawType = draw_type_e::static_surface);
	void										update_surface(const surface_handle_t& i_hdl, voidptr i_vertices,
													voidptr i_indices, const u32 i_vcount, const u32 i_icount);

	template <typename t_surface>
	void										draw_surface_idx(const surface_handle_t& i_surfaceHdl, const material_t& i_matSnapshot,
													const s32 i_segSize, const voidptr i_segOffset);
}
}

// TODO: #ifdef here for compilation?
#include "renderer_gl.hpp"
