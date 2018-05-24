namespace insigne {
namespace renderer {

	template <typename t_surface>
	void draw_surface_idx(const surface_handle_t& i_surfaceHdl, const material_t& i_matSnapshot,
			const s32 i_segSize, const voidptr i_segOffset)
	{
		surface surf = s_surfaces[i_surfaceHdl];
		shader& shdr = s_shaders[i_matSnapshot.shader_handle];

		s32 iCount = 0;
		if (i_segSize < 0) iCount = surf.icount; else iCount = i_segSize;

		pxUseProgram(shdr.gpu_handle);
		// setup material
		for (u32 i = 0; i < i_matSnapshot.float_params.get_size(); i++)
			pxUniform1f(shdr.float_params[i], i_matSnapshot.float_params[i].value);
		for (u32 i = 0; i < i_matSnapshot.vec3_params.get_size(); i++)
			pxUniform3fv(shdr.vec3_params[i], 1, &i_matSnapshot.vec3_params[i].value.x);
		for (u32 i = 0; i < i_matSnapshot.mat4_params.get_size(); i++)
			pxUniformMatrix4fv(shdr.mat4_params[i], 1, GL_FALSE, &(i_matSnapshot.mat4_params[i].value[0][0]));
		for (u32 i = 0; i < i_matSnapshot.texture2d_params.get_size(); i++) {
			texture& tex = s_textures[static_cast<s32>(i_matSnapshot.texture2d_params[i].value)];
			pxActiveTexture(GL_TEXTURE0 + i);
			pxBindTexture(GL_TEXTURE_2D, tex.gpu_handle);
			pxUniform1i(shdr.texture2d_params[i], i);
		}

		// draw
		pxBindBuffer(GL_ARRAY_BUFFER, surf.vbo);
		pxBindBuffer(GL_ELEMENT_ARRAY_BUFFER, surf.ibo);
		t_surface::describe_vertex_data();
		pxDrawElements(GL_TRIANGLES, iCount, GL_UNSIGNED_INT, i_segOffset);
	}

}
}
