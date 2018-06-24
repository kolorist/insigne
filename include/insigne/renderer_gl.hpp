#include "detail/renderer_states_gl.h"

#include <lotus/profiler.h>

namespace insigne {
namespace renderer {

	template <typename t_surface>
	void draw_surface_idx(const surface_handle_t& i_surfaceHdl, const material_t& i_matSnapshot,
			const s32 i_segSize, const voidptr i_segOffset)
	{
		PROFILE_SCOPE(draw_surface_idx);

		detail::surface surf = detail::s_surfaces[i_surfaceHdl];
		detail::shader& shdr = detail::s_shaders[i_matSnapshot.shader_handle];

		s32 iCount = 0;
		if (i_segSize < 0) iCount = surf.icount; else iCount = i_segSize;

		{
			PROFILE_SCOPE(update_shader);
			pxUseProgram(shdr.gpu_handle);
			// setup material
			for (u32 i = 0; i < i_matSnapshot.float_params.get_size(); i++)
				pxUniform1f(shdr.float_params[i], i_matSnapshot.float_params[i].value);
			for (u32 i = 0; i < i_matSnapshot.vec3_params.get_size(); i++)
				pxUniform3fv(shdr.vec3_params[i], 1, &i_matSnapshot.vec3_params[i].value.x);
			for (u32 i = 0; i < i_matSnapshot.mat4_params.get_size(); i++)
				pxUniformMatrix4fv(shdr.mat4_params[i], 1, GL_FALSE, &(i_matSnapshot.mat4_params[i].value[0][0]));
			for (u32 i = 0; i < i_matSnapshot.texture2d_params.get_size(); i++) {
				detail::texture& tex = detail::s_textures[static_cast<s32>(i_matSnapshot.texture2d_params[i].value)];
				pxActiveTexture(GL_TEXTURE0 + i);
				pxBindTexture(GL_TEXTURE_2D, tex.gpu_handle);
				pxUniform1i(shdr.texture2d_params[i], i);
			}
		}

		// draw
		pxBindBuffer(GL_ARRAY_BUFFER, surf.vbo);
		pxBindBuffer(GL_ELEMENT_ARRAY_BUFFER, surf.ibo);
		t_surface::describe_vertex_data();
		{
			PROFILE_SCOPE(issue_draw_call);
			pxDrawElements(GL_TRIANGLES, iCount, GL_UNSIGNED_INT, i_segOffset);
		}
	}

}
}
