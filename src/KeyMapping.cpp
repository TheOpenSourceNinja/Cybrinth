#include "KeyMapping.h"
#include <iostream>

wchar_t KeyMapping::getAction() {
	try {
		return action;
	} catch ( std::exception e ) {
		std::wcerr << L"Error in KeyMapping::getAction(): " << e.what() << std::endl;
		return ' ';
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

void KeyMapping::setAction( wchar_t val ) {
	try {
		action = val;
	} catch ( std::exception e ) {
		std::wcerr << L"Error in KeyMapping::setAction(): " << e.what() << std::endl;
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
