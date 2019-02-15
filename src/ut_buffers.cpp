#include "insigne/ut_buffers.h"

#include "insigne/commands.h"
#include "insigne/internal_states.h"
#include "insigne/detail/rt_buffers.h"

namespace insigne {

// ---------------------------------------------
static inline arena_allocator_t* get_composing_allocator() {
	return detail::g_frame_buffers_allocator[detail::g_composing_cmdbuff];
}

static inline detail::gpu_command_buffer_t& get_composing_command_buffer() {
	return detail::g_buffers_command_buffer[detail::g_composing_cmdbuff];
}
// ---------------------------------------------

static inline void push_command(const buffers_command_t& i_cmd)
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

const ub_handle_t create_ub(const ubdesc_t& i_desc)
{
	ub_handle_t newUBO = detail::create_ub(i_desc);

	buffers_command_t cmd;
	cmd.command_type = buffers_command_type_e::create_ub_buffers;
	cmd.create_ub_data.desc = i_desc;
	cmd.create_ub_data.ub_handle = newUBO;

	push_command(cmd);

	return newUBO;
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

void copy_update_vb(const vb_handle_t i_hdl, voidptr i_data, const u32 i_vcount, const u32 i_stride, const u32 i_offsetElem)
{
	size dataSize = i_vcount * i_stride;
	voidptr data = get_composing_allocator()->allocate(dataSize);
	memcpy(data, i_data, dataSize);
	update_vb(i_hdl, data, i_vcount, i_offsetElem);
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

void copy_update_ib(const ib_handle_t i_hdl, voidptr i_data, const u32 i_icount, const u32 i_offsetElem)
{
	size dataSize = i_icount * sizeof(s32);
	voidptr data = get_composing_allocator()->allocate(dataSize);
	memcpy(data, i_data, dataSize);
	update_ib(i_hdl, data, i_icount, i_offsetElem);
}

void update_ub(const ub_handle_t i_hdl, voidptr i_data, const size i_size, const size i_offset)
{
	buffers_command_t cmd;
	cmd.command_type = buffers_command_type_e::stream_ub_data;
	cmd.stream_ub_data.ub_handle = i_hdl;
	cmd.stream_ub_data.data = i_data;
	cmd.stream_ub_data.data_size = i_size;
	cmd.stream_ub_data.offset = i_offset;

	push_command(cmd);
}

void copy_update_ub(const ub_handle_t i_hdl, voidptr i_data, const size i_size, const size i_offset)
{
	voidptr data = get_composing_allocator()->allocate(i_size);
	memcpy(data, i_data, i_size);
	update_ub(i_hdl, data, i_size, i_offset);
}

void cleanup_buffers_module()
{
	get_composing_allocator()->free_all();
}

}
