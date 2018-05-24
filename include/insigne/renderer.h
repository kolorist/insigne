#pragma once

#include <floral.h>

#include "memory.h"
#include "commons.h"
#include "internal_commons.h"
#include "insigne/gl/identifiers.h"

namespace insigne {
namespace renderer {

	// -----------------------------------------
	// this need to go to 'detail'
	struct param_t {
		floral::crc_string						name;
		GLuint									id;

		param_t() { }
		param_t(const_cstr i_name, const GLuint i_id)
			: name(i_name)
			, id(i_id)
		{ }
	};

	struct shader {
		GLuint									gpu_handle;

		//floral::inplace_array<param_t, 8u>		int_params;
		floral::inplace_array<GLuint, 8u>		float_params;
		floral::inplace_array<GLuint, 8u>		vec3_params;
		floral::inplace_array<GLuint, 4u>		mat4_params;
		floral::inplace_array<GLuint, 4u>		texture2d_params;
		/*
		floral::inplace_array<param_t, 4u>		texture_cube_params;
		floral::inplace_array<param_t, 8u>		vec2_params;
		floral::inplace_array<param_t, 8u>		vec3_params;
		floral::inplace_array<param_t, 8u>		vec4_params;
		floral::inplace_array<param_t, 4u>		mat3_params;
		floral::inplace_array<param_t, 4u>		mat4_params;
		*/
	};

	struct surface {
		s32										stride;
		draw_type_e								draw_type;
		u32										icount;
		GLuint									vbo;
		GLuint									ibo;
	};

	struct texture {
		GLuint									gpu_handle;
		u32										width;
		u32										height;
		GLenum									format;
		GLenum									internal_format;
	};

	typedef floral::fixed_array<shader, linear_allocator_t>					shader_array_t;
	typedef floral::fixed_array<material_template_t, linear_allocator_t>	material_template_array_t;
	typedef floral::fixed_array<texture, linear_allocator_t>				texture_array_t;
	typedef floral::fixed_array<surface, linear_allocator_t>				surface_array_t;

	extern shader_array_t						s_shaders;
	extern material_template_array_t			s_material_templates;
	extern texture_array_t						s_textures;
	extern surface_array_t						s_surfaces;

	// -----------------------------------------
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
	
	shader_handle_t								create_shader(const shader_param_list_t* i_paramList);
	void										compile_shader(shader_handle_t& i_hdl, const_cstr i_vertstr, const_cstr i_fragstr);
	void										compile_shader(const shader_handle_t& i_hdl, const_cstr i_vertStr, const_cstr i_fragStr, const shader_param_list_t* i_paramList);
	const material_template_t&					get_material_template(const shader_handle_t& i_shaderHdl);

	/*
	material_handle_t							create_material();
	void										compile_material(const material_handle_t& i_matHdl, const shader_handle_t& i_shaderHdl, material_param_list_t& i_paramList);
	*/

	surface_handle_t							create_surface();
	void										upload_surface(surface_handle_t& i_hdl, voidptr i_vertices, voidptr i_indices,
													const u32 i_vcount, const u32 i_icount, s32 i_stride,
													const draw_type_e i_drawType = draw_type_e::static_surface);
	void										update_surface(const surface_handle_t& i_hdl, voidptr i_vertices,
													voidptr i_indices, const u32 i_vcount, const u32 i_icount);
	void										draw_surface_idx(const surface_handle_t& i_surfaceHdl, const material_t& i_matSnapshot,
													const s32 i_segSize, const voidptr i_segOffset);

	template <typename t_surface>
	void										draw_surface_idx(const surface_handle_t& i_surfaceHdl, const material_t& i_matSnapshot,
													const s32 i_segSize, const voidptr i_segOffset);
}
}

#include "renderer_gl.hpp"
