#pragma once

#include <floral.h>

#include "../memory.h"
#include "../commons.h"
#include "../commands.h"
#include "types.h"

// rt = render thread
// NOTE: if does not comment, these functions can and will only be executed in render thread internally

namespace insigne {
namespace detail {

typedef floral::fixed_array<shader_desc_t, linear_allocator_t>	shaders_pool_t;

extern shaders_pool_t							g_shaders_pool;

const shader_handle_t							create_shader(const insigne::shader_desc_t& i_desc); /* ut */
void											compile_shader(const shader_handle_t& i_hdl, const shader_desc_t& i_desc);

inline void										initialize_shading_module();
void											process_shading_command_buffer();

}
}

#include "rt_shading.hpp"
