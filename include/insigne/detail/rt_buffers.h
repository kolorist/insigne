#pragma once

#include "../memory.h"
#include "../commons.h"
#include "../commands.h"
#include "types.h"

namespace insigne
{
namespace detail
{

typedef floral::fixed_array<vbdesc_t, linear_allocator_t>	vbs_pool_t;
typedef floral::fixed_array<ibdesc_t, linear_allocator_t>	ibs_pool_t;
typedef floral::fixed_array<ubdesc_t, linear_allocator_t>	ubs_pool_t;

extern vbs_pool_t								g_vbs_pool;
extern ibs_pool_t								g_ibs_pool;
extern ubs_pool_t								g_ubs_pool;

const vb_handle_t								create_vb(const insigne::vbdesc_t& i_desc); /* ut */
const vb_handle_t								get_last_vb();								/* ut */
const ib_handle_t								create_ib(const insigne::ibdesc_t& i_desc); /* ut */
const ib_handle_t								get_last_ib();								/* ut */
const ub_handle_t								create_ub(const insigne::ubdesc_t& i_desc); /* ut */
const ub_handle_t								get_last_ub();								/* ut */

// ---------------------------------------------
inline void										initialize_buffers_module();
void											cleanup_buffers_module();
void											process_buffers_command_buffer(const size i_cmdBuffId);

}
}

#include "rt_buffers.hpp"
