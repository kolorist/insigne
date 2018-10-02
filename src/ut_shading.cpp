#include "insigne/ut_shading.h"

#include <clover.h>

#include "insigne/commands.h"
#include "insigne/detail/render_states.h"
#include "insigne/detail/rt_shading.h"

namespace insigne {

// ---------------------------------------------
inline arena_allocator_t* get_composing_allocator() {
	return detail::g_frame_shader_allocator[detail::s_back_cmdbuff];
}

inline detail::gpu_command_buffer_t& get_composing_command_buffer() {
	return detail::g_shading_command_buffer[detail::s_back_cmdbuff];
}
// ---------------------------------------------

inline void push_command(const shading_command_t& i_cmd)
{
	gpu_command newCmd;
	newCmd.opcode = command::shading_command;
	newCmd.deserialize(i_cmd);

	//get_composing_command_buffer().push_back(newCmd);
	detail::g_shading_command_buffer[detail::s_back_cmdbuff].push_back(newCmd);
}

// ---------------------------------------------

shader_desc_t create_shader_desc()
{
	shader_desc_t newDesc;

	// TODO: hardcode!!!
	newDesc.vs = (cstr)get_composing_allocator()->allocate(SIZE_KB(128));
	newDesc.fs = (cstr)get_composing_allocator()->allocate(SIZE_KB(128));
	newDesc.reflection.textures = get_composing_allocator()->allocate<shader_param_list_t>(MAX_TEXCUBE_UNIFORMS, get_composing_allocator());
	newDesc.reflection.uniform_blocks = get_composing_allocator()->allocate<shader_param_list_t>(MAX_UNIFORM_BLOCKS, get_composing_allocator());

	return newDesc;
}

const shader_handle_t create_shader(const shader_desc_t& i_desc)
{
	LOG_TOPIC("ut_shading");
	shader_handle_t newShaderHdl = detail::create_shader(i_desc);
	CLOVER_DEBUG("New shader will be created with handle: %d", (s32)newShaderHdl);
	CLOVER_DEBUG("\tVS Path: %s", i_desc.vs_path.pm_PathStr);
	CLOVER_DEBUG("\tFS Path: %s", i_desc.fs_path.pm_PathStr);

	shading_command_t cmd;
	cmd.command_type = shading_command_type_e::shader_compile;
	cmd.vs = i_desc.vs;
	cmd.fs = i_desc.fs;
	cmd.reflection = i_desc.reflection;
	cmd.shader_handle = newShaderHdl;

	push_command(cmd);

	return newShaderHdl;
}

void infuse_material(const shader_handle_t i_hdl, material_desc_t& o_mat)
{
	detail::infuse_material(i_hdl, o_mat);
}

void cleanup_shading_module()
{
	get_composing_allocator()->free_all();
}

}
