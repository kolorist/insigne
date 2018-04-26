#include "insigne/render.h"

#include "insigne/stream.h"
#include "insigne/context.h"
#include "insigne/driver.h"
#include "insigne/buffers.h"
#include "insigne/memory.h"

#include "insigne/generated_code/proxy.h"

#include <stdio.h>

namespace insigne {

	// -----------------------------------------
	typedef floral::fixed_array<render_command, linear_allocator_t>	render_command_buffer_t;

	static render_command_buffer_t				s_render_command_buffer;
	// -----------------------------------------
	static floral::condition_variable			s_init_condvar;
	static floral::mutex						s_init_mtx;
	static floral::condition_variable			s_cmdbuffer_condvar;
	static floral::mutex						s_cmdbuffer_mtx;

	void render_thread_func(voidptr i_data)
	{
		create_main_context();
		initialize_stream_thread();
		pxClearColor(0.3f, 0.4f, 0.5f, 1.0f);
		s_init_condvar.notify_one();
		while(true)
		{
			pxClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			/*
			s_cmdbuffer_condvar.wait(s_cmdbuffer_mtx);
			for (u32 i = 0; i < s_render_command_buffer.get_size(); i++) {
				// process
				geometry_stream_command cmd;
				push_command(cmd);
			}

			s_render_command_buffer.empty();
			*/
			swap_buffers();
		}
	}

	// -----------------------------------------
	void initialize_render_thread()
	{
		// TODO: remove hardcode
		s_render_command_buffer.init(1024u, &g_persistance_allocator);

		g_render_thread.entry_point = &insigne::render_thread_func;
		g_render_thread.ptr_data = &s_render_command_buffer;
		g_render_thread.start();
	}

	void wait_for_initialization()
	{
		s_init_condvar.wait(s_init_mtx);
	}

	// -----------------------------------------
	void begin_frame()
	{
	}

	void end_frame()
	{
	}

	void dispatch_frame()
	{
		s_cmdbuffer_condvar.notify_one();
	}

	// -----------------------------------------
	void push_command(render_command& i_cmd)
	{
		s_render_command_buffer.push_back(i_cmd);
	}

}
