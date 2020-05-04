#include "insigne/ut_buffers.h"

#include "insigne/configs.h"
#include "insigne/commands.h"
#include "insigne/internal_states.h"
#include "insigne/detail/rt_buffers.h"

namespace insigne {

// ---------------------------------------------

struct buffers_resource_snapshot_t
{
	vb_handle_t									vb;
	ib_handle_t									ib;
	ub_handle_t									ub;
};

static floral::inplace_array<buffers_resource_snapshot_t, 8>	s_resource_snapshots;

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

const vb_handle_t copy_create_vb(const vbdesc_t& i_desc)
{
	vb_handle_t newVBO = detail::create_vb(i_desc);

	size dataSize = i_desc.count * i_desc.stride;
	voidptr data = get_composing_allocator()->allocate(dataSize);
	memcpy(data, i_desc.data, dataSize);

	buffers_command_t cmd;
	cmd.command_type = buffers_command_type_e::create_vb_buffers;
	cmd.create_vb_data.desc = i_desc;
	cmd.create_vb_data.desc.data = data;
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

const ib_handle_t copy_create_ib(const ibdesc_t& i_desc)
{
	ib_handle_t newIBO = detail::create_ib(i_desc);

	size dataSize = i_desc.count * sizeof(s32);
	voidptr data = get_composing_allocator()->allocate(dataSize);
	memcpy(data, i_desc.data, dataSize);

	buffers_command_t cmd;
	cmd.command_type = buffers_command_type_e::create_ib_buffers;
	cmd.create_ib_data.desc = i_desc;
	cmd.create_ib_data.desc.data = data;
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
	if (i_desc.alignment == 0)
	{
		cmd.create_ub_data.desc.alignment = g_gpu_capacities.ub_desired_offset;
	}
	cmd.create_ub_data.ub_handle = newUBO;

	push_command(cmd);

	return newUBO;
}

const ub_handle_t copy_create_ub(const ubdesc_t& i_desc)
{
	ub_handle_t newUBO = detail::create_ub(i_desc);

	voidptr data = get_composing_allocator()->allocate(i_desc.data_size);
	memcpy(data, i_desc.data, i_desc.data_size);

	buffers_command_t cmd;
	cmd.command_type = buffers_command_type_e::create_ub_buffers;
	cmd.create_ub_data.desc = i_desc;
	if (i_desc.alignment == 0)
	{
		cmd.create_ub_data.desc.alignment = g_gpu_capacities.ub_desired_offset;
	}
	cmd.create_ub_data.desc.data = data;
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

void update_ub(const ub_handle_t i_hdl, voidptr i_data, const size i_size, const size i_offset, const size i_align /* = 0 */)
{
	buffers_command_t cmd;
	cmd.command_type = buffers_command_type_e::stream_ub_data;
	cmd.stream_ub_data.ub_handle = i_hdl;
	cmd.stream_ub_data.data = i_data;
	cmd.stream_ub_data.data_size = i_size;
	cmd.stream_ub_data.offset = i_offset;
	if (i_align == 0)
	{
		cmd.stream_ub_data.alignment = g_gpu_capacities.ub_desired_offset;
	}
	else
	{
		cmd.stream_ub_data.alignment = i_align;
	}

	push_command(cmd);
}

void copy_update_ub(const ub_handle_t i_hdl, const voidptr i_data, const size i_size, const size i_offset, const size i_align /* = 0 */)
{
	voidptr data = get_composing_allocator()->allocate(i_size);
	memcpy(data, i_data, i_size);
	update_ub(i_hdl, data, i_size, i_offset, i_align);
}

void cleanup_buffers_module()
{
	get_composing_allocator()->free_all();
}

ssize get_buffers_resource_state()
{
	ssize stateId = s_resource_snapshots.get_size();
	buffers_resource_snapshot_t newSnapshot;
	newSnapshot.vb = detail::get_last_vb();
	newSnapshot.ib = detail::get_last_ib();
	newSnapshot.ub = detail::get_last_ub();
	s_resource_snapshots.push_back(newSnapshot);
	return stateId;
}

void cleanup_buffers_resource(const ssize i_stateId)
{
	FLORAL_ASSERT_MSG(i_stateId == s_resource_snapshots.get_size() - 1, "Clean up buffers module does not according to snapshot order");
	const buffers_resource_snapshot_t snapShot = s_resource_snapshots.pop_back();

	buffers_command_t cmd;
	cmd.command_type = buffers_command_type_e::clean_up_snapshot;
	cmd.clean_up_snapshot_data.downto_vb = snapShot.vb;
	cmd.clean_up_snapshot_data.downto_ib = snapShot.ib;
	cmd.clean_up_snapshot_data.downto_ub = snapShot.ub;

	push_command(cmd);
}

namespace helpers
{
	
size calculate_nearest_ub_offset(const size i_rawOffset)
{
	return (i_rawOffset / g_gpu_capacities.ub_desired_offset + 1) * g_gpu_capacities.ub_desired_offset;
}

void update_ub_array(const ub_handle_t i_hdl, voidptr i_data, const size i_stride, const size i_elemCount)
{
	const size gpuStride = (i_stride / g_gpu_capacities.ub_desired_offset + 1) * g_gpu_capacities.ub_desired_offset;
	const size gpuRegionSize = gpuStride * i_elemCount;

	voidptr data = get_composing_allocator()->allocate(gpuRegionSize);
	voidptr srcData = i_data;
	voidptr dstData = data;

	for (size i = 0; i < i_elemCount; i++)
	{
		memcpy(dstData, srcData, i_stride);
		srcData = (voidptr)((aptr)srcData + i_stride);
		dstData = (voidptr)((aptr)dstData + gpuStride);
	}

	update_ub(i_hdl, data, gpuRegionSize, 0, gpuStride);
}

}

}
