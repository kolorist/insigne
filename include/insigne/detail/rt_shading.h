#pragma once

#include <floral/stdaliases.h>
#include <floral/containers/fast_array.h>

#include "../memory.h"
#include "../commons.h"
#include "../commands.h"
#include "types.h"

// rt = render thread
// NOTE: if does not comment, these functions can and will only be executed in render thread internally

namespace insigne
{
namespace detail
{
// -------------------------------------------------------------------

using shaders_pool_t = floral::fast_fixed_array<shader_desc_t, linear_allocator_t>;
extern shaders_pool_t							g_shaders_pool;

const shader_handle_t							create_shader(const insigne::shader_desc_t& i_desc);	/* ut */
const shader_handle_t							get_last_shader();										/* ut */
void											compile_shader(shader_desc_t& io_desc, const_cstr i_vs, const_cstr i_fs, const shader_reflection_t& i_refl);
void											infuse_material(const shader_handle_t i_hdl, insigne::material_desc_t& o_mat); /* ut */

// -------------------------------------------------------------------

inline void										initialize_shading_module();
void											cleanup_shading_module();
void											process_shading_command_buffer(const size i_cmdBuffId);

// ---------------------------------------------
}
}

#include "rt_shading.hpp"
