#include "buffers.h"
#include "internal_states.h"

#include "commands.h"
#include "detail/rt_render.h"

namespace insigne {

// ---------------------------------------------
template <typename t_surface>
void push_draw_command(const draw_command_t& i_cmd)
{
	gpu_command newCmd;
	newCmd.opcode = command::draw_command;
	newCmd.deserialize(i_cmd);
	detail::draw_command_buffer_t<t_surface>::command_buffer[detail::g_composing_cmdbuff].push_back(newCmd);
}

// -----------------------------------------
template <typename t_surface>
void draw_surface(const vb_handle_t i_vb, const ib_handle_t i_ib, const material_desc_t& i_mat)
{
	material_desc_t* matSnapshot = detail::g_frame_draw_allocator[detail::g_composing_cmdbuff]->allocate<material_desc_t>();
	(*matSnapshot) = i_mat; // trigger copy assignment

	draw_command_t cmd;
	cmd.material_snapshot = matSnapshot;
	cmd.segment_offset = 0;
	cmd.vb_handle = i_vb;
	cmd.ib_handle = i_ib;
	cmd.segment_size = 0;

	push_draw_command<t_surface>(cmd);
}

}
