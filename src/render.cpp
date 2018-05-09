#include "insigne/render.h"

#include <floral.h>
#include <clover.h>

#include "insigne/commons.h"
#include "insigne/internal_commons.h"
#include "insigne/context.h"
#include "insigne/driver.h"
#include "insigne/buffers.h"
#include "insigne/memory.h"

#include "insigne/renderer.h"

namespace insigne {

#define BUFFERED_FRAMES							3

	// -----------------------------------------
	typedef floral::fixed_array<gpu_command, linear_allocator_t>	gpu_command_buffer_t;

	static gpu_command_buffer_t					s_gpu_command_buffer[BUFFERED_FRAMES];
	static arena_allocator_t*					s_gpu_frame_allocator[BUFFERED_FRAMES];

#define s_composing_cmdbuff						s_gpu_command_buffer[s_back_cmdbuff]
#define s_rendering_cmdbuff						s_gpu_command_buffer[s_front_cmdbuff]
#define s_composing_allocator					(*s_gpu_frame_allocator[s_back_cmdbuff])
#define s_rendering_allocator					(*s_gpu_frame_allocator[s_front_cmdbuff])
	
	static size									s_front_cmdbuff;
	static size									s_back_cmdbuff;
	// -----------------------------------------
	static floral::condition_variable			s_init_condvar;
	static floral::mutex						s_init_mtx;
	static floral::condition_variable			s_cmdbuffer_condvar;
	static floral::mutex						s_cmdbuffer_mtx;

	// -----------------------------------------
	// render state
	static render_state_t						s_render_state;
	enum class render_state_changelog_e {
		depth_test								= 1u << 0,
		depth_write								= 1u << 2,
		cull_face								= 1u << 3,
		blending								= 1u << 4,
		scissor_test							= 1u << 5,
		stencil_test							= 1u << 6
	};

	u32											s_render_state_changelog;

	// materials are also render states
	template <typename t_value>
	struct id_value_pair_t {
		floral::crc_string						id;
		t_value									value;
	};

	template <typename t_value, u32 t_capacity>
	using param_array_t = floral::inplace_array<id_value_pair_t<t_value>, t_capacity>;

	struct material_t {
		shader_handle_t							shader_handle;
		param_array_t<f32, 8u>					float_params;
		param_array_t<texture_handle_t, 4u>		texture2d_params;
	};

	// TODO: this should be a memory pool of materials
	static floral::fixed_array<material_t, linear_allocator_t>	s_materials;

	// -----------------------------------------

	void render_thread_func(voidptr i_data)
	{
		create_main_context();
		renderer::initialize_renderer();
		s_init_condvar.notify_one();
		while(true)
		{
			while (s_front_cmdbuff == s_back_cmdbuff)
				s_cmdbuffer_condvar.wait(s_cmdbuffer_mtx);

			for (u32 i = 0; i < s_gpu_command_buffer[s_front_cmdbuff].get_size(); i++) {
				gpu_command& gpuCmd = s_gpu_command_buffer[s_front_cmdbuff][i];
				gpuCmd.reset_cursor();
				switch (gpuCmd.opcode) {
					case command::draw_geom:
						{
							render_command cmd;
							gpuCmd.serialize(cmd);
							renderer::draw_surface_idx(cmd.surface_handle, cmd.shader_handle);
							break;
						}

					case command::setup_render_state_toggle:
						{
							render_state_toggle_command cmd;
							gpuCmd.serialize(cmd);
							switch (cmd.toggle) {
								// TODO: what the heck, there is no point of using meta-prog along with runtime decision. FIX IT
								case render_state_togglemask_e::depth_test:
									{
										if (cmd.to_value)
											renderer::set_depth_test<true_type>(cmd.depth_func);
										else renderer::set_depth_test<false_type>(cmd.depth_func);
										break;
									}
								case render_state_togglemask_e::depth_write:
									{
										if (cmd.to_value)
											renderer::set_depth_write<true_type>();
										else renderer::set_depth_write<false_type>();
										break;
									}
								case render_state_togglemask_e::cull_face:
									{
										if (cmd.to_value)
											renderer::set_cull_face<true_type>(cmd.front_face);
										else renderer::set_cull_face<false_type>(cmd.front_face);
										break;
									}
								case render_state_togglemask_e::blending:
									{
										if (cmd.to_value)
											renderer::set_blending<true_type>(cmd.blend_equation, cmd.blend_func_sfactor, cmd.blend_func_dfactor);
										else renderer::set_blending<false_type>(cmd.blend_equation, cmd.blend_func_sfactor, cmd.blend_func_dfactor);
										break;
									}
								case render_state_togglemask_e::scissor_test:
									{
										if (cmd.to_value)
											renderer::set_scissor_test<true_type>(cmd.x, cmd.y, cmd.width, cmd.height);
										else renderer::set_scissor_test<false_type>(cmd.x, cmd.y, cmd.width, cmd.height);
										break;
									}
								case render_state_togglemask_e::stencil_test:
									{
										if (cmd.to_value)
											renderer::set_stencil_test<true_type>(cmd.stencil_func, cmd.stencil_mask, cmd.stencil_ref, cmd.stencil_op_sfail, cmd.stencil_op_dpfail, cmd.stencil_op_dppass);
										else renderer::set_stencil_test<false_type>(cmd.stencil_func, cmd.stencil_mask, cmd.stencil_ref, cmd.stencil_op_sfail, cmd.stencil_op_dpfail, cmd.stencil_op_dppass);
										break;
									}
								default:
									break;
							};
							break;
						}

					case command::setup_framebuffer:
						{
							framebuffer_command cmd;
							gpuCmd.serialize(cmd);
							renderer::clear_framebuffer(cmd.clear_color_buffer, cmd.clear_depth_buffer);
							break;
						}

					case command::stream_data:
						{
							stream_command cmd;
							gpuCmd.serialize(cmd);
							switch (cmd.data_type) {
								case stream_type::texture:
									{
										// TODO: add
										renderer::upload_texture2d(cmd.texture_idx, cmd.width, cmd.height, cmd.format,
												cmd.internal_format, cmd.pixel_data_type, cmd.data);
										break;
									}
								
								case stream_type::geometry:
									{
										renderer::upload_surface(cmd.surface_idx, cmd.vertices, cmd.indices, cmd.vcount, cmd.icount, cmd.stride);
										break;
									}

								case stream_type::shader:
									{
										renderer::compile_shader(cmd.shader_idx, cmd.vertex_str, cmd.fragment_str, cmd.shader_param_list);
										break;
									}

								case stream_type::material:
									{
										//renderer::compile_material(cmd.material_idx, cmd.from_shader, *cmd.param_list);
										break;
									}
								
								default:
									break;
							}
							break;
						}

					case command::setup_init_state:
						{
							init_command cmd;
							gpuCmd.serialize(cmd);
							renderer::clear_color(cmd.clear_color);
						};

					case command::invalid:
						{
							break;
						}

					default:
						break;
				}
			}

			//s_gpu_command_buffer[s_front_cmdbuff].empty();
			s_front_cmdbuff = (s_front_cmdbuff + 1) % BUFFERED_FRAMES;
			swap_buffers();
		}
	}

	// -----------------------------------------
	void initialize_render_thread()
	{
		FLORAL_ASSERT_MSG(sizeof(init_command) <= COMMAND_PAYLOAD_SIZE, "Command exceeds payload's capacity!");
		FLORAL_ASSERT_MSG(sizeof(render_command) <= COMMAND_PAYLOAD_SIZE, "Command exceeds payload's capacity!");
		FLORAL_ASSERT_MSG(sizeof(stream_command) <= COMMAND_PAYLOAD_SIZE, "Command exceeds payload's capacity!");
		FLORAL_ASSERT_MSG(sizeof(render_state_toggle_command) <= COMMAND_PAYLOAD_SIZE, "Command exceeds payload's capacity!");

		for (u32 i = 0; i < BUFFERED_FRAMES; i++)
			s_gpu_command_buffer[i].init(GPU_COMMAND_BUFFER_SIZE, &g_persistance_allocator);

		for (u32 i = 0; i < BUFFERED_FRAMES; i++)
			s_gpu_frame_allocator[i] = g_persistance_allocator.allocate_arena<arena_allocator_t>(SIZE_MB(8));

		s_materials.init(32, &g_persistance_allocator);

		s_front_cmdbuff = 0;
		s_back_cmdbuff = 2;
		s_render_state_changelog = 0;

		g_render_thread.entry_point = &insigne::render_thread_func;
		g_render_thread.ptr_data = nullptr;
		g_render_thread.start();
	}

	void wait_for_initialization()
	{
		s_init_condvar.wait(s_init_mtx);
	}

	// -----------------------------------------
	void push_command(const init_command& i_cmd)
	{
		gpu_command newCmd;
		newCmd.opcode = command::setup_init_state;
		newCmd.deserialize(i_cmd);
		//s_gpu_command_buffer[s_back_cmdbuff].push_back(newCmd);
		s_composing_cmdbuff.push_back(newCmd);
	}

	void push_command(const render_command& i_cmd)
	{
		gpu_command newCmd;
		newCmd.opcode = command::draw_geom;
		newCmd.deserialize(i_cmd);
		s_composing_cmdbuff.push_back(newCmd);
	}

	void push_command(const stream_command& i_cmd)
	{
		gpu_command newCmd;
		newCmd.opcode = command::stream_data;
		newCmd.deserialize(i_cmd);
		s_composing_cmdbuff.push_back(newCmd);
	}

	void push_command(const render_state_toggle_command& i_cmd)
	{
		gpu_command newCmd;
		newCmd.opcode = command::setup_render_state_toggle;
		newCmd.deserialize(i_cmd);
		s_composing_cmdbuff.push_back(newCmd);
	}

	// -----------------------------------------
	// render state
	void set_depth_test(const bool i_enable)
	{
		if (i_enable != TEST_BIT_BOOL(s_render_state.toggles, static_cast<u32>(render_state_togglemask_e::depth_test))) {
			SET_BIT(s_render_state.toggles, static_cast<u32>(render_state_togglemask_e::depth_test));		// update state
			SET_BIT(s_render_state_changelog, static_cast<u32>(render_state_changelog_e::depth_test));	// update change log
		}
	}

	void set_depth_write(const bool i_enable)
	{
		if (i_enable != TEST_BIT_BOOL(s_render_state.toggles, static_cast<u32>(render_state_togglemask_e::depth_write))) {
			SET_BIT(s_render_state.toggles, static_cast<u32>(render_state_togglemask_e::depth_write));	// update state
			SET_BIT(s_render_state_changelog, static_cast<u32>(render_state_changelog_e::depth_write));	// update change log
		}
	}

	void set_depth_func(const compare_func_e i_func)
	{
		s_render_state.depth_func = i_func;
	}

	void commit_render_state()
	{
		// quick quit
		if (!s_render_state_changelog)
			return;

		u32 rst = s_render_state.toggles;
		render_state_t rs = s_render_state;
		u32 cl = s_render_state_changelog;

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
		s_render_state_changelog = 0;
	}
	
	// -----------------------------------------
	// state-dependant
	void begin_frame()
	{
		framebuffer_command cmd;
		cmd.clear_color_buffer = true;
		cmd.clear_depth_buffer = true;
		gpu_command newCmd;
		newCmd.opcode = command::setup_framebuffer;
		newCmd.deserialize(cmd);
		s_composing_cmdbuff.push_back(newCmd);
	}

	void end_frame()
	{
		while ((s_back_cmdbuff + 1) % BUFFERED_FRAMES == s_front_cmdbuff);

		s_back_cmdbuff = (s_back_cmdbuff + 1) % BUFFERED_FRAMES;
		s_composing_cmdbuff.empty();
		s_composing_allocator.free_all();
	}

	void dispatch_frame()
	{
		s_cmdbuffer_condvar.notify_one();
	}

	void set_clear_color(f32 i_red, f32 i_green, f32 i_blue, f32 i_alpha)
	{
		init_command cmd;
		cmd.clear_color = floral::vec4f(i_red, i_green, i_blue, i_alpha);
		push_command(cmd);
	}

	const texture_handle_t upload_texture2d(const s32 i_width, const s32 i_height, const texture_format_e i_format, voidptr i_data)
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

		stream_command cmd;
		cmd.data_type = stream_type::texture;
		cmd.data = i_data;
		cmd.format = i_format;
		cmd.width = i_width;
		cmd.height = i_height;
		cmd.internal_format = s_internal_formats[static_cast<s32>(i_format)];
		cmd.pixel_data_type = s_data_types[static_cast<s32>(i_format)];
		cmd.texture_idx = renderer::create_texture();
		push_command(cmd);

		return cmd.texture_idx;
	}

	const surface_handle_t upload_surface(voidptr i_vertices, voidptr i_indices, s32 i_stride, const u32 i_vcount, const u32 i_icount)
	{
		stream_command cmd;
		cmd.data_type = stream_type::geometry;
		cmd.vertices = i_vertices;
		cmd.indices = i_indices;
		cmd.stride = i_stride;
		cmd.vcount = i_vcount;
		cmd.icount = i_icount;
		cmd.has_indices = true;
		cmd.surface_idx = renderer::create_surface();

		push_command(cmd);

		return cmd.surface_idx;
	}

	shader_param_list_t* allocate_shader_param_list(const u32 i_paramCount)
	{
		return s_composing_allocator.allocate<shader_param_list_t>(i_paramCount, &s_composing_allocator);
	}

	const shader_handle_t compile_shader(const_cstr i_vertStr, const_cstr i_fragStr, const shader_param_list_t* i_paramList)
	{
		stream_command cmd;
		cmd.data_type = stream_type::shader;
		cmd.vertex_str = i_vertStr;
		cmd.fragment_str = i_fragStr;
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

		for (u32 i = 0; i < matTemplate.texture2d_param_ids.get_size(); i++) {
			id_value_pair_t<texture_handle_t> newParam;
			newParam.id = matTemplate.texture2d_param_ids[i];
			newParam.value = 0;
			newMaterial.texture2d_params.push_back(newParam);
		}

		material_handle_t newMatHdl = static_cast<material_handle_t>(s_materials.get_size());
		s_materials.push_back(newMaterial);
		return newMatHdl;
	}

	// state dependant
	void draw_surface(const surface_handle_t i_surfaceHdl, const shader_handle_t i_shaderHdl)
	{
		commit_render_state();
		render_command cmd;
		cmd.surface_handle = i_surfaceHdl;
		cmd.shader_handle = i_shaderHdl;

		push_command(cmd);
	}

}
