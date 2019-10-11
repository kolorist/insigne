#pragma once

#include "buffers.h"
#include "commons.h"

#include "detail/types.h"

namespace insigne {

// render---------------------------------------
enum class render_command_type_e {
	invalid = 0,
	present_render,
	draw_state_scissor,
	draw_call,
	framebuffer_create,
	framebuffer_activate,
	framebuffer_capture,
	clean_up_snapshot
};

struct render_command_t {
	render_command_t() {
		// we have to declare this because the default constructor of this struct was deleted
		// due to insigne::framebuffer_desc_t has a floral::vec4f param (which overrides the default constructor)
	}

	render_command_type_e						command_type;
	union {
		// present_render
		struct  {
		} present_render_data;
		// framebuffer_create
		struct {
			framebuffer_handle_t				fb_handle;
			insigne::framebuffer_desc_t			desc;
		} framebuffer_create_data;
		// framebuffer_activate
		struct {
			framebuffer_handle_t				fb_handle;
			s32									x, y, width, height;
		} framebuffer_activate_data;
		// framebuffer_capture
		struct {
			voidptr								pixel_data;
			framebuffer_handle_t				fb_handle;
		} framebuffer_capture_data;
		// clean_up_snapshot
		struct {
			framebuffer_handle_t				downto_handle;
		} clean_up_snapshot_data;
	};
};

// draw-----------------------------------------

enum class draw_command_type_e {
	invalid = 0,
	state_setup_scissor,
	draw_surface
};

struct draw_command_t {
	draw_command_type_e							command_type;
	union {
		struct {
			material_desc_t*					material_snapshot;
			s32									segment_offset;
			vb_handle_t							vb_handle;
			ib_handle_t							ib_handle;
			s32									segment_size;
		} draw_surface_data;

		struct {
			s32									x, y;			// top-left corner
			s32									width, height;
			bool								enabled;
		} state_setup_scissor_data;
	};
};


// textures-------------------------------------
enum class textures_command_type_e {
	invalid = 0,
	create_texture,
	stream_texture,
	clean_up_snapshot
};

struct textures_command_t {
	textures_command_type_e						command_type;
	union {
		// create_texture
		struct {
			texture_handle_t					texture_handle;
			insigne::texture_desc_t				desc;
		} create_texture_data;
		// stream_texture_data
		struct {
			texture_handle_t					texture_handle;
			size								dataSize;
			voidptr								data;
		} stream_texture_data;
		// clean_up_snapshot
		struct {
			texture_handle_t					downto_handle;
		} clean_up_snapshot_data;
	};
};

// shading--------------------------------------
enum class shading_command_type_e {
	invalid = 0,
	shader_compile,
	clean_up_snapshot
};

struct shading_command_t {
	shading_command_type_e						command_type;
	union {
		// shader_compile
		struct {
			shader_handle_t						shader_handle;
			const_cstr							vs, fs;
			shader_reflection_t					reflection;
		} shader_compile_data;
		// clean_up_snapshot
		struct {
			shader_handle_t						downto_handle;
		} clean_up_snapshot_data;
	};
};

// buffers--------------------------------------
enum class buffers_command_type_e {
	invalid = 0,
	create_vb_buffers,
	create_ib_buffers,
	create_ub_buffers,
	stream_vb_data,
	stream_ib_data,
	stream_ub_data,
	clean_up_snapshot
};

struct buffers_command_t {
	buffers_command_type_e						command_type;
	union {
		// create_vb_buffers
		struct {
			vb_handle_t							vb_handle;
			insigne::vbdesc_t					desc;
		} create_vb_data;
		// create_ib_buffers
		struct {
			ib_handle_t							ib_handle;
			insigne::ibdesc_t					desc;
		} create_ib_data;
		// create_ub_buffers
		struct {
			ub_handle_t							ub_handle;
			insigne::ubdesc_t					desc;
		} create_ub_data;
		// stream_vb_data
		struct {
			voidptr								data;
			vb_handle_t							vb_handle;
			u32									vcount;
			u32									offset_elements;
		} stream_vb_data;
		// stream_ib_data
		struct {
			voidptr								data;
			ib_handle_t							ib_handle;
			u32									icount;
			u32									offset_elements;
		} stream_ib_data;
		// stream_ub_data
		struct {
			voidptr								data;
			size								data_size;
			size								offset;
			ub_handle_t							ub_handle;
		} stream_ub_data;
		// clean_up_snapshot
		struct {
			vb_handle_t							downto_vb;
			ib_handle_t							downto_ib;
			ub_handle_t							downto_ub;
		} clean_up_snapshot_data;
	};
};

}
