#pragma once

#include <atomic>

#include <floral.h>

#include "insigne/commons.h"
#include "insigne/buffers.h"
#include "insigne/memory.h"

namespace insigne {
namespace detail {

// synchronization
extern floral::condition_variable				g_init_condvar;
extern floral::mutex							g_init_mtx;

extern size										g_composing_cmdbuff;
extern std::atomic<bool>						g_scene_presented;

typedef floral::inplaced_ring_buffer_mt_spsc<size, BUFFERS_COUNT - 1> waiting_cmdbuffs_t;
extern waiting_cmdbuffs_t						g_waiting_cmdbuffs;

// render---------------------------------------
extern arena_allocator_t*						g_frame_render_allocator[BUFFERS_COUNT];
extern arena_allocator_t*						g_frame_draw_allocator[BUFFERS_COUNT];
extern gpu_command_buffer_t						g_render_command_buffer[BUFFERS_COUNT];
// shading--------------------------------------
extern arena_allocator_t*						g_frame_shader_allocator[BUFFERS_COUNT];
extern gpu_command_buffer_t						g_shading_command_buffer[BUFFERS_COUNT];
// buffers--------------------------------------
extern arena_allocator_t*						g_frame_buffers_allocator[BUFFERS_COUNT];
extern gpu_command_buffer_t						g_buffers_command_buffer[BUFFERS_COUNT];
// textures-------------------------------------
extern arena_allocator_t*						g_frame_textures_allocator[BUFFERS_COUNT];
extern gpu_command_buffer_t						g_textures_command_buffer[BUFFERS_COUNT];

}
}
