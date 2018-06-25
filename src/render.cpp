#include "insigne/render.h"

#include <floral.h>
#include <clover.h>

#include "insigne/context.h"
#include "insigne/driver.h"
#include "insigne/detail/render_states.h"

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
	void wait_for_initialization()
	{
		detail::s_init_condvar.wait(detail::s_init_mtx);
	}

	// -----------------------------------------
	void push_command(const framebuffer_init_command& i_cmd)
	{
		gpu_command newCmd;
		newCmd.opcode = command::init_framebuffer;
		newCmd.deserialize(i_cmd);
		detail::s_generic_command_buffer[detail::s_back_cmdbuff].push_back(newCmd);
	}

	void push_command(const init_command& i_cmd)
	{
		gpu_command newCmd;
		newCmd.opcode = command::setup_init_state;
		newCmd.deserialize(i_cmd);
		detail::s_generic_command_buffer[detail::s_back_cmdbuff].push_back(newCmd);
	}

	void push_command(const load_command& i_cmd)
	{
		gpu_command newCmd;
		newCmd.opcode = command::load_data;
		newCmd.deserialize(i_cmd);
		detail::s_generic_command_buffer[detail::s_back_cmdbuff].push_back(newCmd);
	}

	void push_command(const stream_command& i_cmd)
	{
		gpu_command newCmd;
		newCmd.opcode = command::stream_data;
		newCmd.deserialize(i_cmd);
		detail::s_generic_command_buffer[detail::s_back_cmdbuff].push_back(newCmd);
	}

	void push_command(const render_state_toggle_command& i_cmd)
	{
		gpu_command newCmd;
		newCmd.opcode = command::setup_render_state_toggle;
		newCmd.deserialize(i_cmd);
		detail::s_generic_command_buffer[detail::s_back_cmdbuff].push_back(newCmd);
	}

	// -----------------------------------------
	void begin_frame()
	{
		PROFILE_SCOPE(begin_frame);
		detail::s_waiting_for_swap = true;
	}

	void end_frame()
	{
		PROFILE_SCOPE(end_frame);
		// wait for swap present here
		// spin spin spin spin
		while (detail::s_waiting_for_swap);
	}

	// -----------------------------------------
	void begin_render_pass(const framebuffer_handle_t i_fb)
	{
		// setup framebuffer
		{
			framebuffer_setup_command cmd;
			cmd.framebuffer_idx = i_fb;
			gpu_command newGPUCmd;
			newGPUCmd.opcode = command::setup_framebuffer;
			newGPUCmd.deserialize(cmd);
			detail::s_generic_command_buffer[detail::s_back_cmdbuff].push_back(newGPUCmd);
		}
		// clear framebuffer
		{
			framebuffer_refresh_command cmd;
			cmd.clear_color_buffer = true;
			cmd.clear_depth_buffer = true;
			gpu_command newGPUCmd;
			newGPUCmd.opcode = command::refresh_framebuffer;
			newGPUCmd.deserialize(cmd);
			detail::s_generic_command_buffer[detail::s_back_cmdbuff].push_back(newGPUCmd);
		}
	}

	void end_render_pass(const framebuffer_handle_t i_fb)
	{
		// nothing, LUL
	}

	void mark_present_render()
	{
		present_render_command cmd;
		cmd.placeholder = 1;
		gpu_command newGPUCmd;
		newGPUCmd.opcode = command::present_render;
		newGPUCmd.deserialize(cmd);
		detail::s_generic_command_buffer[detail::s_back_cmdbuff].push_back(newGPUCmd);
	}

	void dispatch_render_pass()
	{
		while ((detail::s_back_cmdbuff + 1) % BUFFERED_FRAMES == detail::s_front_cmdbuff) Sleep(1);

		detail::s_back_cmdbuff = (detail::s_back_cmdbuff + 1) % BUFFERED_FRAMES;
		s_composing_allocator.free_all();

		detail::s_cmdbuffer_condvar.notify_one();
	}

	// -----------------------------------------

	void set_clear_color(f32 i_red, f32 i_green, f32 i_blue, f32 i_alpha)
	{
		init_command cmd;
		cmd.clear_color = floral::vec4f(i_red, i_green, i_blue, i_alpha);
		push_command(cmd);
	}

	color_attachment_list_t* allocate_color_attachment_list(const u32 i_attachCount)
	{
		return s_composing_allocator.allocate<color_attachment_list_t>(i_attachCount, &s_composing_allocator);
	}

	const framebuffer_handle_t create_framebuffer(const s32 i_width, const s32 i_height,
			const f32 i_scale, const bool i_hasDepth, const color_attachment_list_t* i_colorAttachs)
	{
		framebuffer_init_command cmd;
		cmd.color_attachment_list = i_colorAttachs;
		cmd.framebuffer_idx = renderer::create_framebuffer(i_colorAttachs->get_size());
		cmd.width = i_width;
		cmd.height = i_height;
		cmd.scale = i_scale;
		cmd.has_depth = i_hasDepth;

		push_command(cmd);
		return cmd.framebuffer_idx;
	}

	const texture_handle_t extract_color_attachment(const framebuffer_handle_t i_fbHdl, const s32 i_idx)
	{
		return renderer::extract_color_attachment(i_fbHdl, i_idx);
	}

	const texture_handle_t create_texture2d(const s32 i_width, const s32 i_height,
			const texture_format_e i_format,
			const filtering_e i_minFilter, const filtering_e i_magFilter,
			const size i_dataSize, voidptr& o_placeholderData, const bool i_hasMM /* = false */)
	{
		voidptr placeholderData = s_composing_allocator.allocate(i_dataSize);
		texture_handle_t texHdl = upload_texture2d(i_width, i_height,
				i_format,
				i_minFilter, i_magFilter,
				placeholderData, i_hasMM);
		o_placeholderData = placeholderData;
		return texHdl;
	}

	const texture_handle_t upload_texture2d(const s32 i_width, const s32 i_height,
			const texture_format_e i_format,
			const filtering_e i_minFilter, const filtering_e i_magFilter,
			voidptr i_data, const bool i_hasMM /* = false */)
	{
		static texture_internal_format_e s_internal_formats[] = {
			texture_internal_format_e::rgb8,
			texture_internal_format_e::rgb16f,
			texture_internal_format_e::srgb8,
			texture_internal_format_e::rgba8,
			texture_internal_format_e::rgba16f,
			texture_internal_format_e::depth24,
			texture_internal_format_e::depth24_stencil8
		};
		static data_type_e s_data_types[] = {
			data_type_e::elem_unsigned_byte,
			data_type_e::elem_signed_float,
			data_type_e::elem_unsigned_byte,
			data_type_e::elem_unsigned_byte,
			data_type_e::elem_signed_float,
			data_type_e::elem_unsigned_int,
			data_type_e::elem_unsigned_int_24_8
		};

		load_command cmd;
		cmd.data_type = stream_type::texture;
		// FIXME: what if user delete the i_data right after this call?
		cmd.data = i_data;
		cmd.format = i_format;
		cmd.width = i_width;
		cmd.height = i_height;
		cmd.internal_format = s_internal_formats[static_cast<s32>(i_format)];
		cmd.pixel_data_type = s_data_types[static_cast<s32>(i_format)];
		cmd.min_filter = i_minFilter;
		cmd.mag_filter = i_magFilter;
		cmd.has_builtin_mipmaps = i_hasMM;
		cmd.texture_idx = renderer::create_texture();
		push_command(cmd);

		return cmd.texture_idx;
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

}
