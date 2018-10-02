namespace insigne {

inline const s32 get_material_texture_slot(const material_desc_t& i_mat, const_cstr i_id)
{
	floral::crc_string idToSearch(i_id);
	for (u32 i = 0; i < i_mat.textures.get_size(); i++)
		if (i_mat.textures[i].name == idToSearch)
			return i;
	return -1;
}

inline const s32 get_material_uniform_block_slot(const material_desc_t& i_mat, const_cstr i_id)
{
	floral::crc_string idToSearch(i_id);
	for (u32 i = 0; i < i_mat.unform_blocks.get_size(); i++)
		if (i_mat.uniform_blocks[i].name == idToSearch)
			return i;
	return -1;
}

}
