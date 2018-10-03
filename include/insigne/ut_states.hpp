namespace insigne {

template <typename t_surface>
void set_scissor_test(const bool i_enable /* = false */, const s32 i_x /* = 0 */, const s32 i_y /* = 0 */,
		const s32 i_width /* = 0 */, const s32 i_height /* = 0 */)
{
	render_command_t stateCommand;
	stateCommand.scissor_state_setup_data.x = i_x;
	stateCommand.scissor_state_setup_data.y = i_y;
	stateCommand.scissor_state_setup_data.width = i_width;
	stateCommand.scissor_state_setup_data.height = i_height;
	stateCommand.scissor_state_setup_data.test_enable = i_enable;

	gpu_command newCmd;
	newCmd.opcode = command::render_command;
	newCmd.deserialize(stateCommand);
	detail::draw_command_buffer_t<t_surface>::command_buffer[detail::s_back_cmdbuff].push_back(newCmd);
}

}
