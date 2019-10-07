#include "insigne/detail/rt_buffers.h"

#include "insigne/memory.h"
#include "insigne/generated_code/proxy.h"
#include "insigne/internal_states.h"

#include <clover/Logger.h>

/*
 * NOTE: we will not make shadow copy of vertex or index data for the command buffer for now
 * because it will consume too much memory and the copy operation may take a very long time with huge data
 * Thus, the user must be ensure that their data persists at least BUFFERS_COUNT frames
 * after doing an upload_vb() or update_vb() call
 */

namespace insigne {
namespace detail {

vbs_pool_t										g_vbs_pool;
ibs_pool_t										g_ibs_pool;
ubs_pool_t										g_ubs_pool;

// ---------------------------------------------
inline detail::gpu_command_buffer_t& get_buffers_command_buffer(const size i_cmdBuffId) {
	return detail::g_buffers_command_buffer[i_cmdBuffId];
}

static GLenum s_buffer_usage[] = {
	GL_STATIC_DRAW,
	GL_DYNAMIC_DRAW,
	GL_STREAM_DRAW };
// ---------------------------------------------
/* ut */
const vb_handle_t create_vb(const insigne::vbdesc_t& i_desc)
{
	ssize idx = g_vbs_pool.get_size();
	g_vbs_pool.push_back(vbdesc_t());

	return vb_handle_t(idx);
}

const vb_handle_t get_last_vb()
{
	return g_vbs_pool.get_size() - 1;
}

const vbdesc_t& get_vb_desc(const vb_handle_t i_hdl)
{
	return g_vbs_pool[i_hdl];
}

/* ut */
const ib_handle_t create_ib(const insigne::ibdesc_t& i_desc)
{
	ssize idx = g_ibs_pool.get_size();
	g_ibs_pool.push_back(ibdesc_t());

	return ib_handle_t(idx);
}

const ib_handle_t get_last_ib()
{
	return g_ibs_pool.get_size() - 1;
}

const ibdesc_t& get_ib_desc(const ib_handle_t i_hdl)
{
	return g_ibs_pool[i_hdl];
}

/* ut */
const ub_handle_t create_ub(const insigne::ubdesc_t& i_desc)
{
	ssize idx = g_ubs_pool.get_size();
	g_ubs_pool.push_back(ubdesc_t());

	return ub_handle_t(idx);
}

const ub_handle_t get_last_ub()
{
	return g_ubs_pool.get_size() - 1;
}

void upload_vb(const vb_handle_t i_hdl, const insigne::vbdesc_t& i_desc)
{
	vbdesc_t& desc = g_vbs_pool[s32(i_hdl)];

	desc.region_size = i_desc.region_size;
	desc.stride = i_desc.stride;
	desc.count = i_desc.count;
	desc.usage = i_desc.usage;

	GLuint vboHandle;
	GLenum bufferUsage = s_buffer_usage[s32(desc.usage)];

	pxGenBuffers(1, &vboHandle);
	pxBindBuffer(GL_ARRAY_BUFFER, vboHandle);
	// TODO: we can do full pxBufferData if the data is exactly matched with region_size
	// invalidate buffer
	pxBufferData(GL_ARRAY_BUFFER, GLsizeiptr(desc.region_size), nullptr, bufferUsage);
	// upload data right away if needed
	if (i_desc.data) {
		pxBufferSubData(GL_ARRAY_BUFFER, GLintptr(0), GLsizeiptr(desc.stride * desc.count), (const GLvoid*)i_desc.data);
	}
	pxBindBuffer(GL_ARRAY_BUFFER, 0);

	desc.gpu_handle = vboHandle;
}

void upload_ib(const ib_handle_t i_hdl, const insigne::ibdesc_t& i_desc)
{
	ibdesc_t& desc = g_ibs_pool[s32(i_hdl)];

	desc.region_size = i_desc.region_size;
	desc.count = i_desc.count;
	desc.usage = i_desc.usage;

	GLuint iboHandle;
	GLenum bufferUsage = s_buffer_usage[s32(desc.usage)];

	pxGenBuffers(1, &iboHandle);
	pxBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboHandle);
	// TODO: we can do full pxBufferData if the data is exactly matched with region_size
	// invalidate buffer
	pxBufferData(GL_ELEMENT_ARRAY_BUFFER, GLsizeiptr(desc.region_size), nullptr, bufferUsage);
	// upload data right away if needed
	if (i_desc.data) {
		pxBufferSubData(GL_ELEMENT_ARRAY_BUFFER, GLintptr(0), GLsizeiptr(sizeof(s32) * desc.count), (const GLvoid*)i_desc.data);
	}
	pxBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	desc.gpu_handle = iboHandle;
}

void upload_ub(const ub_handle_t i_hdl, const insigne::ubdesc_t& i_desc)
{
	ubdesc_t& desc = g_ubs_pool[s32(i_hdl)];

	desc.region_size = i_desc.region_size;
	desc.data_size = i_desc.data_size;
	desc.usage = i_desc.usage;

	GLuint uboHandle;
	GLenum bufferUsage = s_buffer_usage[s32(desc.usage)];

	pxGenBuffers(1, &uboHandle);
	pxBindBuffer(GL_UNIFORM_BUFFER, uboHandle);
	// TODO: we can do full pxBufferData if the data is exactly matched with region_size
	// invalidate buffer
	pxBufferData(GL_UNIFORM_BUFFER, GLsizeiptr(desc.region_size), nullptr, bufferUsage);
	// upload data right away if needed
	if (i_desc.data) {
		pxBufferSubData(GL_UNIFORM_BUFFER, GLintptr(0), GLsizeiptr(desc.data_size), (const GLvoid*)i_desc.data);
	}
	pxBindBuffer(GL_UNIFORM_BUFFER, 0);

	desc.gpu_handle = uboHandle;
}

void stream_vb_data(const vb_handle_t i_hdl, const voidptr i_data, const u32 i_vcount, const u32 i_offsetElems)
{
	vbdesc_t& desc = g_vbs_pool[s32(i_hdl)];

	desc.count = i_vcount;

	pxBindBuffer(GL_ARRAY_BUFFER, desc.gpu_handle);
	pxBufferSubData(GL_ARRAY_BUFFER, GLintptr(i_offsetElems * desc.stride), GLsizeiptr(desc.stride * desc.count), (const GLvoid*)i_data);
	pxBindBuffer(GL_ARRAY_BUFFER, 0);
}

void stream_ib_data(const ib_handle_t i_hdl, const voidptr i_data, const u32 i_icount, const u32 i_offsetElems)
{
	ibdesc_t& desc = g_ibs_pool[s32(i_hdl)];

	desc.count = i_icount;

	pxBindBuffer(GL_ELEMENT_ARRAY_BUFFER, desc.gpu_handle);
	pxBufferSubData(GL_ELEMENT_ARRAY_BUFFER, GLintptr(i_offsetElems * sizeof(s32)), GLsizeiptr(sizeof(s32) * desc.count), (const GLvoid*)i_data);
	pxBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void stream_ub_data(const ub_handle_t i_hdl, const voidptr i_data, const size i_size, const size i_offset)
{
	ubdesc_t& desc = g_ubs_pool[s32(i_hdl)];

	desc.data_size = i_size;

	pxBindBuffer(GL_UNIFORM_BUFFER, desc.gpu_handle);
	pxBufferSubData(GL_UNIFORM_BUFFER, GLintptr(i_offset), GLsizeiptr(desc.data_size), (const GLvoid*)i_data);
	pxBindBuffer(GL_UNIFORM_BUFFER, 0);
}

static void clean_up_snapshot(const vb_handle_t i_vb, const ib_handle_t i_ib, const ub_handle_t i_ub)
{
	CLOVER_VERBOSE("Cleaning up buffers snapshot...");
	while (i_vb != g_vbs_pool.get_size() - 1)
	{
		vbdesc_t vbDesc = g_vbs_pool.pop_back();
		CLOVER_VERBOSE("Deleting vertex buffer id %d : %zd bytes", vbDesc.gpu_handle, vbDesc.region_size);
		pxDeleteBuffers(1, &vbDesc.gpu_handle);
	}
	while (i_ib != g_ibs_pool.get_size() - 1)
	{
		ibdesc_t ibDesc = g_ibs_pool.pop_back();
		CLOVER_VERBOSE("Deleting index buffer id %d : %zd bytes", ibDesc.gpu_handle, ibDesc.region_size);
		pxDeleteBuffers(1, &ibDesc.gpu_handle);
	}
	while (i_ub != g_ubs_pool.get_size() - 1)
	{
		ubdesc_t ubDesc = g_ubs_pool.pop_back();
		CLOVER_VERBOSE("Deleting uniform buffer id %d : %zd bytes", ubDesc.gpu_handle, ubDesc.region_size);
		pxDeleteBuffers(1, &ubDesc.gpu_handle);
	}
	CLOVER_VERBOSE("Finished cleaning up buffers snapshot");
}

// ---------------------------------------------
void cleanup_buffers_module()
{
	CLOVER_VERBOSE("Cleaning up buffers module...");
	for (ssize i = 0; i < g_vbs_pool.get_size(); i++)
	{
		vbdesc_t& vbDesc = g_vbs_pool[i];
		CLOVER_VERBOSE("Deleting vertex buffer id %d : %zd bytes", vbDesc.gpu_handle, vbDesc.region_size);
		pxDeleteBuffers(1, &vbDesc.gpu_handle);
	}
	CLOVER_VERBOSE("Free %zd vertex buffers", g_vbs_pool.get_size());
	for (ssize i = 0; i < g_ibs_pool.get_size(); i++)
	{
		ibdesc_t& ibDesc = g_ibs_pool[i];
		CLOVER_VERBOSE("Deleting index buffer id %d : %zd bytes", ibDesc.gpu_handle, ibDesc.region_size);
		pxDeleteBuffers(1, &ibDesc.gpu_handle);
	}
	CLOVER_VERBOSE("Free %zd index buffers", g_ibs_pool.get_size());
	for (ssize i = 0; i < g_ubs_pool.get_size(); i++)
	{
		ubdesc_t& ubDesc = g_ubs_pool[i];
		CLOVER_VERBOSE("Deleting uniform buffer id %d : %zd bytes", ubDesc.gpu_handle, ubDesc.region_size);
		pxDeleteBuffers(1, &ubDesc.gpu_handle);
	}
	CLOVER_VERBOSE("Free %zd uniform buffers", g_ubs_pool.get_size());
	g_ubs_pool.~ubs_pool_t();
	g_ibs_pool.~ibs_pool_t();
	g_vbs_pool.~vbs_pool_t();
	CLOVER_VERBOSE("Finished cleaning up buffers module");
}

void process_buffers_command_buffer(const size i_cmdBuffId)
{
	detail::gpu_command_buffer_t& cmdbuff = get_buffers_command_buffer(i_cmdBuffId);
	for (u32 i = 0; i < cmdbuff.get_size(); i++) {
		gpu_command& gpuCmd = cmdbuff[i];
		gpuCmd.reset_cursor();

		switch (gpuCmd.opcode) {
			case command::buffers_command:
				{
					buffers_command_t cmd;
					gpuCmd.serialize(cmd);

					switch (cmd.command_type) {
						case buffers_command_type_e::create_vb_buffers:
							upload_vb(cmd.create_vb_data.vb_handle, cmd.create_vb_data.desc);
							break;
						case buffers_command_type_e::create_ib_buffers:
							upload_ib(cmd.create_ib_data.ib_handle, cmd.create_ib_data.desc);
							break;
						case buffers_command_type_e::create_ub_buffers:
							upload_ub(cmd.create_ub_data.ub_handle, cmd.create_ub_data.desc);
							break;
						case buffers_command_type_e::stream_vb_data:
							stream_vb_data(cmd.stream_vb_data.vb_handle, cmd.stream_vb_data.data,
									cmd.stream_vb_data.vcount, cmd.stream_vb_data.offset_elements);
							break;
						case buffers_command_type_e::stream_ib_data:
							stream_ib_data(cmd.stream_ib_data.ib_handle, cmd.stream_ib_data.data,
									cmd.stream_ib_data.icount, cmd.stream_ib_data.offset_elements);
							break;
						case buffers_command_type_e::stream_ub_data:
							stream_ub_data(cmd.stream_ub_data.ub_handle, cmd.stream_ub_data.data,
									cmd.stream_ub_data.data_size, cmd.stream_ub_data.offset);
							break;
						case buffers_command_type_e::clean_up_snapshot:
							clean_up_snapshot(cmd.clean_up_snapshot_data.downto_vb,
									cmd.clean_up_snapshot_data.downto_ib,
									cmd.clean_up_snapshot_data.downto_ub);
							break;
						default:
							break;
					}

					break;
				}

			default:
				break;
		}
	}

	cmdbuff.empty();
}

}
}
