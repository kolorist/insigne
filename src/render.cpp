#include "insigne/render.h"

#include "insigne/context.h"
#include "insigne/driver.h"
#include "insigne/buffers.h"
#include "insigne/memory.h"

#include "insigne/renderer_gl.h"

namespace insigne {

	// -----------------------------------------
	typedef floral::fixed_array<gpu_command, linear_allocator_t>	gpu_command_buffer_t;

	static gpu_command_buffer_t					s_gpu_command_buffer;
	// -----------------------------------------
	static floral::condition_variable			s_init_condvar;
	static floral::mutex						s_init_mtx;
	static floral::condition_variable			s_cmdbuffer_condvar;
	static floral::mutex						s_cmdbuffer_mtx;

	void render_thread_func(voidptr i_data)
	{
		create_main_context();
		initialize_renderer();
		s_init_condvar.notify_one();
		while(true)
		{
			s_cmdbuffer_condvar.wait(s_cmdbuffer_mtx);
			for (u32 i = 0; i < s_gpu_command_buffer.get_size(); i++) {
				gpu_command& gpuCmd = s_gpu_command_buffer[i];
				gpuCmd.reset_cursor();
				switch (gpuCmd.opcode) {
					case command::draw_geom:
						{
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

			s_gpu_command_buffer.empty();
			swap_buffers();
		}
	}

	// -----------------------------------------
	void initialize_render_thread()
	{
		s_gpu_command_buffer.init(GPU_COMMAND_BUFFER_SIZE, &g_persistance_allocator);

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
		s_gpu_command_buffer.push_back(newCmd);
	}

	void push_command(const render_command& i_cmd)
	{
		gpu_command newCmd;
		newCmd.opcode = command::draw_geom;
		newCmd.deserialize(i_cmd);
		s_gpu_command_buffer.push_back(newCmd);
	}

	void push_command(const stream_command& i_cmd)
	{
		gpu_command newCmd;
		newCmd.opcode = command::stream_data;
		newCmd.deserialize(i_cmd);
		s_gpu_command_buffer.push_back(newCmd);
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
		s_gpu_command_buffer.push_back(newCmd);
	}

	void end_frame()
	{
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

	void upload_texture(texture_format i_format, voidptr i_data)
	{
		stream_command cmd;
		cmd.data_type = stream_type::texture;
		cmd.data = i_data;
		cmd.format = i_format;
		push_command(cmd);
	}

	const surface_handle upload_geometry(voidptr i_vertices, voidptr i_indices, size i_stride, const u32 i_vcount, const u32 i_icount)
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

	const shader_handle compile_shader(const_cstr i_vertstr, const_cstr i_fragstr)
	{
		stream_command cmd;
		cmd.data_type = stream_type::shader;
		cmd.vertex_str = i_vertstr;
		cmd.fragment_str = i_fragstr;
		cmd.shader_idx = create_shader();

		push_command(cmd);
		return cmd.shader_idx;
	}

}
