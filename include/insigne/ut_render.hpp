#include "buffers.h"

namespace insigne {

// -----------------------------------------
template <typename t_surface>
void renderable_surface_t<t_surface>::render()
{
	PROFILE_SCOPE(render);

	for (u32 i = 0; i < detail::draw_command_buffer_t<t_surface>::command_buffer[detail::s_front_cmdbuff].get_size(); i++) {
		gpu_command& gpuCmd = detail::draw_command_buffer_t<t_surface>::command_buffer[detail::s_front_cmdbuff][i];
		gpuCmd.reset_cursor();
		switch (gpuCmd.opcode) {
			case command::draw_geom:
				{
					PROFILE_SCOPE(draw_geom);
					render_command cmd;
					gpuCmd.serialize(cmd);
					renderer::draw_surface_idx<t_surface>(cmd.surface_handle, *cmd.material_snapshot,
							cmd.segment_size, cmd.segment_offset);
					break;
				}
			case command::draw_command:
				{
					draw_command_t cmd;
					gpuCmd.serialize(cmd);
					renderer::draw_indexed_surface<t_surface>(cmd.vb_handle, cmd.ib_handle, cmd.material_snapshot,
							cmd.segment_size, cmd.segment_offset);
					break;
				}
			default:
				break;
		}
	}
}

template <typename t_surface>
void renderable_surface_t<t_surface>::init_buffer(insigne::linear_allocator_t* i_allocator)
{
	using namespace insigne;
	for (u32 i = 0; i < BUFFERED_FRAMES; i++)
		detail::draw_command_buffer_t<t_surface>::command_buffer[i].init(g_settings.draw_command_buffer_size, &g_persistance_allocator);
}

template <typename t_surface>
void push_draw_command(const draw_command_t& i_cmd)
{
	gpu_command newCmd;
	newCmd.opcode = command::draw_command;
	newCmd.deserialize(i_cmd);
	detail::draw_command_buffer_t<t_surface>::command_buffer[detail::s_back_cmdbuff].push_back(newCmd);

	g_debug_frame_counters.num_render_commands++;
}
// -----------------------------------------
template <typename t_surface>
void draw_surface(const vb_handle_t i_vb, const ib_handle_t i_ib, const material_desc_t& i_mat)
{
	material_desc_t* matSnapshot = detail::s_gpu_frame_allocator[detail::s_back_cmdbuff]->allocate<material_desc_t>();
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
