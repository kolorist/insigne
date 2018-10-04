#include <lotus/profiler.h>

namespace insigne {

template <typename>
struct draw_command_buffer_t;

namespace detail {

// ---------------------------------------------
template <typename t_surface>
void render_interface_t<t_surface>::render(const size i_cmdBuffId)
{
	detail::gpu_command_buffer_t& cmdbuff = detail::draw_command_buffer_t<t_surface>::command_buffer[i_cmdBuffId];
	for (u32 i = 0; i < cmdbuff.get_size(); i++) {
		gpu_command& gpuCmd = cmdbuff[i];
		gpuCmd.reset_cursor();
		switch (gpuCmd.opcode) {
			case command::draw_command:
				{
					draw_command_t cmd;
					gpuCmd.serialize(cmd);
					detail::draw_indexed_surface<t_surface>(cmd.vb_handle, cmd.ib_handle, cmd.material_snapshot,
							cmd.segment_size, cmd.segment_offset);
					break;
				}
			default:
				break;
		}
	}
}

template <typename t_surface>
void render_interface_t<t_surface>::init_buffer(insigne::linear_allocator_t* i_allocator)
{
	using namespace insigne;
	for (u32 i = 0; i < BUFFERS_COUNT; i++)
		detail::draw_command_buffer_t<t_surface>::command_buffer[i].init(g_settings.draw_command_buffer_size, &g_persistance_allocator);
}
// ---------------------------------------------

// rendering
template <int t_n, typename t_surface_list>
struct internal_surface_iterator {
	static void setup_states_and_render(const size i_cmdBuffId)
	{
		typedef tl_type_at_t<t_n, t_surface_list> surface_type_t;
		surface_type_t::setup_states();
		surface_type_t::render(i_cmdBuffId);
		internal_surface_iterator<t_n - 1, t_surface_list>::setup_states_and_render(i_cmdBuffId);
	}
};

template <typename t_surface_list>
struct internal_surface_iterator<0, t_surface_list> {
	static void setup_states_and_render(const size i_cmdBuffId)
	{
		typedef tl_type_at_t<0, t_surface_list> surface_type_t;
		surface_type_t::setup_states();
		surface_type_t::render(i_cmdBuffId);
	}
};

template <typename t_surface_list>
void internal_render_surfaces(const size i_cmdBuffId)
{
	internal_surface_iterator<tl_length<t_surface_list>::value - 1, t_surface_list>::setup_states_and_render(i_cmdBuffId);
}

// init gpu command buffer
template <int t_n, typename t_surface_list>
struct internal_buffer_iterator {
	static void init_buffer(linear_allocator_t *i_allocator)
	{
		typedef tl_type_at_t<t_n, t_surface_list> surface_type_t;
		surface_type_t::init_buffer(i_allocator);
		internal_buffer_iterator<t_n - 1, t_surface_list>::init_buffer(i_allocator);
	}

	static void clear_buffer(size idx)
	{
		typedef tl_type_at_t<t_n, t_surface_list> surface_type_t;
		draw_command_buffer_t<surface_type_t>::command_buffer[idx].empty();
		internal_buffer_iterator<t_n - 1, t_surface_list>::clear_buffer(idx);
	}
};

template <typename t_surface_list>
struct internal_buffer_iterator<0, t_surface_list> {
	static void init_buffer(linear_allocator_t *i_allocator)
	{
		typedef tl_type_at_t<0, t_surface_list> surface_type_t;
		surface_type_t::init_buffer(i_allocator);
	}

	static void clear_buffer(size idx)
	{
		typedef tl_type_at_t<0, t_surface_list> surface_type_t;
		draw_command_buffer_t<surface_type_t>::command_buffer[idx].empty();
	}
};

template <typename t_surface_list>
void internal_init_buffer(linear_allocator_t *i_allocator)
{
	internal_buffer_iterator<tl_length<t_surface_list>::value - 1, t_surface_list>::init_buffer(i_allocator);
}

template <typename t_surface_list>
void internal_clear_buffer(size idx)
{
	internal_buffer_iterator<tl_length<t_surface_list>::value - 1, t_surface_list>::clear_buffer(idx);
}

// ---------------------------------------------
template <typename t_surface_list>
void initialize_render_module()
{
	internal_init_buffer<t_surface_list>(&g_persistance_allocator);

	// create default framebuffer desc
	
}

template <typename t_surface_list>
void process_draw_command_buffer(const size i_cmdBuffId)
{
	// geometry render phase
	detail::internal_render_surfaces<t_surface_list>(i_cmdBuffId);
	detail::internal_clear_buffer<t_surface_list>(i_cmdBuffId);
}

// ---------------------------------------------
template <typename t_surface>
void draw_indexed_surface(const vb_handle_t i_vb, const ib_handle_t i_ib, const material_desc_t* i_mat,
		const u32 i_segSize, const voidptr i_segOffset)
{
	const insigne::detail::vbdesc_t& vbDesc = insigne::detail::g_vbs_pool[s32(i_vb)];
	const insigne::detail::ibdesc_t& ibDesc = insigne::detail::g_ibs_pool[s32(i_ib)];
	const insigne::detail::shader_desc_t& shaderDesc = insigne::detail::g_shaders_pool[s32(i_mat->shader_handle)];

	pxUseProgram(shaderDesc.gpu_handle);

	for (u32 i = 0; i < i_mat->uniform_blocks.get_size(); i++) {
		const insigne::detail::ubdesc_t& ubDesc = insigne::detail::g_ubs_pool[s32(i_mat->uniform_blocks[i].value)];
		pxBindBufferBase(GL_UNIFORM_BUFFER, i, ubDesc.gpu_handle);
		pxUniformBlockBinding(shaderDesc.gpu_handle, shaderDesc.slots_config.uniform_blocks[i], i);
	}

	// draw
	static GLenum s_geometryMode[] = {
		GL_POINTS,
		GL_LINE_STRIP,
		GL_LINE_LOOP,
		GL_LINES,
		GL_TRIANGLE_STRIP,
		GL_TRIANGLE_FAN,
		GL_TRIANGLES
	};

	pxBindBuffer(GL_ARRAY_BUFFER, vbDesc.gpu_handle);
	pxBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibDesc.gpu_handle);
	t_surface::describe_vertex_data();
	{
		pxDrawElements(s_geometryMode[s32(t_surface::s_geometry_mode)], ibDesc.count, GL_UNSIGNED_INT, 0);
	}
}

}
}
