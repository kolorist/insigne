#include "insigne/detail/rt_textures.h"

namespace insigne {
namespace detail {

textures_pool_t									g_textures_pool;

// ---------------------------------------------
inline detail::gpu_command_buffer_t& get_textures_command_buffer(const size i_cmdBuffId) {
	return detail::g_textures_command_buffer[i_cmdBuffId];
}

// ---------------------------------------------
const texture_handle_t create_texture_slot(const insigne::texture_desc_t& i_desc)
{
	u32 idx = g_textures_pool.get_size();
	g_textures_pool.push_back(texture_desc_t());

	return texture_handle_t(idx);
}

// ---------------------------------------------
void upload_texture(texture_desc_t& io_desc, const insigne::texture_desc_t& i_uploadDesc)
{
	
}

// ---------------------------------------------
void process_textures_command_buffer(const size i_cmdBuffId)
{
	detail::gpu_command_buffer_t& cmdbuff = get_textures_command_buffer(i_cmdBuffId);
	for (u32 i = 0; i < cmdbuff.get_size(); i++) {
		gpu_command& gpuCmd = cmdbuff[i];
		gpuCmd.reset_cursor();

		switch (gpuCmd.opcode) {
			case command::textures_command:
				{
					textures_command_t cmd;
					gpuCmd.serialize(cmd);

					texture_desc_t& textureDesc = g_textures_pool[s32(cmd.texture_handle)];
					upload_texture(textureDesc, cmd.desc);
					break;
				}
			default:
				break;
		}
	}

}

}
}
