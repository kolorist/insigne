#pragma once

#include <floral.h>
#include <helich.h>

namespace insigne
{
// -------------------------------------------------------------------

typedef helich::allocator<helich::stack_scheme, helich::no_tracking_policy> 	linear_allocator_t;
typedef helich::allocator<helich::freelist_scheme, helich::no_tracking_policy>	freelist_allocator_t;
typedef helich::allocator<helich::stack_scheme, helich::no_tracking_policy>		arena_allocator_t;
typedef helich::allocator<helich::freelist_scheme, helich::no_tracking_policy>	freelist_arena_t;

extern linear_allocator_t						g_persistance_allocator;
extern arena_allocator_t						g_arena_allocator;
extern freelist_allocator_t						g_stream_allocator;

extern freelist_allocator_t						g_temporal_shader_allocator;

namespace detail
{
// -------------------------------------------------------------------

extern linear_allocator_t*						g_draw_cmdbuff_arena;
extern linear_allocator_t*						g_post_draw_cmdbuff_arena;
extern linear_allocator_t*						g_resource_arena;

// -------------------------------------------------------------------
}

// -------------------------------------------------------------------
}
