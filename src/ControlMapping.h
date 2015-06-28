/**
 * @file
 * @author James Dearing <dearingj@lifetime.oregonstate.edu>
 *
 * @section LICENSE
 * Copyright © 2012-2015.
 * This file is part of Cybrinth.
 *
 * Cybrinth is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 *
 * Cybrinth is distributed in the hope that it will be fun, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License along with Cybrinth. If not, see <http://www.gnu.org/licenses/>.
 *
 * @section DESCRIPTION
 * The ControlMapping class is responsible for a 1:1:1 mapping of control (keyboard, mouse, controller, etc.) to event (e.g. player moves up one unit) to player number.
 */

#ifndef CONTROLMAPPING_H
#define CONTROLMAPPING_H

#ifdef WINDOWS
    #include <irrlicht.h>
#else
    #include <irrlicht/irrlicht.h>
#endif
#ifdef HAVE_STRING
	#include <string>
#endif //HAVE_STRING
#include "Integers.h"
#include "PreprocessorCommands.h"

class ControlMapping {
	public:
		enum joystickDirection_t : uint_fast8_t { JOYSTICK_INCREASE, JOYSTICK_DECREASE, JOYSTICK_DO_NOT_USE };
		enum mouseDirection_t : uint_fast8_t { MOUSE_UP, MOUSE_DOWN, MOUSE_RIGHT, MOUSE_LEFT, MOUSE_DO_NOT_USE };
		enum action_t : uint_fast8_t { ACTION_MENU_ACTIVATE, ACTION_MENU_UP, ACTION_MENU_DOWN, ACTION_SCREENSHOT, ACTION_VOLUME_UP, ACTION_VOLUME_DOWN, ACTION_PLAYER_UP, ACTION_PLAYER_DOWN, ACTION_PLAYER_RIGHT, ACTION_PLAYER_LEFT, ACTION_DO_NOT_USE };
		ControlMapping();
		virtual ~ControlMapping();
		irr::EKEY_CODE getKey();
		void setKey( irr::EKEY_CODE val );
		uint_fast8_t getControllerButton();
		void setControllerButton( uint_fast8_t val );
		uint_fast8_t getControllerNumber();
		void setControllerNumber( uint_fast8_t val );
		action_t getAction();
		void setAction( action_t val );
		irr::EMOUSE_INPUT_EVENT getMouseEvent();
		bool getMouseWheelUp();
		uint_fast8_t getPlayer();
		void setPlayer( uint_fast8_t val );
		void setMouseEvent( irr::EMOUSE_INPUT_EVENT val );
		void setMouseWheelUp( bool val );
		joystickDirection_t getJoystickDirection();
		void setJoystickDirection( joystickDirection_t val );
		uint_fast8_t getJoystickAxis();
		void setJoystickAxis( uint_fast8_t val );
		void setMouseDirection( mouseDirection_t val );
		mouseDirection_t getMouseDirection();
		int_fast16_t getJoystickDeadZone();
		void setJoystickDeadZonePercent( uint_fast8_t percent );

		bool operator==( ControlMapping other );

		bool activated; //To be set, unset, and accessed by MainGame. Indicates whether the relevant button has been pressed, or joystick moved, etc. Ensures smooth player movements.
		bool controlsAPlayer;
	protected:
	private:
		irr::EKEY_CODE key;
		irr::EMOUSE_INPUT_EVENT mouseEvent;
		uint_fast8_t controllerButton;
		uint_fast8_t controllerNumber;
		bool mouseWheelUp;
		action_t action;
		uint_fast8_t player;
		joystickDirection_t joystickDirection;
		uint_fast8_t joystickAxis;
		mouseDirection_t mouseDirection;
		int_fast16_t joystickDeadZone;
};

#endif // CONTROLMAPPING_H
