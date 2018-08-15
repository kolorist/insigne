# What does 'insigne' do?
* initialize rendering context (after acquiring drawing surface)
	- rendering thread
	- streamming thread
* rendering api errors checking proxy
* generalize drawing commands: draw calls, resource upload / update, ...
* triangle-level optimization: batching, culling, ...

# How to add a shader param list
- include/insigne/internal\_commons.h
	add to material_template_t

#######################################################################
# How to upload static geometry
```
insigne::surface_handle_t shdl = insigne::upload_surface(
		&(*vertices)[0],
		sizeof(Vertex) * vertices->get_size(),
		&(*indices)[0],
		sizeof(u32) * indices->get_size(),
		sizeof(Vertex),
		vertices->get_size(),
		indices->get_size());
```

# How to upload streamed / dynamic geometry
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

# How to upload texture 2d
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

# How to upload texture cube
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

# How to create shader

# How to create material

# How to render with material

# Debugging information APIs
