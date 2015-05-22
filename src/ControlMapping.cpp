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

#include "ControlMapping.h"
#include <boost/algorithm/string.hpp>
#ifdef HAVE_IOSTREAM
	#include <iostream>
#endif //HAVE_IOSTREAM

ControlMapping::action_t ControlMapping::getAction() {
	try {
		return action;
	} catch ( std::exception &e ) {
		std::wcerr << L"Error in ControlMapping::getAction(): " << e.what() << std::endl;
		return ACTION_DO_NOT_USE;
	}
}

uint_fast8_t ControlMapping::getControllerButton() {
	try {
		return controllerButton;
	} catch ( std::exception &e ) {
		std::wcerr << L"Error in ControlMapping::getControllerButton(): " << e.what() << std::endl;
		return UINT_FAST8_MAX;
	}
}

uint_fast8_t ControlMapping::getControllerNumber() {
	try {
		return controllerNumber;
	} catch ( std::exception &e ) {
		std::wcerr << L"Error in ControlMapping::getControllerNumber(): " << e.what() << std::endl;
		return UINT_FAST8_MAX;
	}
}

irr::EKEY_CODE ControlMapping::getKey() {
	try {
		return key;
	} catch ( std::exception &e ) {
		std::wcerr << L"Error in ControlMapping::getKey(): " << e.what() << std::endl;
		return irr::KEY_KEY_CODES_COUNT;
	}
}

/** Access mouseWheelUp
* \return The current value of mouseWheelUp
*/
bool ControlMapping::getMouseWheelUp() {
	return mouseWheelUp;
}

void ControlMapping::setMouseDirection( mouseDirection_t val ) {
	mouseDirection = val;
}

ControlMapping::mouseDirection_t ControlMapping::getMouseDirection() {
	return mouseDirection;
}

/** Access mouseEvent
* \return The current value of mouseEvent
*/
irr::EMOUSE_INPUT_EVENT ControlMapping::getMouseEvent() {
	return mouseEvent;
}

uint_fast8_t ControlMapping::getPlayer() {
	try {
		return player;
	} catch ( std::exception &e ) {
		std::wcerr << L"Error in ControlMapping::getPlayer(): " << e.what() << std::endl;
		return UINT_FAST8_MAX;
	}
}

void ControlMapping::setAction( action_t val ) {
	try {
		action = val;
	} catch ( std::exception &e ) {
		std::wcerr << L"Error in ControlMapping::setAction(): " << e.what() << std::endl;
		action = ACTION_DO_NOT_USE;
	}
}

void ControlMapping::setControllerButton( uint_fast8_t val ) {
	try {
		controllerButton = val;
	} catch ( std::exception &e ) {
		std::wcerr << L"Error in ControlMapping::setControllerButton(): " << e.what() << std::endl;
	}
}

void ControlMapping::setControllerNumber( uint_fast8_t val ) {
	try {
		controllerNumber = val;
	} catch ( std::exception &e ) {
		std::wcerr << L"Error in ControlMapping::setControllerNumber(): " << e.what() << std::endl;
	}
}

void ControlMapping::setKey( irr::EKEY_CODE val ) {
	try {
		key = val;
	} catch ( std::exception &e ) {
		std::wcerr << L"Error in ControlMapping::setKey(): " << e.what() << std::endl;
	}
}

/** Set mouse event
* \param val New value to set
*/
void ControlMapping::setMouseEvent( irr::EMOUSE_INPUT_EVENT val ) {
	mouseEvent = val;
}

/** Set mouseWheelUp
* \param val New value to set
*/
void ControlMapping::setMouseWheelUp( bool val ) {
	mouseWheelUp = val;
}

void ControlMapping::setPlayer( uint_fast8_t val ) {
	try {
		player = val;
	} catch ( std::exception &e ) {
		std::wcerr << L"Error in ControlMapping::setPlayer(): " << e.what() << std::endl;
	}
}

ControlMapping::joystickDirection_t ControlMapping::getJoystickDirection() {
	return joystickDirection;
}

void ControlMapping::setJoystickDirection( joystickDirection_t val ) {
	joystickDirection = val;
}

uint_fast8_t ControlMapping::getJoystickAxis() {
	return joystickAxis;
}

void ControlMapping::setJoystickAxis( uint_fast8_t val ) {
	joystickAxis = val;
}

void ControlMapping::setJoystickDeadZonePercent( uint_fast8_t percent ) {
	if( percent > 100 ) {
		percent = 100;
	}
	int_fast32_t percent32 = percent;
	percent32 *= INT_FAST16_MAX;
	percent32 /= 100;
	joystickDeadZone = percent32;
}

int_fast16_t ControlMapping::getJoystickDeadZone() {
	return joystickDeadZone;
}

bool ControlMapping::operator==( ControlMapping other ) {
	return ( key == other.getKey() and mouseEvent == other.mouseEvent and controllerButton == other.controllerButton and controllerNumber == other.controllerNumber and mouseWheelUp == other.mouseWheelUp and action == other.action and player == other.player and joystickDirection == other.joystickDirection and joystickAxis == other.joystickAxis and mouseDirection == other.mouseDirection and joystickDeadZone == other.joystickDeadZone );
}

ControlMapping::ControlMapping() {
	try {
		mouseWheelUp = false;
		mouseEvent = irr::EMIE_COUNT;
		key = irr::KEY_KEY_CODES_COUNT;
		setAction( ACTION_DO_NOT_USE );
		controllerButton = UINT_FAST8_MAX;
		controllerNumber = UINT_FAST8_MAX;
		joystickDirection = JOYSTICK_DO_NOT_USE;
		mouseDirection = MOUSE_DO_NOT_USE;
		activated = false;
		joystickAxis = UINT_FAST8_MAX;
		joystickDeadZone = ( INT_FAST16_MAX / 2 );
	} catch ( std::exception &e ) {
		std::wcerr << L"Error in ControlMapping::ControlMapping(): " << e.what() << std::endl;
	}
}

ControlMapping::~ControlMapping() {
	try {
	} catch ( std::exception &e ) {
		std::wcerr << L"Error in ControlMapping::~ControlMapping(): " << e.what() << std::endl;
	}
}
