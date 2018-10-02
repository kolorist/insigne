#include "insigne/render.h"

#include <floral.h>
#include <clover.h>

#include "insigne/system.h"
#include "insigne/context.h"
#include "insigne/driver.h"
#include "insigne/detail/render_states.h"
#include "insigne/counters.h"
#include "insigne/ut_shading.h"
#include "insigne/ut_buffers.h"

namespace insigne {

// -----------------------------------------

#define s_composing_allocator					(*detail::s_gpu_frame_allocator[detail::s_back_cmdbuff])
#define s_rendering_allocator					(*detail::s_gpu_frame_allocator[detail::s_front_cmdbuff])

// -----------------------------------------
// render state
enum class render_state_changelog_e {
	depth_test								= 1u << 0,
	depth_write								= 1u << 1,
	cull_face								= 1u << 2,
	blending								= 1u << 3,
	scissor_test							= 1u << 4,
	stencil_test							= 1u << 5
};

// -----------------------------------------
void push_command(const framebuffer_init_command& i_cmd)
{
	gpu_command newCmd;
	newCmd.opcode = command::init_framebuffer;
	newCmd.deserialize(i_cmd);
	detail::s_generic_command_buffer[detail::s_back_cmdbuff].push_back(newCmd);

	g_debug_frame_counters.num_init_commands++;
}

void push_command(const init_command& i_cmd)
{
	gpu_command newCmd;
	newCmd.opcode = command::setup_init_state;
	newCmd.deserialize(i_cmd);
	detail::s_generic_command_buffer[detail::s_back_cmdbuff].push_back(newCmd);

	g_debug_frame_counters.num_init_commands++;
}

void push_command(const load_command& i_cmd)
{
	gpu_command newCmd;
	newCmd.opcode = command::load_data;
	newCmd.deserialize(i_cmd);
	detail::s_generic_command_buffer[detail::s_back_cmdbuff].push_back(newCmd);

	g_debug_frame_counters.num_load_commands++;
}

void push_command(const stream_command& i_cmd)
{
	gpu_command newCmd;
	newCmd.opcode = command::stream_data;
	newCmd.deserialize(i_cmd);
	detail::s_generic_command_buffer[detail::s_back_cmdbuff].push_back(newCmd);

	g_debug_frame_counters.num_load_commands++;
}

void push_command(const render_state_toggle_command& i_cmd)
{
	gpu_command newCmd;
	newCmd.opcode = command::setup_render_state_toggle;
	newCmd.deserialize(i_cmd);
	detail::s_generic_command_buffer[detail::s_back_cmdbuff].push_back(newCmd);

	g_debug_frame_counters.num_state_commands++;
}

// -----------------------------------------

void set_clear_color(f32 i_red, f32 i_green, f32 i_blue, f32 i_alpha)
{
	init_command cmd;
	cmd.clear_color = floral::vec4f(i_red, i_green, i_blue, i_alpha);
	push_command(cmd);
}


const surface_handle_t upload_surface(voidptr i_vertices, voidptr i_indices,
		s32 i_stride, const u32 i_vcount, const u32 i_icount)
{
	load_command cmd;
	cmd.data_type = stream_type::geometry;
	cmd.vertices = i_vertices;
	cmd.indices = i_indices;
	cmd.draw_type = draw_type_e::static_surface;
	cmd.stride = i_stride;
	cmd.vcount = i_vcount;
	cmd.icount = i_icount;
	cmd.has_indices = true;
	cmd.surface_idx = renderer::create_surface();

	push_command(cmd);

	return cmd.surface_idx;
}

const surface_handle_t upload_surface(voidptr i_vertices, voidptr i_indices,
		s32 i_stride, const u32 i_vcount, const u32 i_icount, const draw_type_e i_drawType)
{
	load_command cmd;
	cmd.data_type = stream_type::geometry;
	cmd.vertices = i_vertices;
	cmd.indices = i_indices;
	cmd.draw_type = i_drawType;
	cmd.stride = i_stride;
	cmd.vcount = i_vcount;
	cmd.icount = i_icount;
	cmd.has_indices = true;
	cmd.surface_idx = renderer::create_surface();

	push_command(cmd);

	return cmd.surface_idx;
}

void update_surface(const surface_handle_t& i_hdl,
		voidptr i_vertices, voidptr i_indices,
		const u32 i_vcount, const u32 i_icount)
{
	stream_command cmd;
	cmd.data_type = stream_type::geometry;
	cmd.vertices = i_vertices;
	cmd.indices = i_indices;
	cmd.vcount = i_vcount;
	cmd.icount = i_icount;
	cmd.surface_idx = i_hdl;

	push_command(cmd);
}

const surface_handle_t upload_surface(voidptr i_vertices, const size i_vsize, voidptr i_indices, const size i_isize,
		const s32 i_stride, const u32 i_vcount, const u32 i_icount)
{
	voidptr vdata = s_composing_allocator.allocate(i_vsize);
	voidptr idata = s_composing_allocator.allocate(i_isize);
	memcpy(vdata, i_vertices, i_vsize);
	memcpy(idata, i_indices, i_isize);

	load_command cmd;
	cmd.data_type = stream_type::geometry;
	cmd.vertices = vdata;
	cmd.indices = idata;
	cmd.draw_type = draw_type_e::static_surface;
	cmd.stride = i_stride;
	cmd.vcount = i_vcount;
	cmd.icount = i_icount;
	cmd.has_indices = true;
	cmd.surface_idx = renderer::create_surface();
	
	push_command(cmd);
	return cmd.surface_idx;
}

const surface_handle_t create_streamed_surface(const s32 i_stride)
{
	load_command cmd;
	cmd.data_type = stream_type::geometry;
	cmd.vertices = nullptr;
	cmd.indices = nullptr;
	cmd.draw_type = draw_type_e::dynamic_surface;
	cmd.stride = i_stride;
	cmd.vcount = 0;
	cmd.icount = 0;
	cmd.has_indices = true;
	cmd.surface_idx = renderer::create_surface();

	push_command(cmd);

	return cmd.surface_idx;
}

const surface_handle_t create_streamed_surface(const surface_descriptor_t& i_desc)
{
	load_command cmd;
	cmd.data_type = stream_type::geometry;
	cmd.vertices = nullptr;
	cmd.indices = nullptr;
	cmd.draw_type = draw_type_e::dynamic_surface;
	cmd.stride = i_desc.vertex_stride;
	cmd.vcount = i_desc.vertices_count;
	cmd.icount = i_desc.indices_count;
	cmd.has_indices = true;
	cmd.surface_idx = renderer::create_surface();

	push_command(cmd);

	return cmd.surface_idx;
}

void update_streamed_surface(const surface_handle_t& i_hdl,
		voidptr i_vertices, const size i_vsize, voidptr i_indices, const size i_isize,
		const u32 i_vcount, const u32 i_icount)
{
	voidptr vdata = s_composing_allocator.allocate(i_vsize);
	voidptr idata = s_composing_allocator.allocate(i_isize);
	memcpy(vdata, i_vertices, i_vsize);
	memcpy(idata, i_indices, i_isize);

	stream_command cmd;
	cmd.data_type = stream_type::geometry;
	cmd.vertices = vdata;
	cmd.indices = idata;
	cmd.vcount = i_vcount;
	cmd.icount = i_icount;
	cmd.surface_idx = i_hdl;

	push_command(cmd);
}

void update_streamed_surface_vertices(const surface_handle_t i_hdl, voidptr i_vertices, const u32 i_vcount)
{
}

shader_param_list_t* allocate_shader_param_list(const u32 i_paramCount)
{
	return s_composing_allocator.allocate<shader_param_list_t>(i_paramCount, &s_composing_allocator);
}

const shader_handle_t compile_shader(const_cstr i_vertStr, const_cstr i_fragStr, const shader_param_list_t* i_paramList)
{
	load_command cmd;
	cmd.data_type = stream_type::shader;
	size vertStrLen = strlen(i_vertStr);
	size fragStrLen = strlen(i_fragStr);
	cstr vertStr = (cstr)s_composing_allocator.allocate(vertStrLen + 1);
	cstr fragStr = (cstr)s_composing_allocator.allocate(fragStrLen + 1);
	strcpy(vertStr, i_vertStr);
	strcpy(fragStr, i_fragStr);
	cmd.vertex_str = vertStr;
	cmd.fragment_str = fragStr;
	cmd.shader_idx = renderer::create_shader(i_paramList);
	cmd.shader_param_list = i_paramList;

	push_command(cmd);
	return cmd.shader_idx;
}

const material_handle_t create_material(const shader_handle_t i_fromShader)
{
	material_t newMaterial;
	newMaterial.shader_handle = i_fromShader;
	const renderer::material_template_t& matTemplate = renderer::get_material_template(i_fromShader);

	// create material from template
	for (u32 i = 0; i < matTemplate.float_param_ids.get_size(); i++) {
		id_value_pair_t<f32> newParam;
		newParam.id = matTemplate.float_param_ids[i];
		newParam.value = 0.0f;
		newMaterial.float_params.push_back(newParam);
	}

	for (u32 i = 0; i < matTemplate.vec3_param_ids.get_size(); i++) {
		id_value_pair_t<floral::vec3f> newParam;
		newParam.id = matTemplate.vec3_param_ids[i];
		newParam.value = floral::vec3f(0.0f);
		newMaterial.vec3_params.push_back(newParam);
	}

	for (u32 i = 0; i < matTemplate.mat4_param_ids.get_size(); i++) {
		id_value_pair_t<floral::mat4x4f> newParam;
		newParam.id = matTemplate.mat4_param_ids[i];
		newParam.value = floral::mat4x4f();
		newMaterial.mat4_params.push_back(newParam);
	}

	for (u32 i = 0; i < matTemplate.texture2d_param_ids.get_size(); i++) {
		id_value_pair_t<texture_handle_t> newParam;
		newParam.id = matTemplate.texture2d_param_ids[i];
		newParam.value = 0;
		newMaterial.texture2d_params.push_back(newParam);
	}

	for (u32 i = 0; i < matTemplate.texturecube_param_ids.get_size(); i++) {
		id_value_pair_t<texture_handle_t> newParam;
		newParam.id = matTemplate.texturecube_param_ids[i];
		newParam.value = 0;
		newMaterial.texturecube_params.push_back(newParam);
	}

	for (u32 i = 0; i < matTemplate.uniform_block_param_ids.get_size(); i++) {
		id_value_pair_t<ub_handle_t> newParam;
		newParam.id = matTemplate.uniform_block_param_ids[i];
		newParam.value = 0;
		newMaterial.uniform_block_params.push_back(newParam);
	}

	material_handle_t newMatHdl = static_cast<material_handle_t>(detail::s_materials.get_size());
	detail::s_materials.push_back(newMaterial);
	return newMatHdl;
}

template <>
const param_id get_material_param<f32>(const material_handle_t i_hdl, const_cstr i_name)
{
	material_t& thisMaterial = detail::s_materials[static_cast<s32>(i_hdl)];
	floral::crc_string idToSearch(i_name);

	for (u32 i = 0; i < thisMaterial.float_params.get_size(); i++) {
		if (thisMaterial.float_params[i].id == idToSearch)
			return i;
	}

	return param_id(-1);
}

template <>
const param_id get_material_param<floral::vec3f>(const material_handle_t i_hdl, const_cstr i_name)
{
	material_t& thisMaterial = detail::s_materials[static_cast<s32>(i_hdl)];
	floral::crc_string idToSearch(i_name);

	for (u32 i = 0; i < thisMaterial.vec3_params.get_size(); i++) {
		if (thisMaterial.vec3_params[i].id == idToSearch)
			return i;
	}

	return param_id(-1);
}

template <>
const param_id get_material_param<floral::mat4x4f>(const material_handle_t i_hdl, const_cstr i_name)
{
	material_t& thisMaterial = detail::s_materials[static_cast<s32>(i_hdl)];
	floral::crc_string idToSearch(i_name);

	for (u32 i = 0; i < thisMaterial.mat4_params.get_size(); i++) {
		if (thisMaterial.mat4_params[i].id == idToSearch)
			return i;
	}

	return param_id(-1);
}

template <>
const param_id get_material_param<texture_handle_t>(const material_handle_t i_hdl, const_cstr i_name)
{
	material_t& thisMaterial = detail::s_materials[static_cast<s32>(i_hdl)];
	floral::crc_string idToSearch(i_name);

	for (u32 i = 0; i < thisMaterial.texture2d_params.get_size(); i++) {
		if (thisMaterial.texture2d_params[i].id == idToSearch)
			return i;
	}

	return param_id(-1);
}

const param_id get_material_param_texcube(const material_handle_t i_hdl, const_cstr i_name)
{
	material_t& thisMaterial = detail::s_materials[static_cast<s32>(i_hdl)];
	floral::crc_string idToSearch(i_name);

	for (u32 i = 0; i < thisMaterial.texturecube_params.get_size(); i++) {
		if (thisMaterial.texturecube_params[i].id == idToSearch)
			return i;
	}

	return param_id(-1);
}

const param_id get_material_param_ub(const material_handle_t i_hdl, const_cstr i_name)
{
	material_t& thisMaterial = detail::s_materials[static_cast<s32>(i_hdl)];
	floral::crc_string idToSearch(i_name);

	for (u32 i = 0; i < thisMaterial.uniform_block_params.get_size(); i++) {
		if (thisMaterial.uniform_block_params[i].id == idToSearch)
			return i;
	}

	return param_id(-1);
}


template <>
void set_material_param(const material_handle_t i_hdl, const param_id i_paramId, const f32& i_value)
{
	s32 pidx = static_cast<s32>(i_paramId);
	if (pidx < 0) return;

	material_t& thisMaterial = detail::s_materials[static_cast<s32>(i_hdl)];
	thisMaterial.float_params[pidx].value = i_value;
}

template <>
void set_material_param(const material_handle_t i_hdl, const param_id i_paramId, const floral::vec3f& i_value)
{
	s32 pidx = static_cast<s32>(i_paramId);
	if (pidx < 0) return;

	material_t& thisMaterial = detail::s_materials[static_cast<s32>(i_hdl)];
	thisMaterial.vec3_params[pidx].value = i_value;
}

template <>
void set_material_param(const material_handle_t i_hdl, const param_id i_paramId, const floral::mat4x4f& i_value)
{
	s32 pidx = static_cast<s32>(i_paramId);
	if (pidx < 0) return;

	material_t& thisMaterial = detail::s_materials[static_cast<s32>(i_hdl)];
	thisMaterial.mat4_params[pidx].value = i_value;
}

template <>
void set_material_param(const material_handle_t i_hdl, const param_id i_paramId, const texture_handle_t& i_value)
{
	s32 pidx = static_cast<s32>(i_paramId);
	if (pidx < 0) return;

	material_t& thisMaterial = detail::s_materials[static_cast<s32>(i_hdl)];
	thisMaterial.texture2d_params[pidx].value = i_value;
}

void set_material_param_texcube(const material_handle_t i_hdl, const param_id i_paramId, const texture_handle_t i_tex)
{
	s32 pidx = static_cast<s32>(i_paramId);
	if (pidx < 0) return;

	material_t& thisMaterial = detail::s_materials[static_cast<s32>(i_hdl)];
	thisMaterial.texturecube_params[pidx].value = i_tex;
}

void set_material_param_ub(const material_handle_t i_hdl, const param_id i_paramId, const ub_handle_t i_value)
{
	s32 pidx = static_cast<s32>(i_paramId);
	if (pidx < 0) return;

	material_t& thisMaterial = detail::s_materials[static_cast<s32>(i_hdl)];
	thisMaterial.uniform_block_params[pidx].value = i_value;
}

}
