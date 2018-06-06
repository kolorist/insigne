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
	// render state
	void set_depth_test(const bool i_enable)
	{
		if (i_enable != TEST_BIT_BOOL(detail::s_render_state.toggles, static_cast<u32>(render_state_togglemask_e::depth_test))) {
			SET_BIT(detail::s_render_state.toggles, static_cast<u32>(render_state_togglemask_e::depth_test));		// update state
			SET_BIT(detail::s_render_state_changelog, static_cast<u32>(render_state_changelog_e::depth_test));	// update change log
		}
	}

	void set_depth_write(const bool i_enable)
	{
		if (i_enable != TEST_BIT_BOOL(detail::s_render_state.toggles, static_cast<u32>(render_state_togglemask_e::depth_write))) {
			SET_BIT(detail::s_render_state.toggles, static_cast<u32>(render_state_togglemask_e::depth_write));	// update state
			SET_BIT(detail::s_render_state_changelog, static_cast<u32>(render_state_changelog_e::depth_write));	// update change log
		}
	}

	void set_depth_func(const compare_func_e i_func)
	{
		detail::s_render_state.depth_func = i_func;
	}

	void set_cull_face(const bool i_enable)
	{
		if (i_enable != TEST_BIT_BOOL(detail::s_render_state.toggles, static_cast<u32>(render_state_togglemask_e::cull_face))) {
			SET_BIT(detail::s_render_state.toggles, static_cast<u32>(render_state_togglemask_e::cull_face));
			SET_BIT(detail::s_render_state_changelog, static_cast<u32>(render_state_changelog_e::cull_face));
		}
	}

	void set_front_face(const front_face_e i_frontFace)
	{
		detail::s_render_state.front_face = i_frontFace;
	}

	void set_blend(const bool i_enable)
	{
		if (i_enable != TEST_BIT_BOOL(detail::s_render_state.toggles, static_cast<u32>(render_state_togglemask_e::blending))) {
			SET_BIT(detail::s_render_state.toggles, static_cast<u32>(render_state_togglemask_e::blending));
			SET_BIT(detail::s_render_state_changelog, static_cast<u32>(render_state_changelog_e::blending));
		}
	}

	void set_blend_equation(const blend_equation_e i_blendEqu)
	{
		detail::s_render_state.blend_equation = i_blendEqu;
	}

	void set_blend_function(const factor_e i_sfactor, const factor_e i_dfactor)
	{
		detail::s_render_state.blend_func_sfactor = i_sfactor;
		detail::s_render_state.blend_func_dfactor = i_dfactor;
	}

	void set_scissor(const bool i_enable)
	{
		if (i_enable != TEST_BIT_BOOL(detail::s_render_state.toggles, static_cast<u32>(render_state_togglemask_e::scissor_test))) {
			SET_BIT(detail::s_render_state.toggles, static_cast<u32>(render_state_togglemask_e::scissor_test));
			SET_BIT(detail::s_render_state_changelog, static_cast<u32>(render_state_changelog_e::scissor_test));
		}
	}

	void set_scissor_rect(const s32 i_x, const s32 i_y, const s32 i_width, const s32 i_height)
	{
		detail::s_render_state.scissor_x = i_x;
		detail::s_render_state.scissor_y = i_y;
		detail::s_render_state.scissor_width = i_width;
		detail::s_render_state.scissor_height = i_height;
	}

	void commit_render_state()
	{
		// quick quit
		if (!detail::s_render_state_changelog)
			return;

		u32 rst = detail::s_render_state.toggles;
		render_state_t rs = detail::s_render_state;
		u32 cl = detail::s_render_state_changelog;

		//FIXME: cannot update render state properties while the state is ON

		// depth_test
		if (TEST_BIT(rst, static_cast<u32>(render_state_togglemask_e::depth_test))) {
			render_state_toggle_command cmd;
			cmd.toggle = render_state_togglemask_e::depth_test;
			cmd.depth_func = rs.depth_func;
			cmd.to_value = TEST_BIT_BOOL(rst, static_cast<u32>(render_state_togglemask_e::depth_test));
			push_command(cmd);
		}

		// depth_write
		if (TEST_BIT(rst, static_cast<u32>(render_state_togglemask_e::depth_write))) {
			render_state_toggle_command cmd;
			cmd.toggle = render_state_togglemask_e::depth_write;
			cmd.to_value = TEST_BIT_BOOL(rst, static_cast<u32>(render_state_togglemask_e::depth_write));
			push_command(cmd);
		}

		// cull_face
		if (TEST_BIT(rst, static_cast<u32>(render_state_togglemask_e::cull_face))) {
			render_state_toggle_command cmd;
			cmd.toggle = render_state_togglemask_e::cull_face;
			cmd.front_face = rs.front_face;
			cmd.to_value = TEST_BIT_BOOL(rst, static_cast<u32>(render_state_togglemask_e::cull_face));
			push_command(cmd);
		}

		// blending
		if (TEST_BIT(rst, static_cast<u32>(render_state_togglemask_e::blending))) {
			render_state_toggle_command cmd;
			cmd.toggle = render_state_togglemask_e::blending;
			cmd.blend_equation = rs.blend_equation;
			cmd.blend_func_sfactor = rs.blend_func_sfactor;
			cmd.blend_func_dfactor = rs.blend_func_dfactor;
			cmd.to_value = TEST_BIT_BOOL(rst, static_cast<u32>(render_state_togglemask_e::blending));
			push_command(cmd);
		}

		// scissor_test
		if (TEST_BIT(rst, static_cast<u32>(render_state_togglemask_e::scissor_test))) {
			render_state_toggle_command cmd;
			cmd.toggle = render_state_togglemask_e::scissor_test;
			cmd.x = rs.scissor_x;
			cmd.y = rs.scissor_y;
			cmd.width = rs.scissor_width;
			cmd.height = rs.scissor_height;
			cmd.to_value = TEST_BIT_BOOL(rst, static_cast<u32>(render_state_togglemask_e::scissor_test));
			push_command(cmd);
		}

		// stencil_test
		if (TEST_BIT(rst, static_cast<u32>(render_state_togglemask_e::stencil_test))) {
			render_state_toggle_command cmd;
			cmd.toggle = render_state_togglemask_e::stencil_test;
			cmd.stencil_func = rs.stencil_func;
			cmd.stencil_mask = rs.stencil_mask;
			cmd.stencil_ref = rs.stencil_ref;
			cmd.stencil_op_sfail = rs.stencil_op_sfail;
			cmd.stencil_op_dpfail = rs.stencil_op_dpfail;
			cmd.stencil_op_dppass = rs.stencil_op_dppass;
			cmd.to_value = TEST_BIT_BOOL(rst, static_cast<u32>(render_state_togglemask_e::stencil_test));
			push_command(cmd);
		}

		// after this, the render state will be clean
		detail::s_render_state_changelog = 0;

	}
	
	// -----------------------------------------
	// state-dependant
	void begin_frame()
	{
		begin_frame(-1);
	}

	void begin_frame(const framebuffer_handle_t i_fb)
	{
		// setup the framebuffer
		framebuffer_setup_command setupCmd;
		setupCmd.framebuffer_idx = i_fb;
		gpu_command newSetupCmd;
		newSetupCmd.opcode = command::setup_framebuffer;
		newSetupCmd.deserialize(setupCmd);
		detail::s_generic_command_buffer[detail::s_back_cmdbuff].push_back(newSetupCmd);

		// refresh it
		framebuffer_refresh_command refreshCmd;
		refreshCmd.clear_color_buffer = true;
		refreshCmd.clear_depth_buffer = true;
		gpu_command newRefreshCmd;
		newRefreshCmd.opcode = command::refresh_framebuffer;
		newRefreshCmd.deserialize(refreshCmd);
		detail::s_generic_command_buffer[detail::s_back_cmdbuff].push_back(newRefreshCmd);
	}

	void end_frame()
	{
		end_frame(-1);
	}

	void end_frame(const framebuffer_handle_t i_fb)
	{
		while ((detail::s_back_cmdbuff + 1) % BUFFERED_FRAMES == detail::s_front_cmdbuff) Sleep(1);

		detail::s_back_cmdbuff = (detail::s_back_cmdbuff + 1) % BUFFERED_FRAMES;
		s_composing_allocator.free_all();
	}

	void dispatch_frame()
	{
		dispatch_frame(-1);
	}

	void dispatch_frame(const framebuffer_handle_t i_fb)
	{
		detail::s_cmdbuffer_condvar.notify_one();
	}

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
