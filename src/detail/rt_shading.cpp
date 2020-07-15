#include "insigne/detail/rt_shading.h"

#include "insigne/memory.h"
#include "insigne/counters.h"
#include "insigne/generated_code/proxy.h"
#include "insigne/internal_states.h"

#include <clover/SinkTopic.h>
#include <clover/Logger.h>

#include <lotus/profiler.h>

namespace insigne
{
namespace detail
{

shaders_pool_t									g_shaders_pool;

// ---------------------------------------------
inline detail::gpu_command_buffer_t& get_shading_command_buffer(const size i_cmdBuffId) {
	return detail::g_shading_command_buffer[i_cmdBuffId];
}
// ---------------------------------------------
/* ut */
const shader_handle_t create_shader(const insigne::shader_desc_t& i_desc)
{
	size idx = g_shaders_pool.get_size();
	g_shaders_pool.push_back(shader_desc_t());

	shader_desc_t& desc = g_shaders_pool[idx];
	desc.vs_path = i_desc.vs_path;
	desc.fs_path = i_desc.fs_path;
	desc.gpu_handle = 0;

	// TODO: crash because the inplace_array constructor is not triggered

	// we have to create material_template here because user may infuse a material
	// right after calling create_shader()
	for (ssize i = 0; i < i_desc.reflection.textures->get_size(); i++) {
		const_cstr pName = i_desc.reflection.textures->at(i).name;
		desc.material_template.textures.push_back(floral::crc_string(pName));
	}

	for (ssize i = 0; i < i_desc.reflection.uniform_blocks->get_size(); i++) {
		const_cstr pName = i_desc.reflection.uniform_blocks->at(i).name;
		desc.material_template.uniform_blocks.push_back(floral::crc_string(pName));
	}

	return shader_handle_t(idx);
}

/* ut */
const shader_handle_t get_last_shader()
{
	return g_shaders_pool.get_size() - 1;
}

void compile_shader(shader_desc_t& io_desc, const_cstr i_vs, const_cstr i_fs, const shader_reflection_t& i_refl)
{
	LOG_TOPIC("rt_shading");
	CLOVER_DEBUG("Compiling shader");
	CLOVER_DEBUG("\tVertex Shader Path: %s", io_desc.vs_path.pm_PathStr);
	CLOVER_DEBUG("\tFragment Shader Path: %s", io_desc.fs_path.pm_PathStr);

	GLuint newShader = 0;
	GLuint fs = 0, vs = 0;
	newShader = pxCreateProgram();
	fs = pxCreateShader(GL_FRAGMENT_SHADER);
	vs = pxCreateShader(GL_VERTEX_SHADER);

	// compile vertex shader
	{
		GLint result = GL_FALSE;
		s32 infoLength;
		pxShaderSource(vs, 1, (const GLchar**)&i_vs, nullptr);
		pxCompileShader(vs);
		pxGetShaderiv(vs, GL_COMPILE_STATUS, &result);
		if (!result) {
			pxGetShaderiv(vs, GL_INFO_LOG_LENGTH, &infoLength);
			cstr shaderErrorMsg = g_arena_allocator.allocate_array<c8>(infoLength);
			pxGetShaderInfoLog(vs, infoLength + 1, NULL, shaderErrorMsg);
			CLOVER_ERROR("Vertex Shader error:\n%s", shaderErrorMsg);
			pxDeleteShader(vs);
			pxDeleteShader(fs);
			return;
		}
	}

	// compile fragment shader
	{
		GLint result = GL_FALSE;
		int infoLength;
		pxShaderSource(fs, 1, (const GLchar**)&i_fs, nullptr);
		pxCompileShader(fs);
		pxGetShaderiv(fs, GL_COMPILE_STATUS, &result);
		if (!result) {
			pxGetShaderiv(fs, GL_INFO_LOG_LENGTH, &infoLength);
			cstr shaderErrorMsg = g_arena_allocator.allocate_array<c8>(infoLength);
			pxGetShaderInfoLog(fs, infoLength + 1, NULL, shaderErrorMsg);
			CLOVER_ERROR("Fragment Shader error:\n%s", shaderErrorMsg);
			pxDeleteShader(vs);
			pxDeleteShader(fs);
			return;
		}
	}

	g_arena_allocator.free_all();

	// link them all!
	pxAttachShader(newShader, vs);
	pxAttachShader(newShader, fs);
	pxLinkProgram(newShader);
	
	{
		GLint result = GL_FALSE;
		int infoLength;
		pxGetProgramiv(newShader, GL_LINK_STATUS, &result);
		if (!result)
		{
			pxGetProgramiv(newShader, GL_INFO_LOG_LENGTH, &infoLength);
			cstr shaderErrorMsg = g_arena_allocator.allocate_array<c8>(infoLength);
			pxGetProgramInfoLog(newShader, infoLength + 1, NULL, shaderErrorMsg);
			CLOVER_ERROR("Shader Linking error:\n%s", shaderErrorMsg);
			return;
		}
	}

	// reflection to template
	{
		for (u32 i = 0; i < i_refl.textures->get_size(); i++) {
			const_cstr pName = i_refl.textures->at(i).name;
			GLint id = pxGetUniformLocation(newShader, pName);
			if (id >= 0) {
				// nothing for now
			} else {
				// TODO: replace with assertion
				CLOVER_ERROR("Cannot find texture uniform '%s'", pName);
			}
			io_desc.slots_config.textures.push_back(id);
		}

		for (u32 i = 0; i < i_refl.uniform_blocks->get_size(); i++) {
			const_cstr pName = i_refl.uniform_blocks->at(i).name;
			GLuint id = pxGetUniformBlockIndex(newShader, pName);
			if (id != GL_INVALID_INDEX) {
				// nothing for now
			} else {
				// TODO: replace with assertion
				CLOVER_ERROR("Cannot find uniform block '%s'", pName);
			}
			io_desc.slots_config.uniform_blocks.push_back(id);
		}
	}

	io_desc.gpu_handle = newShader;

	// release temp shaders
	pxDeleteShader(vs);
	pxDeleteShader(fs);

}

/* ut */
void infuse_material(const shader_handle_t i_hdl, insigne::material_desc_t& o_mat)
{
	const shader_desc_t& shaderDesc = g_shaders_pool[s32(i_hdl)];
	const material_template_t& matTemplate = shaderDesc.material_template;
	// TODO: validate shader

	o_mat.shader_handle = i_hdl; // potential LHS?
	o_mat.textures.empty();
	o_mat.uniform_blocks.empty();
	for (u32 i = 0; i < matTemplate.textures.get_size(); i++) {
		insigne::name_value_pair_t<texture_handle_t> nvp;
		nvp.name = matTemplate.textures[i];
		nvp.value = texture_handle_t(-1);
		o_mat.textures.push_back(nvp);
	}

	for (u32 i = 0; i < matTemplate.uniform_blocks.get_size(); i++) {
		insigne::name_value_pair_t<ubmat_desc_t> nvp;
		nvp.name = matTemplate.uniform_blocks[i];
		nvp.value = ubmat_desc_t { 0, 0, -1 };
		o_mat.uniform_blocks.push_back(nvp);
	}
}

static void clean_up_snapshot(const shader_handle_t i_shaderHandle)
{
	CLOVER_VERBOSE("Cleaning up shading snapshot...");
	while (i_shaderHandle != g_shaders_pool.get_size() - 1)
	{
		shader_desc_t shaderDesc = g_shaders_pool.pop_back();
		CLOVER_VERBOSE("Deleting shader id %d: '%s' - '%s'",
				shaderDesc.gpu_handle,
				shaderDesc.vs_path.pm_PathStr, shaderDesc.fs_path.pm_PathStr);
		pxDeleteProgram(shaderDesc.gpu_handle);
	}
	CLOVER_VERBOSE("Finished cleaning up shading snapshot");
}

// ---------------------------------------------
void cleanup_shading_module()
{
	CLOVER_VERBOSE("Cleaning up shading module...");
	for (ssize i = 0; i < g_shaders_pool.get_size(); i++)
	{
		shader_desc_t& shaderDesc = g_shaders_pool[i];
		CLOVER_VERBOSE("Deleting shader id %d: '%s' - '%s'",
				shaderDesc.gpu_handle,
				shaderDesc.vs_path.pm_PathStr, shaderDesc.fs_path.pm_PathStr);
		pxDeleteProgram(shaderDesc.gpu_handle);
	}
	CLOVER_VERBOSE("Free %zd shaders", g_shaders_pool.get_size());
	CLOVER_VERBOSE("Finished cleaning up shading module...");
}

void process_shading_command_buffer(const size i_cmdBuffId)
{
	PROFILE_SCOPE("process_shading_command_buffer");

	detail::gpu_command_buffer_t& cmdbuff = get_shading_command_buffer(i_cmdBuffId);

	u64 writeSlot = g_global_counters.current_write_slot;
	g_debug_frame_counters[writeSlot].num_shading_commands += (u32)cmdbuff.get_size();

	for (u32 i = 0; i < cmdbuff.get_size(); i++) {
		gpu_command& gpuCmd = cmdbuff[i];
		gpuCmd.reset_cursor();

		switch (gpuCmd.opcode) {
			case command::shading_command:
			{
				shading_command_t cmd;
				gpuCmd.serialize(cmd);
				switch (cmd.command_type) {
					case shading_command_type_e::shader_compile:
					{
						shader_desc_t& shaderDesc = g_shaders_pool[s32(cmd.shader_compile_data.shader_handle)];
						compile_shader(shaderDesc,
								cmd.shader_compile_data.vs,
								cmd.shader_compile_data.fs,
								cmd.shader_compile_data.reflection);
						break;
					}
					case shading_command_type_e::clean_up_snapshot:
					{
						clean_up_snapshot(cmd.clean_up_snapshot_data.downto_handle);
						break;
					}
					default:
						break;
				}
				break;
			}

			default:
				break;
		}
	}

	cmdbuff.clear();
}

}
}
