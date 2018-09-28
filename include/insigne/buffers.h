#pragma once

#include <floral.h>

#include <string.h>

#include "commons.h"
#include "internal_commons.h"

namespace insigne {

#define COMMAND_PAYLOAD_SIZE					64
#define GPU_COMMAND_BUFFER_SIZE					1024u

enum class command {
	invalid = 0,
	setup_init_state,
	setup_render_state_toggle,
	setup_blending_state,
	setup_scissor_state,
	init_framebuffer,
	setup_framebuffer,
	refresh_framebuffer,
	draw_geom,
	load_data,
	stream_data,
	present_render
};

// this struct is copyable
struct gpu_command {
	command									opcode;

	u8										payload[COMMAND_PAYLOAD_SIZE];
	size									pos;
	size									psize;

	gpu_command()
		: opcode(command::invalid)
		, pos(0)
		, psize(0)
	{ }

	template <class t_target>
	void deserialize(const t_target& i_target)
	{
		size dataSize = sizeof(t_target);
		memcpy(&payload[pos], &i_target, dataSize);
		pos += dataSize;
		psize += dataSize;
	}

	template <class t_target>
	void serialize(t_target& o_target)
	{
		size dataSize = sizeof(t_target);
		memcpy(&o_target, &payload[pos], dataSize);
		pos += dataSize;
	}

	void empty()							{ psize = 0; }
	void reset_cursor()						{ pos = 0; }
};

// ------------payload----------------------
struct init_command {
	floral::vec4f							clear_color;
};

enum class depth_cmd_e {
	toggle = 0,
	change_func
};

struct render_state_toggle_command {
	render_state_togglemask_e				toggle;
	bool									to_value;
	union {
		// depth_test
		struct {
			compare_func_e					depth_func;
		};
		// depth_write
		struct {
		};
		// cull_face
		struct {
			front_face_e					front_face;
		};
		// blending
		struct {
			blend_equation_e				blend_equation;
			factor_e						blend_func_sfactor;
			factor_e						blend_func_dfactor;
		};
		// scissor_test
		struct {
			s32								x, y, width, height;
		};
		// stencil_test
		struct {
			compare_func_e					stencil_func;
			u32								stencil_mask;
			s32								stencil_ref;
			operation_e						stencil_op_sfail;
			operation_e						stencil_op_dpfail;
			operation_e						stencil_op_dppass;
		};
	};
};

struct blending_state_command {
	compare_func_e							blend_equation;
	factor_e								blend_sfactor;
	factor_e								blend_dfactor;
	bool									blend_enable;
};

struct scissor_state_command {
	s32										x, y;
	s32										width, height;
	bool									scissor_test;
};

struct framebuffer_refresh_command {
	s32										x, y, width, height;
	bool									region_clear;
	bool									clear_color_buffer;
	bool									clear_depth_buffer;
};

struct framebuffer_setup_command {
	framebuffer_handle_t					framebuffer_idx;
	s32										lower_left_x, lower_left_y;
	s32										width, height;
	bool									has_custom_viewport;
};

struct framebuffer_init_command {
	const color_attachment_list_t*			color_attachment_list;
	framebuffer_handle_t					framebuffer_idx;
	s32										width, height;
	f32										scale;
	bool									has_depth;
};

struct render_command {
	material_t*								material_snapshot;
	voidptr									segment_offset;
	surface_handle_t						surface_handle;
	s32										segment_size;
};

enum class stream_type {
	invalid = 0,
	texture2d,
	texture_cube,
	geometry,
	geometry_vertices,
	geometry_indices,
	shader
};

struct load_command {
	stream_type								data_type;
	union {
		// texture
		struct {
			voidptr							data;
			texture_handle_t				texture_idx;
			texture_format_e				format;
			texture_internal_format_e		internal_format;
			data_type_e						pixel_data_type;
			filtering_e						min_filter;
			filtering_e						mag_filter;
			s32								width, height;
			bool							has_builtin_mipmaps;
		};

		// geom
		struct {
			voidptr							vertices;
			voidptr							indices;
			draw_type_e						draw_type;
			s32								stride;
			u32								vcount;
			u32								icount;
			surface_handle_t				surface_idx;
			bool							has_indices;
		};

		// shader
		struct {
			const_cstr						vertex_str;
			const_cstr						fragment_str;
			const shader_param_list_t*		shader_param_list;
			shader_handle_t					shader_idx;
		};

	};

	load_command() {}
	~load_command() {}
};

struct stream_command {
	stream_type								data_type;
	union {
		// geom
		struct {
			voidptr							vertices;
			voidptr							indices;
			u32								vcount;
			u32								icount;
			surface_handle_t				surface_idx;
		};
		// geom_vertices
		struct {
			voidptr								vertices;
			u32									vcount;
			surface_handle_t					surface_idx;
		};
		// geom_indices
		struct {
			voidptr								indices;
			u32									icount;
			surface_handle_t					surface_idx;
		};
	};
};

struct present_render_command {
	u32										placeholder;
	// nothing
};

}
