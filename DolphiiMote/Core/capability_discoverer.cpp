// Copyright 2013 Max Malmgren

// This file is part of DolphiiMote.

// DolphiiMote is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// DolphiiMote is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with DolphiiMote.  If not, see <http://www.gnu.org/licenses/>.

#include "capability_discoverer.h"

namespace dolphiimote {

	bool passthrough_mode(wiimote &mote)
	{
		return is_set(mote.enabled_capabilities, wiimote_capabilities::Extension | wiimote_capabilities::MotionPlus);
	}
	void capability_discoverer::data_received(dolphiimote_callbacks &callbacks, int wiimote_number, checked_array<const u8> data)
	{
		auto& mote = wiimote_states[wiimote_number];
		u8 message_type = data[1];

		if (message_type == 0x20)
			handle_status_report(wiimote_number, data);
		if (is_set(mote.enabled_capabilities, wiimote_capabilities::MotionPlus)) {
			if (!mote.extension_motion_plus_state && is_set(mote.enabled_capabilities, wiimote_capabilities::Extension)) {
				mote.available_capabilities &= ~wiimote_capabilities::Extension;
				enable_motion_plus_no_passthrough(wiimote_number);
			}
			if (mote.extension_motion_plus_state && !is_set(mote.available_capabilities, wiimote_capabilities::Extension)) {
				mote.available_capabilities |= wiimote_capabilities::Extension;
				enable_only_extension(wiimote_number);
			}
		}
	}

	bool can_unobstrusively_enable_extension(const wiimote& mote)
	{
		return !is_set(mote.enabled_capabilities, wiimote_capabilities::MotionPlus);
	}

	void capability_discoverer::handle_extension_connected(int wiimote)
	{
		auto& mote = wiimote_states[wiimote];

		if (can_unobstrusively_enable_extension(mote))
			init_and_identify_extension_controller(wiimote);
	}

	void capability_discoverer::handle_extension_disconnected(int wiimote)
	{
		auto& mote = wiimote_states[wiimote];

		mote.available_capabilities &= ~wiimote_capabilities::Extension;
		mote.set_extension_disabled();
	}

	void capability_discoverer::handle_extension_controller_changed(bool extension_controller_connected, int wiimote, bool& changed)
	{
		auto& mote = wiimote_states[wiimote];

		if (extension_controller_connected && !is_set(mote.available_capabilities, wiimote_capabilities::Extension))
		{
			handle_extension_connected(wiimote);
			changed = true;
		}

		if (!extension_controller_connected && is_set(mote.available_capabilities, wiimote_capabilities::Extension) && !passthrough_mode(mote))
		{
			handle_extension_disconnected(wiimote);
			changed = true;
		}
	}

	void capability_discoverer::handle_status_report(int wiimote_number, checked_array<const u8> data)
	{
		if (data.size() < 5)
			return;

		u8 flags = data[4];

		bool battery_low = flags & 0x01;
		bool extension_controller_connected = flags & 0x02;
		bool speaker_enabled = flags & 0x04;
		bool ir_camera_enabled = flags & 0x08;

		bool led_1 = flags & 0x10;
		bool led_2 = flags & 0x20;
		bool led_3 = flags & 0x40;
		bool led_4 = flags & 0x80;

		bool capabilities_changed = false;

		handle_extension_controller_changed(flags & 0x02, wiimote_number, capabilities_changed);

		if (capabilities_changed)
			dispatch_capabilities_changed(wiimote_number, callbacks);
	}

	void fill_capabilities(dolphiimote_capability_status &status, wiimote &mote)
	{
		status.available_capabilities = mote.available_capabilities;
		status.enabled_capabilities = mote.enabled_capabilities;
		status.extension_type = mote.extension_type;
		status.extension_id = mote.extension_id;
	}

	void capability_discoverer::dispatch_capabilities_changed(int wiimote, dolphiimote_callbacks callbacks)
	{
		dolphiimote_capability_status status = { 0 };

		fill_capabilities(status, wiimote_states[wiimote]);

		if (callbacks.capabilities_changed)
			callbacks.capabilities_changed(wiimote, &status, callbacks.userdata);
	}

	std::map<u64, wiimote_extensions::type> _id_to_extension_type;

	std::map<u64, wiimote_extensions::type>& id_to_extension_type()
	{
		if (_id_to_extension_type.size() == 0)
		{
			_id_to_extension_type[0x000000000000] = wiimote_extensions::None;
			_id_to_extension_type[0x0000A4200000] = wiimote_extensions::Nunchuck;
			_id_to_extension_type[0x0000A4200505] = wiimote_extensions::Passthrough; // Activated Wii Motion Plus in Nunchuck passthrough mode
			_id_to_extension_type[0x0100A4200505] = wiimote_extensions::Passthrough; // Activated Wii Motion Plus in Nunchuck passthrough mode
			_id_to_extension_type[0xFF00A4200000] = wiimote_extensions::Nunchuck; //WEIRD - had a nunchuk that always gave off this ID. Was there any difference?
			_id_to_extension_type[0x0000A4200101] = wiimote_extensions::ClassicController;
			_id_to_extension_type[0x0000A4200705] = wiimote_extensions::Passthrough; // Activated Wii Motion Plus in Classic Controller passthrough mode
			_id_to_extension_type[0x0100A4200705] = wiimote_extensions::Passthrough; // Activated Wii Motion Plus in Classic Controller passthrough mode
			_id_to_extension_type[0x0100A4200101] = wiimote_extensions::ClassicControllerPro;
			_id_to_extension_type[0x0000A4200103] = wiimote_extensions::GHGuitar;
			_id_to_extension_type[0x0100A4200103] = wiimote_extensions::GHWorldTourDrums;
			_id_to_extension_type[0x0000A4200402] = wiimote_extensions::BalanceBoard;
			_id_to_extension_type[0x0000A4200405] = wiimote_extensions::MotionPlus;
			_id_to_extension_type[0x0100A4200405] = wiimote_extensions::MotionPlus;
		}

		return _id_to_extension_type;
	}
	wiimote_extensions::type get_type_from_id(u64 id) {
		if (id_to_extension_type().find(id) != id_to_extension_type().end())
			return id_to_extension_type()[id];
		return wiimote_extensions::None;
	}
	void capability_discoverer::update_extension_type_from_id(int wiimote_number)
	{
		u64 id = wiimote_states[wiimote_number].extension_id;
		if (id_to_extension_type().find(id) != id_to_extension_type().end())
			wiimote_states[wiimote_number].extension_type = id_to_extension_type()[id];
		else if (id == 0xFFFFFFFFFFFF)
			init_and_identify_extension_controller(wiimote_number); //Retry, because the controller extension was likely just plugged in (status report).
		else
			printf("capability_discoverer::update_extension_type_from_id: id %012llx NOT FOUND\n", id);
	}

	u64 capability_discoverer::read_extension_id(checked_array<const u8> data)
	{
		u64 id = 0;

		for (unsigned int i = 0; i < 6; i++)
			id |= ((u64)data[5 - i]) << (i * 8);

		return id;
	}

	void capability_discoverer::handle_extension_id_message(int wiimote_number, checked_array<const u8> data, dolphiimote_callbacks callbacks)
	{
		auto& mote = wiimote_states[wiimote_number];
		u8 error_bit = reader.read_error_bit(data);
		checked_array<const u8> extension_id = data.sub_array(7, 6);

		if (error_bit == 0 && data.valid())
		{
			u64 id = read_extension_id(extension_id);
			//When we get a passthrough id, don't override the current extension id!
			if (get_type_from_id(id) == wiimote_extensions::Passthrough) {
				mote.enabled_capabilities |= wiimote_capabilities::Extension;
				mote.enabled_capabilities |= wiimote_capabilities::MotionPlus;
			}
			else {
				if (id == 0x0100A4200405)
				{
					mote.enabled_capabilities &= ~wiimote_capabilities::Extension;
					mote.enabled_capabilities |= wiimote_capabilities::MotionPlus;
				}
				else if (!(get_type_from_id(id) == wiimote_extensions::MotionPlus))
				{
					mote.extension_id = id;
					mote.enabled_capabilities |= wiimote_capabilities::Extension;
					mote.available_capabilities |= wiimote_capabilities::Extension;
				}
			}
		}
		else
		{
			mote.extension_id = 0;
			mote.enabled_capabilities &= ~wiimote_capabilities::Extension;
		}
		update_extension_type_from_id(wiimote_number);
		dispatch_capabilities_changed(wiimote_number, callbacks);
		if (mote.extension_type == wiimote_extensions::BalanceBoard) {
			reader.read(wiimote_number, 0xA40024, 16, std::bind(&capability_discoverer::handle_balanceboard_calibration1, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
			reader.read(wiimote_number, 0xA40024 + 16, 8, std::bind(&capability_discoverer::handle_balanceboard_calibration2, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
		}
		//Appearently calibration data is stored for joysticks and things, but this is probably a pointless thing to bother calibrating for.
		/*if (mote.extension_type == wiimote_extensions::Nunchuck) {
			reader.read(wiimote_number, 0xA40020, 16, std::bind(&capability_discoverer::handle_balanceboard_calibration1, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
		}
		if (mote.extension_type == wiimote_extensions::ClassicController || mote.extension_type == wiimote_extensions::ClassicControllerPro) {
			reader.read(wiimote_number, 0xA40020, 16, std::bind(&capability_discoverer::handle_balanceboard_calibration1, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
		}*/
	}
	void capability_discoverer::handle_balanceboard_calibration1(int wiimote_number, checked_array<const u8> data, dolphiimote_callbacks callbacks)
	{
		u8 error_bit = reader.read_error_bit(data);
		checked_array<const u8> calib = data.sub_array(7, 16);
		wiimote_states[wiimote_number].calibrations.balance_board.kg0.top_right = calib[0] << 8 | calib[1];
		wiimote_states[wiimote_number].calibrations.balance_board.kg0.bottom_right = calib[2] << 8 | calib[3];
		wiimote_states[wiimote_number].calibrations.balance_board.kg0.top_left = calib[4] << 8 | calib[5];
		wiimote_states[wiimote_number].calibrations.balance_board.kg0.bottom_left = calib[6] << 8 | calib[7];
		wiimote_states[wiimote_number].calibrations.balance_board.kg17.top_right = calib[8] << 8 | calib[9];
		wiimote_states[wiimote_number].calibrations.balance_board.kg17.bottom_right = calib[10] << 8 | calib[11];
		wiimote_states[wiimote_number].calibrations.balance_board.kg17.top_left = calib[12] << 8 | calib[13];
		wiimote_states[wiimote_number].calibrations.balance_board.kg17.bottom_left = calib[14] << 8 | calib[15];
	}
	void capability_discoverer::handle_balanceboard_calibration2(int wiimote_number, checked_array<const u8> data, dolphiimote_callbacks callbacks)
	{
		u8 error_bit = reader.read_error_bit(data);
		checked_array<const u8> calib = data.sub_array(7, 8);

		wiimote_states[wiimote_number].calibrations.balance_board.kg34.top_right = calib[0] << 8 | calib[1];
		wiimote_states[wiimote_number].calibrations.balance_board.kg34.bottom_right = calib[2] << 8 | calib[3];
		wiimote_states[wiimote_number].calibrations.balance_board.kg34.top_left = calib[4] << 8 | calib[5];
		wiimote_states[wiimote_number].calibrations.balance_board.kg34.bottom_left = calib[6] << 8 | calib[7];
	}
	void capability_discoverer::handle_motionplus_id_message(int wiimote_number, checked_array<const u8> data, dolphiimote_callbacks callbacks)
	{
		u8 error_bit = reader.read_error_bit(data);

		if (error_bit == 0)
		{
			wiimote_states[wiimote_number].available_capabilities |= wiimote_capabilities::MotionPlus;
		}
		else
		{
			wiimote_states[wiimote_number].available_capabilities &= ~wiimote_capabilities::MotionPlus;
		}

		dispatch_capabilities_changed(wiimote_number, callbacks);
	}

	void capability_discoverer::send_extension_id_read_message(int wiimote_number)
	{
		reader.read(wiimote_number, 0XA400FA, 0x06, std::bind(&capability_discoverer::handle_extension_id_message, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
		//According to wiibrew: Read 0xa400fa for 6 extension ID bytes.
	}

	void capability_discoverer::init_and_identify_extension_controller(int wiimote_number)
	{
		sender.write_register(wiimote_number, 0xA400F0, 0x55, 1);
		sender.write_register(wiimote_number, 0xA400FB, 0x00, 1, std::bind(&capability_discoverer::send_extension_id_read_message, this, std::placeholders::_1));

		//According to wiibrew: init by writing 0x55 to 0x(4)A400F0, then writing 0x00 to 0x(4)A400FB
	}

	void capability_discoverer::determine_capabilities(int wiimote_number)
	{
		reader.read(wiimote_number, 0xA600FE, 0x02, std::bind(&capability_discoverer::handle_motionplus_id_message, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
		//According to wiibrew: Read 0xa600fe for 2 extension ID bytes.
		init_and_identify_extension_controller(wiimote_number);
	}

	void capability_discoverer::send_status_request(int wiimote_number)
	{
		sender.send(wiimote_message(wiimote_number, {0xa2, 0x15, 0}, 3, true));
	}

	void capability_discoverer::enable_motion_plus_no_passthrough(int wiimote_number)
	{
		sender.write_register(wiimote_number, 0xA400F0, 0x55, 1);
		sender.write_register(wiimote_number, 0xA400FB, 0x00, 1);
		sender.write_register(wiimote_number, 0xA600F0, 0x55, 1);
		sender.write_register(wiimote_number, 0xA600FE, 0x04, 1, std::bind(&capability_discoverer::send_extension_id_read_message, this, std::placeholders::_1));

		auto& mote = wiimote_states[wiimote_number];
		mote.enabled_capabilities &= ~wiimote_capabilities::Extension;
		mote.enabled_capabilities |= wiimote_capabilities::MotionPlus;
	}

	void capability_discoverer::enable_only_extension(int wiimote)
	{
		if (is_set(wiimote_states[wiimote].enabled_capabilities, wiimote_capabilities::MotionPlus))
		{
			sender.write_register(wiimote, 0xA400F0, 0x55, 1);  //According to Wiibrew: Writing 0x55 to 0x(4)A400F0 deactivates the MotionPlus
			wiimote_states[wiimote].enabled_capabilities &= ~wiimote_capabilities::MotionPlus;
		}

		if (!is_set(wiimote_states[wiimote].enabled_capabilities, wiimote_capabilities::Extension))
			init_and_identify_extension_controller(wiimote);
	}

	void capability_discoverer::enable_motion_plus_extension_passthrough(int wiimote_number)
	{
		sender.write_register(wiimote_number, 0xA400F0, 0x55, 1);
		sender.write_register(wiimote_number, 0xA400FB, 0x00, 1);
		sender.write_register(wiimote_number, 0xA600F0, 0x55, 1);

		if (wiimote_states[wiimote_number].extension_type == (dolphiimote_EXTENSION_CLASSIC_CONTROLLER
			| dolphiimote_EXTENSION_CLASSIC_CONTROLLER_PRO | dolphiimote_EXTENSION_GUITAR_HERO_GUITAR
			| dolphiimote_EXTENSION_GUITAR_HERO_WORLD_TOUR_DRUMS))
			sender.write_register(wiimote_number, 0xA600FE, 0x07, 1, std::bind(&capability_discoverer::send_extension_id_read_message, this, std::placeholders::_1));
		else
			sender.write_register(wiimote_number, 0xA600FE, 0x05, 1, std::bind(&capability_discoverer::send_extension_id_read_message, this, std::placeholders::_1));
		/* for Classic Controller Interleave with MotionPlus, need to send 0x07, not 0x05 (numchuck)
		sender.write_register(wiimote_number, 0xA600FE, 0x07, 1);*/

		wiimote_states[wiimote_number].enabled_capabilities |= wiimote_capabilities::MotionPlus | wiimote_capabilities::Extension;
	}

	void capability_discoverer::handle_motion_plus_and_extension_enabling(int wiimote_number, wiimote_capabilities::type capabilities_to_enable)
	{
		if ((capabilities_to_enable & wiimote_capabilities::Extension) && (capabilities_to_enable & wiimote_capabilities::MotionPlus))
			enable_motion_plus_extension_passthrough(wiimote_number);
		else if (is_set(capabilities_to_enable, wiimote_capabilities::Extension))
			enable_only_extension(wiimote_number);
		else if (is_set(capabilities_to_enable, wiimote_capabilities::MotionPlus))
			enable_motion_plus_no_passthrough(wiimote_number);
		else
		{
			//Don't really need to disable; doesn't hurt.
		}
	}

	void capability_discoverer::enable(int wiimote_number, wiimote_capabilities::type capabilities_to_enable)
	{
		if (capabilities_to_enable == wiimote_states[wiimote_number].enabled_capabilities)
		{
			printf("capability_discoverer::enable: Capabilities already enabled\n");
			return;
		}

		if ((capabilities_to_enable & wiimote_states[wiimote_number].available_capabilities) != capabilities_to_enable)
		{
			printf("capability_discoverer::enable: Capabilities not available to enable. Checking capabilities\n");
			determine_capabilities(wiimote_number);
			return;
		}

		handle_motion_plus_and_extension_enabling(wiimote_number, capabilities_to_enable);
	}

}