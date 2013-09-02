#include "KeyMapping.h"
#include <algorithm/string.hpp>
#include <iostream>

KeyMapping::action_t KeyMapping::getAction() {
	try {
		return action;
	} catch ( std::exception e ) {
		std::wcerr << L"Error in KeyMapping::getAction(): " << e.what() << std::endl;
		return ERROR_DO_NOT_USE;
	}
}

std::wstring KeyMapping::getActionAsString() {
	try {
		switch( action ) {
			case MENU: {
				return L"menu";
				break;
			}
			case SCREENSHOT: {
				return L"screenshot";
				break;
			}
			case VOLUME_UP: {
				return L"volumeup";
				break;
			}
			case VOLUME_DOWN: {
				return L"volumedown";
				break;
			}
			case UP: {
				return L"up";
				break;
			}
			case DOWN: {
				return L"down";
				break;
			}
			case RIGHT: {
				return L"right";
				break;
			}
			case LEFT: {
				return L"left";
				break;
			}
		}
	} catch( std::exception e ) {
		std::wcout << L"Error in KeyMapping::getActionAsString(): " << e.what() << std::endl;
		return L"";
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
		} else if( boost::iequals( val, L"up" ) ) {
			action = UP;
		} else if( boost::iequals( val, L"down" ) ) {
			action = DOWN;
		} else if( boost::iequals( val, L"right" ) ) {
			action = RIGHT;
		} else if( boost::iequals( val, L"left" ) ) {
			action = LEFT;
		} else {
			action = ERROR_DO_NOT_USE;
		}
	} catch( std::exception e ) {
		std::wcerr << L"Error in KeyMapping::setActionFromString(): " << e.what() << std::endl;
		action = ERROR_DO_NOT_USE;
	}
}

irr::EKEY_CODE KeyMapping::getKey() {
	try {
		return key;
	} catch ( std::exception e ) {
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

uint_least8_t KeyMapping::getPlayer() {
	try {
		return player;
	} catch ( std::exception e ) {
		std::wcerr << L"Error in KeyMapping::getPlayer(): " << e.what() << std::endl;
		return UINT_LEAST8_MAX;
	}
}

void KeyMapping::setAction( action_t val ) {
	try {
		action = val;
	} catch ( std::exception e ) {
		std::wcerr << L"Error in KeyMapping::setAction(): " << e.what() << std::endl;
		action = ERROR_DO_NOT_USE;
	}
}

void KeyMapping::setKey( irr::EKEY_CODE val ) {
	try {
		key = val;
	} catch ( std::exception e ) {
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

void KeyMapping::setPlayer( uint_least8_t val ) {
	try {
		player = val;
	} catch ( std::exception e ) {
		std::wcerr << L"Error in KeyMapping::setPlayer(): " << e.what() << std::endl;
	}
}

KeyMapping::KeyMapping() {
	try {
		mouseWheelUp = false;
		mouseEvent = irr::EMIE_COUNT;
		key = irr::KEY_KEY_CODES_COUNT;
		setAction( ERROR_DO_NOT_USE );
	} catch ( std::exception e ) {
		std::wcerr << L"Error in KeyMapping::KeyMapping(): " << e.what() << std::endl;
	}
}

KeyMapping::~KeyMapping() {
	try {
	} catch ( std::exception e ) {
		std::wcerr << L"Error in KeyMapping::~KeyMapping(): " << e.what() << std::endl;
	}
}
