#include "insigne/ut_textures.h"

#include <math.h>

#include "insigne/internal_states.h"
#include "insigne/commands.h"
#include "insigne/detail/rt_textures.h"

namespace insigne {

// ---------------------------------------------

struct textures_resource_snapshot_t
{
	texture_handle_t							handle;
};

static floral::inplace_array<textures_resource_snapshot_t, 8>	s_resource_snapshots;

// ---------------------------------------------
static inline arena_allocator_t* get_composing_allocator() {
	return detail::g_frame_textures_allocator[detail::g_composing_cmdbuff];
}

static inline detail::gpu_command_buffer_t& get_composing_command_buffer() {
	return detail::g_textures_command_buffer[detail::g_composing_cmdbuff];
}

// ---------------------------------------------
static inline void push_command(const textures_command_t& i_cmd)
{
	gpu_command newCmd;
	newCmd.opcode = command::textures_command;
	newCmd.deserialize(i_cmd);

	get_composing_command_buffer().push_back(newCmd);
}

// -------------------------------------------------------------------

const size calculate_texture_memsize(const texture_desc_t& i_desc)
{
	// bit-per-pixel, CPU side
	static size s_bpp[] = {
		2 * 8,									// rg
		2 * 16,									// hdr_rg
		3 * 8,									// rgb
		3 * 16,									// hdr_rgb
		3 * 32,									// hdr_rgb_high
		3 * 16,									// hdr_rgb_half
		3 * 8,									// srgb
		4 * 8,									// srgba
		4 * 8,									// rgba
		4 * 16,									// hdr_rgba
		1 * 32,									// depth
		1 * 32									// depth_stencil
	};
	size bytePerPixel = s_bpp[s32(i_desc.format)] / 8;

	size dataSize = 0;
	if (i_desc.has_mipmap)
	{
		FLORAL_ASSERT_MSG(i_desc.width == i_desc.height, "Texture with mipmap enable must have width == height");
		// TODO: check power of two
		s32 mipsCount = s32(log2(i_desc.width)) + 1;
		dataSize = ((1 << (2 * mipsCount)) - 1) / 3 * bytePerPixel;
	}
	else
	{
		dataSize = i_desc.width * i_desc.height * bytePerPixel;
	}

	if (i_desc.dimension == texture_dimension_e::tex_cube)
	{
		dataSize *= 6;
	}

	return dataSize;
}

const size prepare_texture_desc(texture_desc_t& io_desc)
{
	size dataSize = calculate_texture_memsize(io_desc);
	io_desc.data = get_composing_allocator()->allocate(dataSize);
	return dataSize;
}

const texture_handle_t create_texture(const texture_desc_t& i_desc)
{
	texture_handle_t newTextureHdl = detail::create_texture();

	textures_command_t cmd;
	cmd.command_type = textures_command_type_e::create_texture;
	cmd.create_texture_data.texture_handle = newTextureHdl;
	cmd.create_texture_data.desc = i_desc;

	push_command(cmd);

	return newTextureHdl;
}

void update_texture(const texture_handle_t i_hdl, const texture_desc_t& i_desc)
{
	textures_command_t cmd;
	cmd.command_type = textures_command_type_e::stream_texture;
	cmd.stream_texture_data.texture_handle = i_hdl;
	cmd.stream_texture_data.data = i_desc.data;

	push_command(cmd);
}

void copy_update_texture(const texture_handle_t i_hdl, voidptr i_data, const size i_dataSize)
{
	voidptr data = get_composing_allocator()->allocate(i_dataSize);
	memcpy(data, i_data, i_dataSize);

	textures_command_t cmd;
	cmd.command_type = textures_command_type_e::stream_texture;
	cmd.stream_texture_data.texture_handle = i_hdl;
	cmd.stream_texture_data.data = data;

	push_command(cmd);
}

// ---------------------------------------------
void cleanup_textures_module()
{
	get_composing_allocator()->free_all();
}

ssize get_textures_resource_state()
{
	ssize stateId = s_resource_snapshots.get_size();
	textures_resource_snapshot_t newSnapshot;
	newSnapshot.handle = detail::get_last_texture();
	s_resource_snapshots.push_back(newSnapshot);
	return stateId;
}

void cleanup_textures_resource(const ssize i_stateId)
{
	FLORAL_ASSERT_MSG(i_stateId == s_resource_snapshots.get_size() - 1, "Clean up textures module does not according to snapshot order");
	const textures_resource_snapshot_t snapShot = s_resource_snapshots.pop_back();

	textures_command_t cmd;
	cmd.command_type = textures_command_type_e::clean_up_snapshot;
	cmd.clean_up_snapshot_data.downto_handle = snapShot.handle;

	push_command(cmd);
}

}
