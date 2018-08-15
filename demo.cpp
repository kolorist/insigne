/*
 * What does 'insigne' do?
 * initialize rendering context (after acquiring drawing surface)
 * 	rendering api errors checking proxy
 * 	generalize drawing commands: draw calls, resource upload / update, ...
 * 	triangle-level optimization: batching, culling, ...
 *
 */

/*
 * How to add a shader param list
 * include/insigne/internal_commons.h
 *	add to material_template_t
 */

//----------------------------------------------

//----------------------------------------------
// How to upload static geometry
	insigne::surface_handle_t shdl = insigne::upload_surface(
		&(*vertices)[0],
		sizeof(Vertex) * vertices->get_size(),
		&(*indices)[0],
		sizeof(u32) * indices->get_size(),
		sizeof(Vertex),
		vertices->get_size(),
		indices->get_size());

// How to upload streamed / dynamic geometry
	insigne::surface_handle_t shdl = insigne::create_streamed_surface(
		sizeof(Vertex));

	update_streamed_surface(
		shdl,
		&(*vertices)[0],
		sizeof(Vertex) * vertices->get_size(),
		&(*indices)[0],
		sizeof(u32) * indices->get_size(),
		vertices->get_size(),
		indices->get_size());

// How to upload texture 2d
	insigne::texture_handle_t texHdl = insigne::create_texture2d(
		width,
		height,
		insigne::texture_format_e::rgb,
		insigne::filtering_e::linear_mipmap_linear,
		insigne::filtering_e::linear,
		dataSize,
		texData,
		true);
	dataStream.read_bytes((p8)texData, dataSize);

// How to upload texture cube
	insigne::texture_handle_t texHdl = insigne::create_texturecube(
		width,
		height,
		insigne::texture_format_e::hdr_rgb,
		insigne::filtering_e::linear,
		insigne::filtering_e::linear,
		dataSizeOneFace,
		texData,
		false);
	dataStream.read_bytes((p8)texData, dataSizeOneFace * 6);

// How to create shader
	// allocate a param list
	insigne::shader_param_list_t* paramList = insigne::allocate_shader_param_list(8);
	// vertex shader
	paramList->push_back(insigne::shader_param_t("iu_PerspectiveWVP", insigne::param_data_type_e::param_mat4));
	paramList->push_back(insigne::shader_param_t("iu_TransformMat", insigne::param_data_type_e::param_mat4));

	// fragment shader
	paramList->push_back(insigne::shader_param_t("iu_TexBaseColor", insigne::param_data_type_e::param_sampler2d));
	paramList->push_back(insigne::shader_param_t("iu_TexMetallic", insigne::param_data_type_e::param_sampler2d));
	paramList->push_back(insigne::shader_param_t("iu_TexRoughness", insigne::param_data_type_e::param_sampler2d));
	paramList->push_back(insigne::shader_param_t("iu_LightDirection", insigne::param_data_type_e::param_vec3));
	paramList->push_back(insigne::shader_param_t("iu_LightIntensity", insigne::param_data_type_e::param_vec3));	

	paramList->push_back(insigne::shader_param_t("iu_IrrMap", insigne::param_data_type_e::param_sampler_cube));
	paramList->push_back(insigne::shader_param_t("iu_SpecMap", insigne::param_data_type_e::param_sampler_cube));

	// read data to vertSource and fragSource
	insigne::shader_handle_t newShader = insigne::compile_shader(vertSource, fragSource, i_paramList);
	// the vertSource and fragSource can now be freed, they have been copied to buffer

// How to create material
	// after we have the shader with paramList load
	insigne::material_handle_t material = insigne::create_material(i_shaderHdl);

	// getting parameters
	insigne::param_id v1 = insigne::get_material_param<floral::mat4x4f>(m_MaterialHandle, "iu_PerspectiveWVP");
	insigne::param_id v2 = insigne::get_material_param<insigne::texture_handle_t>(m_MaterialHandle, "iu_TexMetallic");
	insigne::param_id v3 = insigne::get_material_param<floral::vec3f>(m_MaterialHandle, "iu_LightDirection");
	insigne::param_id v4 = insigne::get_material_param_texcube(m_MaterialHandle, "iu_IrrMap");

	// settings parameters
	insigne::set_material_param(material, v1, wvpMatrix);
	insigne::set_material_param_texcube(material, v4, cubeMapTexHandle);

// How to render with material
	insigne::draw_surface<SurfaceType>(surfaceHandle, materialHandle);

// Debugging information APIs
	//TODO
