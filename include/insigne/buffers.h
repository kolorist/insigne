#pragma once

#include <floral.h>

#include <string.h>

#include "buffers.h"

namespace insigne {

#define COMMAND_PAYLOAD_SIZE					128
#define GPU_COMMAND_BUFFER_SIZE					1024u

	enum class command {
		invalid = 0,
		setup_init_state,
		setup_render_state,
		setup_framebuffer,
		draw_geom,
		stream_data
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
	struct init_command {
		floral::vec4f							clear_color;
	};

	struct state_command {
		bool									depth_test;
		bool									depth_write;
	};

	struct framebuffer_command {
		bool									clear_color_buffer;
		bool									clear_depth_buffer;
	};

	struct render_command {
		surface_handle_t						surface_handle;
		shader_handle_t							shader_handle;
	};

	enum class stream_type {
		invalid = 0,
		texture,
		geometry,
		shader
	};
	struct stream_command {
		stream_type								data_type;
		union {
			// texture
			struct {
				voidptr							data;
				texture_format_e				format;
			};

			// geom
			struct {
				voidptr							vertices;
				voidptr							indices;
				s32								stride;
				u32								vcount;
				u32								icount;
				u32								surface_idx;
				bool							has_indices;
			};

			// shader
			struct {
				const_cstr						vertex_str;
				const_cstr						fragment_str;
				u32								shader_idx;
			};
		};
	};

}
