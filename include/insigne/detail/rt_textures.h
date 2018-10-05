#pragma once

#include <floral.h>

#include "../memory.h"
#include "../commons.h"
#include "../commands.h"
#include "types.h"

namespace insigne {
namespace detail {

typedef floral::fixed_array<texture_desc_t, linear_allocator_t>	textures_pool_t;
extern textures_pool_t							g_textures_pool;

const texture_handle_t							create_texture(const insigne::texture_desc_t& i_desc); /* ut */

// ---------------------------------------------
void											initialize_textures_module();
void											process_textures_command_buffer(const size i_cmdBuffId);

}
}
