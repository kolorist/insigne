#include "insigne/stream.h"

#include "insigne/context.h"
#include "insigne/driver.h"
#include "insigne/buffers.h"
#include "insigne/memory.h"

#include "insigne/generated_code/proxy.h"

#include <stdio.h>

namespace insigne {
	// -----------------------------------------
	//typedef floral::fixed_array<geometry_stream_command, linear_allocator_t>	geometry_stream_command_buffer_t;
	typedef floral::fixed_array<texture_stream_command, linear_allocator_t>		texture_stream_command_buffer_t;
	typedef floral::ring_buffer_mt<geometry_stream_command, linear_allocator_t, 1024u>	geometry_stream_command_buffer_t;
	
	static geometry_stream_command_buffer_t		s_geom_stream_command_buffer;
	static texture_stream_command_buffer_t		s_tex_stream_command_buffer;

	void stream_thread_func(voidptr i_data)
	{
		while(true)
		{
			geometry_stream_command cmd = s_geom_stream_command_buffer.wait_and_pop();
		}
	}

	// -----------------------------------------
	void initialize_stream_thread()
	{
		s_geom_stream_command_buffer.assign_allocator(&g_persistance_allocator);

		g_stream_thread.entry_point = &insigne::stream_thread_func;
		g_stream_thread.ptr_data = nullptr;
		g_stream_thread.start();

	}

	void push_command(geometry_stream_command& i_cmd)
	{
		s_geom_stream_command_buffer.push(i_cmd);
	}
}
