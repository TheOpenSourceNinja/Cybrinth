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

ControlMapping::controllerDirection_t ControlMapping::getControllerDirection() {
	return controllerDirection;
}

void ControlMapping::setControllerDirection( controllerDirection_t val ) {
	controllerDirection = val;
}

uint_fast8_t ControlMapping::getControllerAxis() {
	return controllerAxis;
}
void ControlMapping::setControllerAxis( uint_fast8_t val ) {
	controllerAxis = val;
}

ControlMapping::ControlMapping() {
	try {
		mouseWheelUp = false;
		mouseEvent = irr::EMIE_COUNT;
		key = irr::KEY_KEY_CODES_COUNT;
		setAction( ACTION_DO_NOT_USE );
		controllerButton = UINT_FAST8_MAX;
		controllerNumber = UINT_FAST8_MAX;
		controllerDirection = CONTROLLER_DO_NOT_USE;
		mouseDirection = MOUSE_DO_NOT_USE;
		activated = false;
		controllerAxis = UINT_FAST8_MAX;
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
