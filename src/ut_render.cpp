#include "insigne/ut_render.h"

#include <lotus/profiler.h>

#include "insigne/internal_states.h"
#include "insigne/counters.h"
#include "insigne/ut_shading.h"
#include "insigne/ut_buffers.h"
#include "insigne/ut_textures.h"

namespace insigne {

// ---------------------------------------------
inline arena_allocator_t* get_composing_allocator() {
	return detail::g_frame_render_allocator[detail::g_back_cmdbuff];
}

inline detail::gpu_command_buffer_t& get_composing_command_buffer() {
	return detail::g_render_command_buffer[detail::g_back_cmdbuff];
}
// ---------------------------------------------

// render entrypoint----------------------------
void begin_frame()
{
	{
		g_global_counters.current_submit_frame_idx++;
		memset(&g_debug_frame_counters, 0, sizeof(debug_frame_counters));
	}

	// TODO: spinlock
	detail::s_waiting_for_swap = true;
}

void end_frame()
{
	g_debug_global_counters.submitted_frames++;
	// wait for swap present here
	// spin spin spin spin
	while (detail::s_waiting_for_swap);
}

void begin_render_pass(const framebuffer_handle_t i_fb)
{
}

void begin_render_pass(const framebuffer_handle_t i_fb, const s32 i_x, const s32 i_y, const s32 i_width, const s32 i_height)
{
}

void end_render_pass(const framebuffer_handle_t i_fb)
{
	// nothing, LUL
}

void mark_present_render()
{
	present_render_command cmd;
	cmd.placeholder = 1;
	gpu_command newGPUCmd;
	newGPUCmd.opcode = command::present_render;
	newGPUCmd.deserialize(cmd);
	get_composing_command_buffer().push_back(newGPUCmd);
}

void dispatch_render_pass()
{
	{
		while ((detail::g_back_cmdbuff + 1) % BUFFERS_COUNT == detail::g_front_cmdbuff);
	}

	detail::g_back_cmdbuff = (detail::g_back_cmdbuff + 1) % BUFFERS_COUNT;
	{
		cleanup_shading_module();
		cleanup_buffers_module();
		cleanup_textures_module();
		cleanup_render_module();
	}

	{
		detail::g_cmdbuffer_condvar.notify_one();
	}
}

// ---------------------------------------------
void cleanup_render_module()
{
	get_composing_allocator()->free_all();
}

}
