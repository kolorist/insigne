#pragma once

#include "buffers.h"
#include "commons.h"

#include "detail/types.h"

namespace insigne {

// shading--------------------------------------
enum class shading_command_type_e {
	invalid = 0,
	shader_compile
};

struct shading_command_t {
	shading_command_type_e						command_type;
	union {
		// shader_compile
		struct {
			shader_handle_t						shader_handle;
			const_cstr							vs, fs;
			shader_reflection_t					reflection;
		};
	};
};

// buffers--------------------------------------
enum class buffers_command_type_e {
	invalid = 0,
	create_vb_buffers,
	create_ib_buffers,
	stream_vb_data,
	stream_ib_data
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
	};
};

}
