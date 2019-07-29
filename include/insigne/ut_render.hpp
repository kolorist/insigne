#include "buffers.h"
#include "internal_states.h"

#include "detail/rt_render.h"

namespace insigne
{

// -----------------------------------------
template <typename t_surface>
void draw_surface(const vb_handle_t i_vb, const ib_handle_t i_ib, const material_desc_t& i_mat)
{
	material_desc_t* matSnapshot = detail::g_frame_draw_allocator[detail::g_composing_cmdbuff]->allocate<material_desc_t>();
	(*matSnapshot) = i_mat; // trigger copy assignment

	draw_command_t cmd;
	cmd.command_type = draw_command_type_e::draw_surface;

	cmd.draw_surface_data.material_snapshot = matSnapshot;
	cmd.draw_surface_data.segment_offset = 0;
	cmd.draw_surface_data.vb_handle = i_vb;
	cmd.draw_surface_data.ib_handle = i_ib;
	cmd.draw_surface_data.segment_size = 0;

	push_draw_command(t_surface::index, cmd);
}

template <typename t_surface>
void draw_surface(const vb_handle_t i_vb, const ib_handle_t i_ib, const material_desc_t& i_mat,
		const s32 i_idxOffset, const s32 i_elemCount)
{
	material_desc_t* matSnapshot = detail::g_frame_draw_allocator[detail::g_composing_cmdbuff]->allocate<material_desc_t>();
	(*matSnapshot) = i_mat; // trigger copy assignment

	draw_command_t cmd;
	cmd.command_type = draw_command_type_e::draw_surface;

	cmd.draw_surface_data.material_snapshot = matSnapshot;
	cmd.draw_surface_data.segment_offset = i_idxOffset;
	cmd.draw_surface_data.vb_handle = i_vb;
	cmd.draw_surface_data.ib_handle = i_ib;
	cmd.draw_surface_data.segment_size = i_elemCount;

	push_draw_command(t_surface::index, cmd);
}

template <typename t_surface>
void setup_scissor(const bool i_enabled, const s32 i_x, const s32 i_y, const s32 i_width, const s32 i_height)
{
	draw_command_t cmd;
	cmd.command_type = draw_command_type_e::state_setup_scissor;

	cmd.state_setup_scissor_data.enabled = i_enabled;
	cmd.state_setup_scissor_data.x = i_x;
	cmd.state_setup_scissor_data.y = i_y;
	cmd.state_setup_scissor_data.width = i_width;
	cmd.state_setup_scissor_data.height = i_height;

	push_draw_command(t_surface::index, cmd);
}

}
