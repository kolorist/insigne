#pragma once

#include "../memory.h"
#include "../commons.h"
#include "../commands.h"
#include "types.h"

namespace insigne {
namespace detail {

typedef floral::fixed_array<vbdesc_t, linear_allocator_t>	vbs_pool_t;
typedef floral::fixed_array<ibdesc_t, linear_allocator_t>	ibs_pool_t;

extern vbs_pool_t								g_vbs_pool;
extern ibs_pool_t								g_ibs_pool;

const vb_handle_t								create_vb(const insigne::vbdesc_t& i_desc); /* ut */
const ib_handle_t								create_ib(const insigne::ibdesc_t& i_desc); /* it */

// ---------------------------------------------
inline void										initialize_buffers_module();
void											process_buffers_command_buffer();

}
}

#include "rt_buffers.hpp"