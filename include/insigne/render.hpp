#include "detail/render.h"
#include "detail/render_states.h"

#include <lotus/profiler.h>

#include "renderer.h"
#include "counters.h"

namespace insigne {

	// -----------------------------------------
	template <typename t_surface>
	void renderable_surface_t<t_surface>::render()
	{
		PROFILE_SCOPE(render);

		for (u32 i = 0; i < detail::draw_command_buffer_t<t_surface>::command_buffer[detail::s_front_cmdbuff].get_size(); i++) {
			gpu_command& gpuCmd = detail::draw_command_buffer_t<t_surface>::command_buffer[detail::s_front_cmdbuff][i];
			gpuCmd.reset_cursor();
			switch (gpuCmd.opcode) {
				case command::setup_scissor_state:
					{
						scissor_state_command cmd;
						gpuCmd.serialize(cmd);
						if (cmd.scissor_test) {
							renderer::set_scissor_test<true_type>(cmd.x, cmd.y, cmd.width, cmd.height);
						} else {
							renderer::set_scissor_test<false_type>(0, 0, 0, 0);
						}
						break;
					}
				case command::draw_geom:
					{
						PROFILE_SCOPE(draw_geom);
						render_command cmd;
						gpuCmd.serialize(cmd);
						renderer::draw_surface_idx<t_surface>(cmd.surface_handle, *cmd.material_snapshot,
								cmd.segment_size, cmd.segment_offset);
						break;
					}
				default:
					break;
			}
		}
	}

	template <typename t_surface>
	void renderable_surface_t<t_surface>::init_buffer(insigne::linear_allocator_t* i_allocator)
	{
		using namespace insigne;
		for (u32 i = 0; i < BUFFERED_FRAMES; i++)
			detail::draw_command_buffer_t<t_surface>::command_buffer[i].init(g_renderer_settings.draw_command_buffer_size, &g_persistance_allocator);
	}

	// -----------------------------------------
	template <typename t_surface_list>
	void render_thread_func(voidptr i_data)
	{
		// profiler init
		lotus::init_capture_for_this_thread(1, "render_thread");
		create_main_context();
		renderer::initialize_renderer();
		detail::s_init_condvar.notify_one();

		while (true) {
			{
				PROFILE_SCOPE(WaitForCommandBuffer);
				while (detail::s_front_cmdbuff == detail::s_back_cmdbuff)
					detail::s_cmdbuffer_condvar.wait(detail::s_cmdbuffer_mtx);
			}

			g_global_counters.current_render_frame_idx++;

			bool swapThisRenderPass = false;
			// generic phase
			for (u32 i = 0; i < detail::s_generic_command_buffer[detail::s_front_cmdbuff].get_size(); i++) {
				gpu_command& gpuCmd = detail::s_generic_command_buffer[detail::s_front_cmdbuff][i];
				gpuCmd.reset_cursor();
				switch (gpuCmd.opcode) {
					case command::init_framebuffer:
						{
							framebuffer_init_command cmd;
							gpuCmd.serialize(cmd);
							renderer::init_framebuffer(cmd.framebuffer_idx, cmd.width, cmd.height, cmd.scale,
									cmd.has_depth, cmd.color_attachment_list);
							break;
						}

					case command::setup_framebuffer:
						{
							PROFILE_SCOPE(SetupFramebuffer);
							framebuffer_setup_command cmd;
							gpuCmd.serialize(cmd);
							if (!cmd.has_custom_viewport) {
								renderer::setup_framebuffer(cmd.framebuffer_idx);
							} else {
								renderer::setup_framebuffer(cmd.framebuffer_idx, cmd.lower_left_x, cmd.lower_left_y, cmd.width, cmd.height);
							}
							break;
						}

					case command::refresh_framebuffer:
						{
							PROFILE_SCOPE(RefreshFramebuffer);
							framebuffer_refresh_command cmd;
							gpuCmd.serialize(cmd);
							if (!cmd.region_clear) {
								renderer::set_scissor_test<false_type>(0, 0, 0, 0);
							} else {
								renderer::set_scissor_test<true_type>(cmd.x, cmd.y, cmd.width, cmd.height);
							}
							renderer::clear_framebuffer(cmd.clear_color_buffer, cmd.clear_depth_buffer);
							break;
						}

					case command::load_data:
						{
							load_command cmd;
							gpuCmd.serialize(cmd);
							switch (cmd.data_type) {
								case stream_type::texture2d:
									{
										if (cmd.has_builtin_mipmaps)
											renderer::upload_texture2d_mm(cmd.texture_idx, cmd.width, cmd.height, cmd.format,
													cmd.internal_format, cmd.pixel_data_type, cmd.data,
													cmd.min_filter, cmd.mag_filter);
										else
											renderer::upload_texture2d(cmd.texture_idx, cmd.width, cmd.height, cmd.format,
													cmd.internal_format, cmd.pixel_data_type, cmd.data,
													cmd.min_filter, cmd.mag_filter);
										break;
									}

								case stream_type::texture_cube:
									{
										if (cmd.has_builtin_mipmaps)
											renderer::upload_texturecube_mm(cmd.texture_idx, cmd.width, cmd.height, cmd.format,
													cmd.internal_format, cmd.pixel_data_type, cmd.data,
													cmd.min_filter, cmd.mag_filter);
										else
											renderer::upload_texturecube(cmd.texture_idx, cmd.width, cmd.height, cmd.format,
													cmd.internal_format, cmd.pixel_data_type, cmd.data,
													cmd.min_filter, cmd.mag_filter);
										break;
									}
								
								case stream_type::geometry:
									{
										renderer::upload_surface(cmd.surface_idx, cmd.vertices, cmd.indices,
												cmd.vcount, cmd.icount, cmd.stride, cmd.draw_type);
										break;
									}

								case stream_type::shader:
									{
										renderer::compile_shader(cmd.shader_idx, cmd.vertex_str, cmd.fragment_str, cmd.shader_param_list);
										break;
									}

								default:
									break;
							}
							break;
						}

					case command::stream_data:
						{
							stream_command cmd;
							gpuCmd.serialize(cmd);
							switch (cmd.data_type) {
								case stream_type::geometry:
									{
										renderer::update_surface(cmd.surface_idx, cmd.vertices, cmd.indices,
												cmd.vcount, cmd.icount);
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
							break;
						}

					case command::present_render:
						{
							swapThisRenderPass = true;
							break;
						}

					case command::invalid:
						{
							break;
						}

					default:
						break;
				}
			}

			// geometry render phase
			detail::internal_render_surfaces<t_surface_list>();
			detail::internal_clear_buffer<t_surface_list>(detail::s_front_cmdbuff);
			detail::s_generic_command_buffer[detail::s_front_cmdbuff].empty();

			detail::s_front_cmdbuff = (detail::s_front_cmdbuff + 1) % BUFFERED_FRAMES;
			if (swapThisRenderPass) {
				PROFILE_SCOPE(SwapBuffers);
				swap_buffers();
				detail::s_waiting_for_swap = false;
				g_debug_global_counters.rendered_frames++;
			}
		}
	}

	template <typename t_surface_list>
	void initialize_render_thread()
	{
		FLORAL_ASSERT_MSG(sizeof(init_command) <= COMMAND_PAYLOAD_SIZE, "Command exceeds payload's capacity!");
		FLORAL_ASSERT_MSG(sizeof(render_command) <= COMMAND_PAYLOAD_SIZE, "Command exceeds payload's capacity!");
		FLORAL_ASSERT_MSG(sizeof(load_command) <= COMMAND_PAYLOAD_SIZE, "Command exceeds payload's capacity!");
		FLORAL_ASSERT_MSG(sizeof(render_state_toggle_command) <= COMMAND_PAYLOAD_SIZE, "Command exceeds payload's capacity!");

		g_global_counters.current_render_frame_idx = 0;
		g_global_counters.current_submit_frame_idx = 0;
		g_debug_global_counters.submitted_frames = 0;
		g_debug_global_counters.rendered_frames = 0;

		// generic buffer init
		for (u32 i = 0; i < BUFFERED_FRAMES; i++)
			detail::s_generic_command_buffer[i].init(g_renderer_settings.generic_command_buffer_size, &g_persistance_allocator);
		// draw buffer init
		detail::internal_init_buffer<t_surface_list>(&g_persistance_allocator);

		for (u32 i = 0; i < BUFFERED_FRAMES; i++)
			detail::s_gpu_frame_allocator[i] = g_persistance_allocator.allocate_arena<arena_allocator_t>(
					SIZE_MB(g_renderer_settings.frame_allocator_size_mb));

		detail::s_materials.init(32, &g_persistance_allocator);
		g_debug_global_counters.materials_cap = detail::s_materials.get_size();

		detail::s_front_cmdbuff = 0;
		detail::s_back_cmdbuff = 2;
		detail::s_render_state_changelog = 0;
		detail::s_waiting_for_swap = true;

		g_render_thread.entry_point = &insigne::render_thread_func<t_surface_list>;
		g_render_thread.ptr_data = nullptr;
		g_render_thread.start();
	}

	// -----------------------------------------
	template <typename t_surface>
	void push_draw_command(const render_command& i_cmd)
	{
		gpu_command newCmd;
		newCmd.opcode = command::draw_geom;
		newCmd.deserialize(i_cmd);
		detail::draw_command_buffer_t<t_surface>::command_buffer[detail::s_back_cmdbuff].push_back(newCmd);

		g_debug_frame_counters.num_render_commands++;
	}
	// -----------------------------------------
	template <typename t_surface>
	void set_scissor_test(const bool i_enable /* = false */, const s32 i_x /* = 0 */, const s32 i_y /* = 0 */, const s32 i_width /* = 0 */, const s32 i_height /* = 0 */)
	{
		scissor_state_command stateCommand;
		stateCommand.x = i_x;
		stateCommand.y = i_y;
		stateCommand.width = i_width;
		stateCommand.height = i_height;
		stateCommand.scissor_test = i_enable;

		gpu_command newCmd;
		newCmd.opcode = command::setup_scissor_state;
		newCmd.deserialize(stateCommand);
		detail::draw_command_buffer_t<t_surface>::command_buffer[detail::s_back_cmdbuff].push_back(newCmd);
	}

	template <typename t_surface>
	void draw_surface(const surface_handle_t i_surfaceHdl, const material_handle_t i_matHdl)
	{
		material_t* matSnapshot = detail::s_gpu_frame_allocator[detail::s_back_cmdbuff]->allocate<material_t>();
		(*matSnapshot) = detail::s_materials[static_cast<s32>(i_matHdl)];

		render_command cmd;
		cmd.material_snapshot = matSnapshot;
		cmd.segment_offset = 0;
		cmd.surface_handle = i_surfaceHdl;
		cmd.segment_size = -1;

		push_draw_command<t_surface>(cmd);
	}
	
	template <typename t_surface>
	void draw_surface_segmented(const surface_handle_t i_surfaceHdl, const material_handle_t i_matHdl,
			const s32 i_segSize, const voidptr i_segOffset)
	{
		material_t* matSnapshot = detail::s_gpu_frame_allocator[detail::s_back_cmdbuff]->allocate<material_t>();
		(*matSnapshot) = detail::s_materials[static_cast<s32>(i_matHdl)];

		render_command cmd;
		cmd.material_snapshot = matSnapshot;
		cmd.segment_offset = i_segOffset;
		cmd.surface_handle = i_surfaceHdl;
		cmd.segment_size = i_segSize;

		push_draw_command<t_surface>(cmd);
	}
}
