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
		struct {
			shader_handle_t						shader_handle;
			const_cstr							vs, fs;
			shader_reflection_t					reflection;
		};
	};
};

// ---------------------------------------------

}
