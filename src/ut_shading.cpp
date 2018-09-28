#include "insigne/ut_shading.h"

#include "insigne/commands.h"
#include "insigne/detail/render_states.h"
#include "insigne/detail/rt_shading.h"

namespace insigne {

// ---------------------------------------------
inline arena_allocator_t& get_composing_allocator() {
	return (*detail::g_frame_shader_allocator[detail::s_back_cmdbuff]);
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

	get_composing_command_buffer().push_back(newCmd);
}

// ---------------------------------------------

shader_desc_t create_shader_desc()
{
	shader_desc_t newDesc;

	// TODO: hardcode!!!
	newDesc.vs = (cstr)get_composing_allocator().allocate(SIZE_KB(128));
	newDesc.fs = (cstr)get_composing_allocator().allocate(SIZE_KB(128));

	return newDesc;
}

const shader_handle_t create_shader(const shader_desc_t& i_desc)
{
	shader_handle_t newShaderHdl = detail::create_shader(i_desc);

	shading_command_t cmd;
	cmd.command_type = shading_command_type_e::shader_compile;
	cmd.shader_handle = newShaderHdl;

	push_command(cmd);

	return newShaderHdl;
}

void cleanup_shading_module()
{
	get_composing_allocator().free_all();
}

}
