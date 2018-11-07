#include "configs.h"
#include "internal_states.h"

namespace insigne {

template <typename t_surface>
void register_surface_type()
{
	u32 idx = detail::g_draw_command_buffers.get_size();
	detail::g_draw_command_buffers.push_back(detail::cmdbuffs_t());
	detail::cmdbuffs_t& cmdbuff = detail::g_draw_command_buffers[idx];
	for (u32 i = 0; i < BUFFERS_COUNT; i++) {
		cmdbuff.command_buffer[i].init(g_settings.draw_command_buffer_size, &g_persistance_allocator);
	}
	cmdbuff.states_setup_func = &t_surface::setup_states;
	cmdbuff.vertex_data_setup_func = &t_surface::describe_vertex_data;
	cmdbuff.geometry_mode = t_surface::geometry_mode;

	g_settings.surface_types_count++;
	t_surface::index = idx;
}

}
