#include "buffers.h"
#include "internal_states.h"

namespace insigne {

// -----------------------------------------
template <typename t_surface>
void renderable_surface_t<t_surface>::render()
{
	PROFILE_SCOPE(render);

	for (u32 i = 0; i < detail::draw_command_buffer_t<t_surface>::command_buffer[detail::g_front_cmdbuff].get_size(); i++) {
		gpu_command& gpuCmd = detail::draw_command_buffer_t<t_surface>::command_buffer[detail::g_front_cmdbuff][i];
		gpuCmd.reset_cursor();
		/*
		switch (gpuCmd.opcode) {
			default:
				break;
		}
		*/
	}
}

template <typename t_surface>
void renderable_surface_t<t_surface>::init_buffer(insigne::linear_allocator_t* i_allocator)
{
	using namespace insigne;
	for (u32 i = 0; i < BUFFERS_COUNT; i++)
		detail::draw_command_buffer_t<t_surface>::command_buffer[i].init(g_settings.draw_command_buffer_size, &g_persistance_allocator);
}

template <typename t_surface>
void push_draw_command(const draw_command_t& i_cmd)
{
	gpu_command newCmd;
	newCmd.opcode = command::draw_command;
	newCmd.deserialize(i_cmd);
	detail::draw_command_buffer_t<t_surface>::command_buffer[detail::g_back_cmdbuff].push_back(newCmd);
}
// -----------------------------------------
template <typename t_surface>
void draw_surface(const vb_handle_t i_vb, const ib_handle_t i_ib, const material_desc_t& i_mat)
{
	material_desc_t* matSnapshot = detail::g_frame_draw_allocator[detail::g_back_cmdbuff]->allocate<material_desc_t>();
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
