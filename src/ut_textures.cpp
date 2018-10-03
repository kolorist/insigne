#include "insigne/ut_textures.h"

#include <math.h>

#include "insigne/internal_states.h"
#include "insigne/commands.h"

namespace insigne {

// ---------------------------------------------
static inline arena_allocator_t* get_composing_allocator() {
	return detail::g_frame_textures_allocator[detail::g_back_cmdbuff];
}

static inline detail::gpu_command_buffer_t& get_composing_command_buffer() {
	return detail::g_textures_command_buffer[detail::g_back_cmdbuff];
}

// ---------------------------------------------
static inline void push_command(const textures_command_t& i_cmd)
{
	gpu_command newCmd;
	newCmd.opcode = command::textures_command;
	newCmd.deserialize(i_cmd);

	get_composing_command_buffer().push_back(newCmd);
}

// ---------------------------------------------

void prepare_texture_desc(texture_desc_t& io_desc)
{
	// bit-per-pixel
	static size s_bpp[] = {
		3 * 8, // rgb
		3 * 16, // hdr_rgb
		3 * 8, // srgb
		4 * 8, // rgba
		4 * 16, // hdr_rgba
		24, // depth
		32, // depth_stencil
	};
	size bytePerPixel = s_bpp[s32(io_desc.format)] / 8;

	size dataSize = 0;
	if (io_desc.has_mipmap) {
		FLORAL_ASSERT_MSG(io_desc.width == io_desc.height, "Texture with mipmap enable must have width == height");
		// TODO: check power of two
		s32 mipsCount = s32(log2(io_desc.width)) + 1;
		dataSize = ((1 << (2 * mipsCount)) - 1) / 3 * bytePerPixel;
	} else {
		dataSize = io_desc.width * io_desc.height * bytePerPixel;
	}

	if (io_desc.dimension == texture_dimension_e::tex_cube)
		dataSize *= 6;

	io_desc.data = get_composing_allocator()->allocate(dataSize);
}

// ---------------------------------------------
void cleanup_textures_module()
{
}

}
