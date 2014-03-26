#include "KeyMapping.h"
#include <algorithm/string.hpp>
#ifdef HAVE_IOSTREAM
#include <iostream>
#endif //HAVE_IOSTREAM

KeyMapping::action_t KeyMapping::getAction() {
	try {
		return action;
	} catch ( std::exception &e ) {
		std::wcerr << L"Error in KeyMapping::getAction(): " << e.what() << std::endl;
		return ERROR_DO_NOT_USE;
	}
}

std::wstring KeyMapping::getActionAsString() {
	try {
		switch( action ) {
			case MENU: {
				return L"menu";
			}
			case SCREENSHOT: {
				return L"screenshot";
			}
			case VOLUME_UP: {
				return L"volumeup";
			}
			case VOLUME_DOWN: {
				return L"volumedown";
			}
			case UP: {
				return L"up";
			}
			case DOWN: {
				return L"down";
			}
			case RIGHT: {
				return L"right";
			}
			case LEFT: {
				return L"left";
			}
			default: {
				return L"ERROR DO NOT USE";
			}
		}
	} catch( std::exception &e ) {
		std::wcout << L"Error in KeyMapping::getActionAsString(): " << e.what() << std::endl;
		return L"ERROR";
	}
}

void KeyMapping::setActionFromString( std::wstring val ) {
	try {
		if( boost::iequals( val, L"menu" ) ) {
			action = MENU;
		} else if( boost::iequals( val, L"screenshot" ) ) {
			action = SCREENSHOT;
		} else if( boost::iequals( val, L"volumeup" ) ) {
			action = VOLUME_UP;
		} else if( boost::iequals( val, L"volumedown" ) ) {
			action = VOLUME_DOWN;
		} else if( boost::iequals( val, L"u" ) or boost::iequals( val, L"up" ) ) {
			action = UP;
		} else if( boost::iequals( val, L"d" ) or boost::iequals( val, L"down" ) ) {
			action = DOWN;
		} else if( boost::iequals( val, L"r" ) or boost::iequals( val, L"right" ) ) {
			action = RIGHT;
		} else if( boost::iequals( val, L"l" ) or boost::iequals( val, L"left" ) ) {
			action = LEFT;
		} else {
			std::wstring error = L"Action string not recognized: " + val;
			throw error;
		}
	} catch( std::exception &e ) {
		std::wcerr << L"Error in KeyMapping::setActionFromString(): " << e.what() << std::endl;
		action = ERROR_DO_NOT_USE;
	} catch( std::wstring &e ) {
		std::wcerr << L"Error in KeyMapping::setActionFromString(): " << e << std::endl;
		action = ERROR_DO_NOT_USE;
	}
}

uint_fast8_t KeyMapping::getGamepadButton() {
	try {
		return gamepadButton;
	} catch ( std::exception &e ) {
		std::wcerr << L"Error in KeyMapping::getGamepadButton(): " << e.what() << std::endl;
		return UINT_FAST8_MAX;
	}
}

uint_fast8_t KeyMapping::getGamepadNumber() {
	try {
		return gamepadNumber;
	} catch ( std::exception &e ) {
		std::wcerr << L"Error in KeyMapping::getGamepadNumber(): " << e.what() << std::endl;
		return UINT_FAST8_MAX;
	}
}

irr::EKEY_CODE KeyMapping::getKey() {
	try {
		return key;
	} catch ( std::exception &e ) {
		std::wcerr << L"Error in KeyMapping::getKey(): " << e.what() << std::endl;
		return irr::KEY_KEY_CODES_COUNT;
	}
}

/** Access mouseWheelUp
* \return The current value of mouseWheelUp
*/
bool KeyMapping::getMouseWheelUp() {
	return mouseWheelUp;
}

/** Access mouseEvent
* \return The current value of mouseEvent
*/
irr::EMOUSE_INPUT_EVENT KeyMapping::getMouseEvent() {
	return mouseEvent;
}

uint_fast8_t KeyMapping::getPlayer() {
	try {
		return player;
	} catch ( std::exception &e ) {
		std::wcerr << L"Error in KeyMapping::getPlayer(): " << e.what() << std::endl;
		return UINT_FAST8_MAX;
	}
}

void KeyMapping::setAction( action_t val ) {
	try {
		action = val;
	} catch ( std::exception &e ) {
		std::wcerr << L"Error in KeyMapping::setAction(): " << e.what() << std::endl;
		action = ERROR_DO_NOT_USE;
	}
}

void KeyMapping::setGamepadButton( uint_fast8_t val ) {
	try {
		gamepadButton = val;
	} catch ( std::exception &e ) {
		std::wcerr << L"Error in KeyMapping::setGamepadButton(): " << e.what() << std::endl;
	}
}

void KeyMapping::setGamepadNumber( uint_fast8_t val ) {
	try {
		gamepadNumber = val;
	} catch ( std::exception &e ) {
		std::wcerr << L"Error in KeyMapping::setGamepadNumber(): " << e.what() << std::endl;
	}
}

void KeyMapping::setKey( irr::EKEY_CODE val ) {
	try {
		key = val;
	} catch ( std::exception &e ) {
		std::wcerr << L"Error in KeyMapping::setKey(): " << e.what() << std::endl;
	}
}

/** Set mouse event
* \param val New value to set
*/
void KeyMapping::setMouseEvent( irr::EMOUSE_INPUT_EVENT val ) {
	mouseEvent = val;
}

/** Set mouseWheelUp
* \param val New value to set
*/
void KeyMapping::setMouseWheelUp( bool val ) {
	mouseWheelUp = val;
}

void KeyMapping::setPlayer( uint_fast8_t val ) {
	try {
		player = val;
	} catch ( std::exception &e ) {
		std::wcerr << L"Error in KeyMapping::setPlayer(): " << e.what() << std::endl;
	}
}

KeyMapping::KeyMapping() {
	try {
		mouseWheelUp = false;
		mouseEvent = irr::EMIE_COUNT;
		key = irr::KEY_KEY_CODES_COUNT;
		setAction( ERROR_DO_NOT_USE );
		gamepadButton = UINT_FAST8_MAX;
		gamepadNumber = UINT_FAST8_MAX;
	} catch ( std::exception &e ) {
		std::wcerr << L"Error in KeyMapping::KeyMapping(): " << e.what() << std::endl;
	}
}

KeyMapping::~KeyMapping() {
	try {
	} catch ( std::exception &e ) {
		std::wcerr << L"Error in KeyMapping::~KeyMapping(): " << e.what() << std::endl;
	}
}
