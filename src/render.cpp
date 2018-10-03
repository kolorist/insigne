#include "insigne/render.h"

#include <floral.h>
#include <clover.h>

#include "insigne/system.h"
#include "insigne/context.h"
#include "insigne/driver.h"
#include "insigne/detail/render_states.h"
#include "insigne/counters.h"
#include "insigne/ut_shading.h"
#include "insigne/ut_buffers.h"

namespace insigne {

// -----------------------------------------

#define s_composing_allocator					(*detail::s_gpu_frame_allocator[detail::s_back_cmdbuff])
#define s_rendering_allocator					(*detail::s_gpu_frame_allocator[detail::s_front_cmdbuff])

// -----------------------------------------
void push_command(const framebuffer_init_command& i_cmd)
{
	gpu_command newCmd;
	newCmd.opcode = command::init_framebuffer;
	newCmd.deserialize(i_cmd);
	detail::s_generic_command_buffer[detail::s_back_cmdbuff].push_back(newCmd);

	g_debug_frame_counters.num_init_commands++;
}

void push_command(const init_command& i_cmd)
{
	gpu_command newCmd;
	newCmd.opcode = command::setup_init_state;
	newCmd.deserialize(i_cmd);
	detail::s_generic_command_buffer[detail::s_back_cmdbuff].push_back(newCmd);

	g_debug_frame_counters.num_init_commands++;
}

void push_command(const load_command& i_cmd)
{
	gpu_command newCmd;
	newCmd.opcode = command::load_data;
	newCmd.deserialize(i_cmd);
	detail::s_generic_command_buffer[detail::s_back_cmdbuff].push_back(newCmd);

	g_debug_frame_counters.num_load_commands++;
}

void push_command(const stream_command& i_cmd)
{
	gpu_command newCmd;
	newCmd.opcode = command::stream_data;
	newCmd.deserialize(i_cmd);
	detail::s_generic_command_buffer[detail::s_back_cmdbuff].push_back(newCmd);

	g_debug_frame_counters.num_load_commands++;
}

void push_command(const render_state_toggle_command& i_cmd)
{
	gpu_command newCmd;
	newCmd.opcode = command::setup_render_state_toggle;
	newCmd.deserialize(i_cmd);
	detail::s_generic_command_buffer[detail::s_back_cmdbuff].push_back(newCmd);

	g_debug_frame_counters.num_state_commands++;
}

// -----------------------------------------

}
