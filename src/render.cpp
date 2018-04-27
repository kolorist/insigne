#include "insigne/render.h"

#include "insigne/context.h"
#include "insigne/driver.h"
#include "insigne/buffers.h"
#include "insigne/memory.h"

#include "insigne/renderer.h"

namespace insigne {

#define MAX_GPU_COMMAND_BUFFERS					3

	// -----------------------------------------
	typedef floral::fixed_array<gpu_command, linear_allocator_t>	gpu_command_buffer_t;

	static gpu_command_buffer_t					s_gpu_command_buffer[MAX_GPU_COMMAND_BUFFERS];
	static size									s_front_cmdbuff;
	static size									s_back_cmdbuff;
	// -----------------------------------------
	static floral::condition_variable			s_init_condvar;
	static floral::mutex						s_init_mtx;
	static floral::condition_variable			s_cmdbuffer_condvar;
	static floral::mutex						s_cmdbuffer_mtx;

	// -----------------------------------------
	struct render_state {
		u32										rs_toggles;
		u32										rs_values;
		stencil_mask_t							stencil_mask;
		stencil_ref_t							stencil_ref;
		scissor_pos_t							scissor_x, scissor_y;
		scissor_size_t							scissor_width, scissor_height;
	};
	static render_state							s_render_state;

	void render_thread_func(voidptr i_data)
	{
		create_main_context();
		initialize_renderer();
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
							draw_surface_idx(cmd.surface_handle, cmd.shader_handle);
							break;
						}

					case command::setup_render_state:
						{
							break;
						}

					case command::setup_framebuffer:
						{
							framebuffer_command cmd;
							gpuCmd.serialize(cmd);
							clear_framebuffer(cmd.clear_color_buffer, cmd.clear_depth_buffer);
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
										break;
									}
								
								case stream_type::geometry:
									{
										upload_surface(cmd.surface_idx, cmd.vertices, cmd.indices, cmd.vcount, cmd.icount, cmd.stride);
										break;
									}

								case stream_type::shader:
									{
										compile_shader(cmd.shader_idx, cmd.vertex_str, cmd.fragment_str);
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
							clear_color(cmd.clear_color);
						};

					case command::invalid:
						{
							break;
						}

					default:
						break;
				}
			}

			s_gpu_command_buffer[s_front_cmdbuff].empty();
			s_front_cmdbuff = (s_front_cmdbuff + 1) % MAX_GPU_COMMAND_BUFFERS;
			swap_buffers();
		}
	}

	// -----------------------------------------
	void initialize_render_thread()
	{
		for (size i = 0; i < MAX_GPU_COMMAND_BUFFERS; i++)
			s_gpu_command_buffer[i].init(GPU_COMMAND_BUFFER_SIZE, &g_persistance_allocator);
		s_front_cmdbuff = 0;
		s_back_cmdbuff = 2;

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
		s_gpu_command_buffer[s_back_cmdbuff].push_back(newCmd);
	}

	void push_command(const render_command& i_cmd)
	{
		gpu_command newCmd;
		newCmd.opcode = command::draw_geom;
		newCmd.deserialize(i_cmd);
		s_gpu_command_buffer[s_back_cmdbuff].push_back(newCmd);
	}

	void push_command(const stream_command& i_cmd)
	{
		gpu_command newCmd;
		newCmd.opcode = command::stream_data;
		newCmd.deserialize(i_cmd);
		s_gpu_command_buffer[s_back_cmdbuff].push_back(newCmd);
	}
	// -----------------------------------------
	void begin_frame()
	{
		framebuffer_command cmd;
		cmd.clear_color_buffer = true;
		cmd.clear_depth_buffer = true;
		gpu_command newCmd;
		newCmd.opcode = command::setup_framebuffer;
		newCmd.deserialize(cmd);
		s_gpu_command_buffer[s_back_cmdbuff].push_back(newCmd);
	}

	void end_frame()
	{
		while ((s_back_cmdbuff + 1) % MAX_GPU_COMMAND_BUFFERS == s_front_cmdbuff);
		s_back_cmdbuff = (s_back_cmdbuff + 1) % MAX_GPU_COMMAND_BUFFERS;
	}

	void dispatch_frame()
	{
		s_cmdbuffer_condvar.notify_one();
	}

	// -----------------------------------------
	void set_clear_color(f32 i_red, f32 i_green, f32 i_blue, f32 i_alpha)
	{
		init_command cmd;
		cmd.clear_color = floral::vec4f(i_red, i_green, i_blue, i_alpha);
		push_command(cmd);
	}

	void upload_texture(texture_format_e i_format, voidptr i_data)
	{
		stream_command cmd;
		cmd.data_type = stream_type::texture;
		cmd.data = i_data;
		cmd.format = i_format;
		push_command(cmd);
	}

	const surface_handle_t upload_surface(voidptr i_vertices, voidptr i_indices, size i_stride, const u32 i_vcount, const u32 i_icount)
	{
		stream_command cmd;
		cmd.data_type = stream_type::geometry;
		cmd.vertices = i_vertices;
		cmd.indices = i_indices;
		cmd.stride = i_stride;
		cmd.vcount = i_vcount;
		cmd.icount = i_icount;
		cmd.has_indices = true;
		cmd.surface_idx = create_surface();

		push_command(cmd);

		return cmd.surface_idx;
	}

	const shader_handle_t compile_shader(const_cstr i_vertstr, const_cstr i_fragstr)
	{
		stream_command cmd;
		cmd.data_type = stream_type::shader;
		cmd.vertex_str = i_vertstr;
		cmd.fragment_str = i_fragstr;
		cmd.shader_idx = create_shader();

		push_command(cmd);
		return cmd.shader_idx;
	}

	void draw_surface(const surface_handle_t i_surfaceHdl, const shader_handle_t i_shaderHdl)
	{
		render_command cmd;
		cmd.surface_handle = i_surfaceHdl;
		cmd.shader_handle = i_shaderHdl;

		push_command(cmd);
	}

}
