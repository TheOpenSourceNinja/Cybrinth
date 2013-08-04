#include "KeyMapping.h"

unsigned char KeyMapping::getAction() {
	return action;
}

irr::EKEY_CODE KeyMapping::getKey() {
	return key;
}

uint_least8_t KeyMapping::getPlayer() {
	return player;
}

void KeyMapping::setAction( unsigned char val ) {
	action = val;
}

void KeyMapping::setKey( irr::EKEY_CODE val ) {
	key = val;
}

void KeyMapping::setPlayer( uint_least8_t val ) {
	player = val;
}

KeyMapping::KeyMapping() {
	//ctor
}

KeyMapping::~KeyMapping() {
	//dtor
}
