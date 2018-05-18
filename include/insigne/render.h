#pragma once

#include <floral.h>

#include "commons.h"
#include "internal_commons.h"
#include "buffers.h"
#include "memory.h"
#include "generated_code/proxy.h"

namespace insigne {
	// -----------------------------------------
	typedef floral::fixed_array<gpu_command, linear_allocator_t>	gpu_command_buffer_t;

#define BUFFERED_FRAMES							3

	template <typename t_surface>
	struct draw_command_buffer_t {
		static gpu_command_buffer_t				command_buffer[BUFFERED_FRAMES];
	};
	extern gpu_command_buffer_t					s_generic_command_buffer[BUFFERED_FRAMES];

	extern floral::condition_variable			s_init_condvar;
	extern floral::mutex						s_init_mtx;
	extern floral::condition_variable			s_cmdbuffer_condvar;
	extern floral::mutex						s_cmdbuffer_mtx;

	extern arena_allocator_t*					s_gpu_frame_allocator[BUFFERED_FRAMES];
	extern size									s_front_cmdbuff;
	extern size									s_back_cmdbuff;
	extern render_state_t						s_render_state;
	extern u32									s_render_state_changelog;
	extern floral::fixed_array<material_t, linear_allocator_t>	s_materials;
	extern material_handle_t					s_current_material;

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

#if 0
	const surface_handle_t						upload_surface(voidptr i_vertices, voidptr i_indices,
													s32 i_stride, const u32 i_vcount, const u32 i_icount);
	const surface_handle_t						upload_surface(voidptr i_vertices, voidptr i_indices,
													s32 i_stride, const u32 i_vcount, const u32 i_icount, const draw_type_e i_drawType);
	void										update_surface(const surface_handle_t& i_hdl,
													voidptr i_vertices, voidptr i_indices,
													const u32 i_vcount, const u32 i_icount);
#endif

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

	/*
	 * 3 cases:
	 * 	- the material is identical with previous draw call: just draw the geometry
	 * 	- the material is different with previous draw call: re-bind it
	 * 	- the material parameters are different with previous draw call: update them
	 */
	/*
	void										draw_surface(const surface_handle_t i_surfaceHdl, const material_handle_t i_matHdl);
	void										draw_surface_segmented(const surface_handle_t i_surfaceHdl, const material_handle_t i_matHdl,
													const s32 i_segSize, const voidptr i_segOffset);
													*/
	template <typename TSurface>
	void										draw_surface(const surface_handle_t i_surfaceHdl, const material_handle_t i_matHdl);
	template <typename TSurface>
	void										draw_surface_segmented(const surface_handle_t i_surfaceHdl, const material_handle_t i_matHdl,
													const s32 i_segSize, const voidptr i_segOffset);
}

#include "render.hpp"
