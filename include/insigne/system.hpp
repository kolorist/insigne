#include "configs.h"
#include "internal_states.h"

namespace insigne
{

template <typename t_surface>
void register_surface_type()
{
	floral::lock_guard guard(detail::g_draw_config_mtx);

	ssize idx = detail::g_draw_command_buffers.get_size();
	detail::g_draw_command_buffers.push_back(detail::cmdbuffs_t());
	detail::cmdbuffs_t& cmdbuff = detail::g_draw_command_buffers[idx];
	for (ssize i = 0; i < BUFFERS_COUNT; i++)
	{
		cmdbuff.command_buffer[i].reserve(t_surface::draw_calls_budget, detail::g_draw_cmdbuff_arena);
	}
	cmdbuff.states_setup_func = &t_surface::setup_states;
	cmdbuff.vertex_data_setup_func = &t_surface::describe_vertex_data;
	cmdbuff.geometry_mode = t_surface::geometry_mode;

	g_settings.surface_types_count++;
	t_surface::index = idx;
}

template <typename t_surface>
void unregister_surface_type()
{
	floral::lock_guard guard(detail::g_draw_config_mtx);
	using namespace detail;
	
	ssize cmdBuffSize = g_draw_command_buffers.get_size();
	ssize toRemoveCmdBuffIdx = t_surface::index;

	FLORAL_ASSERT(cmdBuffSize - 1 == toRemoveCmdBuffIdx);
	if (toRemoveCmdBuffIdx == cmdBuffSize - 1)
	{
		cmdbuffs_t& cmdbuff = g_draw_command_buffers[toRemoveCmdBuffIdx];
		for (ssize i = BUFFERS_COUNT - 1; i >= 0; i--)
		{
			cmdbuff.command_buffer[i].~gpu_command_buffer_t();
		}
		cmdbuff.states_setup_func = nullptr;
		cmdbuff.vertex_data_setup_func = nullptr;
		cmdbuff.geometry_mode = geometry_mode_e::points;

		g_draw_command_buffers.resize(cmdBuffSize - 1);
		g_settings.surface_types_count--;
	}
}

template <typename t_surface>
void register_post_surface_type()
{
	floral::lock_guard guard(detail::g_post_draw_config_mtx);

	ssize idx = detail::g_post_draw_command_buffers.get_size();
	detail::g_post_draw_command_buffers.push_back(detail::cmdbuffs_t());
	detail::cmdbuffs_t& cmdbuff = detail::g_post_draw_command_buffers[idx];
	for (ssize i = 0; i < BUFFERS_COUNT; i++)
	{
		cmdbuff.command_buffer[i].reserve(t_surface::draw_calls_budget, detail::g_post_draw_cmdbuff_arena);
	}
	cmdbuff.states_setup_func = &t_surface::setup_states;
	cmdbuff.vertex_data_setup_func = &t_surface::describe_vertex_data;
	cmdbuff.geometry_mode = t_surface::geometry_mode;

	g_settings.post_surface_types_count++;
	t_surface::index = idx | 0x8000;
}

template <typename t_surface>
void unregister_post_surface_type()
{
	floral::lock_guard guard(detail::g_draw_config_mtx);
	using namespace detail;
	
	ssize cmdBuffSize = g_post_draw_command_buffers.get_size();
	ssize toRemoveCmdBuffIdx = t_surface::index & 0x7FFF;

	FLORAL_ASSERT(cmdBuffSize - 1 == toRemoveCmdBuffIdx);
	if (toRemoveCmdBuffIdx == cmdBuffSize - 1)
	{
		cmdbuffs_t& cmdbuff = g_post_draw_command_buffers[toRemoveCmdBuffIdx];
		for (ssize i = BUFFERS_COUNT - 1; i >= 0; i--)
		{
			cmdbuff.command_buffer[i].~gpu_command_buffer_t();
		}
		cmdbuff.states_setup_func = nullptr;
		cmdbuff.vertex_data_setup_func = nullptr;
		cmdbuff.geometry_mode = geometry_mode_e::points;

		g_post_draw_command_buffers.resize(cmdBuffSize - 1);
		g_settings.post_surface_types_count--;
	}
}

}
