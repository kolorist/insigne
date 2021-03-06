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
// create shader (user thread)
	insigne::shader_desc_t desc = insigne::create_shader_desc();
	desc.vs_path = <floral::path to vs> // debug purpose
	desc.fs_path = <floral::path to fs> // debug purpose

	{
		insigne::shader_param_t sparam;
		sparam.name = <sampler2D_shader_id>;
		desc.reflection.textures->push_back(sparam);
	}

	{
		insigne::shader_param_t sparam;
		sparam.name = <uniform_buffer_shader_id>;
		desc.reflection.uniform_buffers->push_back(sparam);
	}

	desc.vs = <vs string data, zero terminated>
	desc.fs = <fs string data, zero terminated>

	insigne::shader_handle_t newShader = insigne::create_shader(desc);

	// infuse material (user thread)
	insigne::material_desc_t newMaterial;
	insigne::infuse_material(newShader, newMaterial);

// ---------------------------------------------
// create vertex buffer (user thread)
	insigne::vbdesc_t desc;
	desc.region_size = <buffer_size_in_bytes>;
	desc.stride = sizeof(<element_type>);
	desc.count = <number_of_elements>;
	desc.data = <pointer_to_data>; // must not be nullptr for static buffer
	desc.usage = insigne::buffer_usage_e::dynamic; // or insigne::buffer_usage_e::static or insigne::buffer_usage_e::stream
	insigne::vb_handle_t vbHdl = insigne::create_vb(desc);

	// inner impl (user thread)
	vb_handle_t request_new_vb_handle(const insigne::vbdesc_t& i_desc);
	// inner impl (render thread)
	
	// inner impl (shared)
	floral::dynamic_array_stricted<detail::vbdesc_t, FreelistAllocator> s_vb_pool;

// create index buffer
	insigne::ibdesc_t desc;
	desc.region_size = <buffer_size_in_bytes>;
	desc.count = <number_of_elements>;
	desc.data = <pointer_to_data>; // must not be nullptr for static buffer
	desc.usage = insigne::buffer_usage_e::dynamic; // or insigne::buffer_usage_e::static or insigne::buffer_usage_e::stream
	insigne::ib_handle_t ibHdl = insigne::create_ib(desc);
// update vertex buffer
	insigne::update_vb(<handle_of_vb>, <pointer_to_data>, <number_of_elements>, <offset_in_elements_count> = 0);
// update index buffer
	insigne::update_ib(<handle_of_ib>, <pointer_to_data>, <number_of_indices>, <offset_in_indices_count> = 0);

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
