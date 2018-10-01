#include "insigne/ut_buffers.h"

#include "insigne/commands.h"
#include "insigne/detail/render_states.h"
#include "insigne/detail/rt_buffers.h"

namespace insigne {

// ---------------------------------------------
inline arena_allocator_t* get_composing_allocator() {
	return detail::g_frame_buffers_allocator[detail::s_back_cmdbuff];
}

inline detail::gpu_command_buffer_t& get_composing_command_buffer() {
	return detail::g_buffers_command_buffer[detail::s_back_cmdbuff];
}
// ---------------------------------------------

inline void push_command(const buffers_command_t& i_cmd)
{
	gpu_command newCmd;
	newCmd.opcode = command::buffers_command;
	newCmd.deserialize(i_cmd);
	get_composing_command_buffer().push_back(newCmd);
}
// ---------------------------------------------

const vb_handle_t create_vb(const vbdesc_t& i_desc)
{
	vb_handle_t newVBO = detail::create_vb(i_desc);

	buffers_command_t cmd;
	cmd.command_type = buffers_command_type_e::create_vb_buffers;
	cmd.create_vb_data.desc = i_desc;
	cmd.create_vb_data.vb_handle = newVBO;

	push_command(cmd);

	return newVBO;
}

const ib_handle_t create_ib(const ibdesc_t& i_desc)
{
	ib_handle_t newIBO = detail::create_ib(i_desc);

	buffers_command_t cmd;
	cmd.command_type = buffers_command_type_e::create_ib_buffers;
	cmd.create_ib_data.desc = i_desc;
	cmd.create_ib_data.ib_handle = newIBO;

	push_command(cmd);

	return newIBO;
}

void update_vb(const vb_handle_t i_hdl, voidptr i_data, const u32 i_vcount, const u32 i_offsetElem)
{
	buffers_command_t cmd;
	cmd.command_type = buffers_command_type_e::stream_vb_data;
	cmd.stream_vb_data.vb_handle = i_hdl;
	cmd.stream_vb_data.data = i_data;
	cmd.stream_vb_data.vcount = i_vcount;
	cmd.stream_vb_data.offset_elements = i_offsetElem;

	push_command(cmd);
}

void update_ib(const ib_handle_t i_hdl, voidptr i_data, const u32 i_icount, const u32 i_offsetElem)
{
	buffers_command_t cmd;
	cmd.command_type = buffers_command_type_e::stream_ib_data;
	cmd.stream_ib_data.ib_handle = i_hdl;
	cmd.stream_ib_data.data = i_data;
	cmd.stream_ib_data.icount = i_icount;
	cmd.stream_ib_data.offset_elements = i_offsetElem;

	push_command(cmd);
}

void cleanup_buffers_module()
{
	get_composing_allocator()->free_all();
}

}
