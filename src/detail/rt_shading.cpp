#include "insigne/detail/rt_shading.h"

#include "insigne/detail/render_states.h"

namespace insigne {
namespace detail {

shaders_pool_t									g_shaders_pool;

// ---------------------------------------------
inline detail::gpu_command_buffer_t& get_rendering_command_buffer() {
	return detail::g_shading_command_buffer[detail::s_front_cmdbuff];
}
// ---------------------------------------------

const shader_handle_t create_shader(const insigne::shader_desc_t& i_desc)
{
	u32 idx = g_shaders_pool.get_size();
	g_shaders_pool.push_back(shader_desc_t());

	shader_desc_t& desc = g_shaders_pool[idx];
	desc.vs_path = i_desc.vs_path;
	desc.fs_path = i_desc.fs_path;
	desc.vs = i_desc.vs;
	desc.fs = i_desc.fs;

	return shader_handle_t(idx);
}

void process_shading_command_buffer()
{
	detail::gpu_command_buffer_t& cmdbuff = get_rendering_command_buffer();
	for (u32 i = 0; i < cmdbuff.get_size(); i++) {
		gpu_command& gpuCmd = cmdbuff[i];
		gpuCmd.reset_cursor();

		switch (gpuCmd.opcode) {
			case command::shading_command:
				{
					shading_command_t cmd;
					gpuCmd.serialize(cmd);
					shader_desc_t& desc = g_shaders_pool[cmd.shader_handle];

					// compile!!!

					break;
				}

			default:
				break;
		}
	}

	cmdbuff.empty();
}

}
}
