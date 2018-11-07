#pragma once

#include <floral.h>

#include <string.h>

#include "commons.h"
#include "internal_commons.h"

namespace insigne {

#define COMMAND_PAYLOAD_SIZE					64
#define GPU_COMMAND_BUFFER_SIZE					1024u

enum class command {
	invalid = 0,
	shading_command,
	buffers_command,
	textures_command,
	render_command,
	draw_command
};

// this struct is copyable
struct gpu_command {
	command									opcode;

	u8										payload[COMMAND_PAYLOAD_SIZE];
	size									pos;
	size									psize;

	gpu_command()
		: opcode(command::invalid)
		, pos(0)
		, psize(0)
	{ }

	template <class t_target>
	void deserialize(const t_target& i_target)
	{
		size dataSize = sizeof(t_target);
		memcpy(&payload[pos], &i_target, dataSize);
		pos += dataSize;
		psize += dataSize;
	}

	template <class t_target>
	void serialize(t_target& o_target)
	{
		size dataSize = sizeof(t_target);
		memcpy(&o_target, &payload[pos], dataSize);
		pos += dataSize;
	}

	void empty()							{ psize = 0; }
	void reset_cursor()						{ pos = 0; }
};

// ------------payload----------------------
namespace detail {
// draw-----------------------------------------
typedef floral::fixed_array<gpu_command, linear_allocator_t>	gpu_command_buffer_t;
}

}
