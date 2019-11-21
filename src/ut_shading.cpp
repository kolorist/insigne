#include "insigne/ut_shading.h"

#include <clover.h>

#include "insigne/commands.h"
#include "insigne/buffers.h"
#include "insigne/internal_states.h"
#include "insigne/detail/rt_shading.h"

namespace insigne {

// ---------------------------------------------

struct shading_resource_snapshot_t
{
	shader_handle_t								handle;
};

static floral::inplace_array<shading_resource_snapshot_t, 8>	s_resource_snapshots;

// ---------------------------------------------
static inline arena_allocator_t* get_composing_allocator() {
	return detail::g_frame_shader_allocator[detail::g_composing_cmdbuff];
}

static inline detail::gpu_command_buffer_t& get_composing_command_buffer() {
	return detail::g_shading_command_buffer[detail::g_composing_cmdbuff];
}
// ---------------------------------------------

static inline void push_command(const shading_command_t& i_cmd)
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
	newDesc.vs = (cstr)get_composing_allocator()->allocate(SIZE_KB(128));
	newDesc.fs = (cstr)get_composing_allocator()->allocate(SIZE_KB(128));
	newDesc.reflection.textures = get_composing_allocator()->allocate<shader_param_list_t>(MAX_TEXTURE_SAMPLERS, get_composing_allocator());
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
	cmd.shader_compile_data.vs = i_desc.vs;
	cmd.shader_compile_data.fs = i_desc.fs;
	cmd.shader_compile_data.reflection = i_desc.reflection;
	cmd.shader_compile_data.shader_handle = newShaderHdl;

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

ssize get_shading_resource_state()
{
	ssize stateId = s_resource_snapshots.get_size();
	shading_resource_snapshot_t newSnapshot;
	newSnapshot.handle = detail::get_last_shader();
	s_resource_snapshots.push_back(newSnapshot);
	return stateId;
}

void cleanup_shading_resource(const ssize i_stateId)
{
	FLORAL_ASSERT_MSG(i_stateId == s_resource_snapshots.get_size() - 1, "Clean up shading module does not according to snapshot order");
	const shading_resource_snapshot_t snapShot = s_resource_snapshots.pop_back();

	shading_command_t cmd;
	cmd.command_type = shading_command_type_e::clean_up_snapshot;
	cmd.clean_up_snapshot_data.downto_handle = snapShot.handle;

	push_command(cmd);
}

namespace helpers
{

void assign_uniform_block(material_desc_t& io_mat, const_cstr i_id, const size i_offset, const size i_range, const ub_handle_t i_ub)
{
	ssize slot = get_material_uniform_block_slot(io_mat, i_id);
	io_mat.uniform_blocks[slot].value = ubmat_desc_t { i_offset, i_range, i_ub };
}

void assign_texture(material_desc_t& io_mat, const_cstr i_id, const texture_handle_t i_tex)
{
	ssize slot = get_material_texture_slot(io_mat, i_id);
	io_mat.textures[slot].value = i_tex;
}

}

}
