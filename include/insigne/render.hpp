namespace insigne {
	template <class t_param_type>
	const param_id get_material_param(const material_handle_t i_hdl, const_cstr i_name)
	{
		return param_id(-1);
	}

	template <class t_param_type>
	void set_material_param(const material_handle_t i_hdl, const param_id i_paramIdx, const t_param_type& i_value)
	{
	}
}
