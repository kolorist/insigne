#include "insigne/memory.h"

namespace insigne
{

namespace detail
{
linear_allocator_t*								g_draw_cmdbuff_arena = nullptr;
linear_allocator_t*								g_post_draw_cmdbuff_arena = nullptr;
}

}
