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
 * The Player class represents a player in the game.
 */
 
#include "Player.h"
#include <irrlicht/irrlicht.h>
#ifdef HAVE_IOSTREAM
	#include <iostream>
#endif //HAVE_IOSTREAM
#include "colors.h"
#include <boost/filesystem.hpp>
#include "MainGame.h"
#include "XPMImageLoader.h"

// cppcheck-suppress uninitMemberVar
Player::Player() {
	try {
		x = 0;
		y = 0;
		xInterp = 0;
		yInterp = 0;
		moving = false;
		setColors( BLACK, WHITE ); //The two colors should always be different so they can stand out against any background.
		texture = nullptr;
		isHuman = true;
		timeTakenThisMaze = 0;
		stepsTakenThisMaze = 0;
		keysCollectedThisMaze = 0;
		scoreTotal = 0;
		setPlayerNumber( UINT_FAST8_MAX );
		reset();
		setGM( nullptr );
	} catch( std::exception &e ) {
		std::wcerr << L"Error in Player::Player(): " << e.what() << std::endl;
	}
}

Player::~Player() {
	try {
	} catch( std::exception &e ) {
		std::wcerr << L"Error in Player::~Player(): " << e.what() << std::endl;
	}
}

//Draws a filled circle. Somebody please implement a faster algorithm.
void Player::createTexture( irr::IrrlichtDevice* device, uint_fast16_t size ) {
	try {
		irr::video::IVideoDriver* driver = device->getVideoDriver();
		XPMImageLoader loader;
		
		irr::video::IImage* tempImage = driver->createImage( irr::video::ECF_A8R8G8B8, irr::core::dimension2d< irr::u32 >( size, size ) );
		loader.loadOtherImage( driver, tempImage, XPMImageLoader::PLAYER );
		
		irr::core::stringw textureName = "player-xpm";
		
		adjustImageColors( tempImage );
		
		texture = resizer.imageToTexture( driver, tempImage, textureName );

		if( texture == nullptr ) {
			irr::video::IImage* temp = driver->createImage( irr::video::ECF_A1R5G5B5, irr::core::dimension2d< irr::u32 >( size, size ) );
			temp->fill( WHITE );
			texture = resizer.imageToTexture( driver, temp, "generic player" );
		}
		
		if( texture not_eq nullptr and texture->getSize() not_eq irr::core::dimension2d< irr::u32 >( size, size ) ) {
			auto newTexture = resizer.resize( texture, size, size, driver );
			driver->removeTexture( texture );
			texture = newTexture;
		}
	} catch( std::exception &e ) {
		std::wcerr << L"Error in Player::createTexture(): " << e.what() << std::endl;
	}
}

void Player::draw( irr::IrrlichtDevice* device, uint_fast16_t width, uint_fast16_t height ) {
	try {
		uint_fast16_t size;

		if( width < height ) {
			size = width;
		} else {
			size = height;
		}

		if( texture->getSize().Width not_eq size ) {
			loadTexture( device, size );
			if( texture == nullptr or texture == NULL ) {
				createTexture( device, size );
			}
		}

		Object::draw( device, width, height );
	} catch( std::exception &e ) {
		std::wcerr << L"Error in Player::draw(): " << e.what() << std::endl;
	}
}

uint_fast8_t Player::getItem() {
	return heldItem;
}

Collectable::type_t Player::getItemType() {
	return heldItemType;
}

intmax_t Player::getScoreLastMaze() {
	return scoreLastMaze;
}
intmax_t Player::getScoreTotal() {
	return scoreTotal;
}

void Player::giveItem( uint_fast8_t item, Collectable::type_t type ) {
	heldItem = item;
	heldItemType = type;
	if( mg not_eq nullptr ) {
		mg->getCollectable( heldItem )->setX( x );
		mg->getCollectable( heldItem )->setY( y );
		mg->getCollectable( heldItem )->owned = true;
	}
}

bool Player::hasItem() {
	return heldItem not_eq UINT_FAST8_MAX;
}

bool Player::hasItem( uint_fast8_t item ) {
	return heldItem == item;
}

void Player::loadTexture( irr::IrrlichtDevice* device ) {
	try {
		if( not ( texture == nullptr or texture == NULL ) ) {
			device->getVideoDriver()->removeTexture( texture );
			texture = nullptr;
		}
		loadTexture( device, 1 );
		if( texture == nullptr or texture == NULL ) {
			createTexture( device, 1 );
		}
	} catch( std::exception &e ) {
		std::wcerr << L"Error in Player::loadTexture(): " << e.what() << std::endl;
	}
}

void Player::loadTexture( irr::IrrlichtDevice* device, uint_fast16_t size ) {
	irr::core::stringw fileName = L"player";
	if( not ( texture == nullptr or texture == NULL ) ) {
		device->getVideoDriver()->removeTexture( texture );
		texture = nullptr;
	}
	Object::loadTexture( device, size, fileName );
	
	if( texture == nullptr or texture == NULL ) {
		if( playerNumber < 10 ) {
			fileName += L"00";
		} else if( playerNumber < 100 ) {
			fileName += L"0";
		}
		fileName += playerNumber;
		Object::loadTexture( device, size, fileName );
	}
	
	if( texture == nullptr or texture == NULL ) {
		createTexture( device, size );
	}
}

void Player::moveX( int_fast8_t val ) {
	try {
		Object::moveX( val );
		stepsTakenThisMaze += 1;
		if( hasItem() and mg not_eq nullptr ) {
			mg->getCollectable( heldItem )->moveX( val );
		}
	} catch( std::exception &e ) {
		std::wcerr << L"Error in Player::moveX(): " << e.what() << std::endl;
	}
}

void Player::moveY( int_fast8_t val ) {
	try {
		Object::moveY( val );
		stepsTakenThisMaze += 1;
		if( hasItem() and mg not_eq nullptr ) {
			mg->getCollectable( heldItem )->moveY( val );
		}
	} catch( std::exception &e ) {
		std::wcerr << L"Error in Player::moveY(): " << e.what() << std::endl;
	}
}

void Player::removeItem() {
	if( hasItem() ) {
		mg->eraseCollectable( heldItem );
		heldItem = UINT_FAST8_MAX;
	}
}

void Player::reset() {
	try {
		stepsTakenLastMaze = stepsTakenThisMaze;
		stepsTakenThisMaze = 0;
		timeTakenLastMaze = timeTakenThisMaze;
		timeTakenThisMaze = 0;
		keysCollectedLastMaze = keysCollectedThisMaze;
		keysCollectedThisMaze = 0;
		heldItem = UINT_FAST8_MAX;
		scoreLastMaze = 0;
	} catch( std::exception &e ) {
		std::wcerr << L"Error in Player::reset(): " << e.what() << std::endl;
	}
}

void Player::setColorBasedOnNum() {
	Object::setColorBasedOnNum( playerNumber );
}

void Player::setGM( MainGame* newGM ) {
	mg = newGM;
}

void Player::setPlayerNumber( uint_fast8_t newNumber ) {
	playerNumber = newNumber;
}

void Player::setScore( intmax_t newScore ) {
	scoreTotal += newScore;
	scoreLastMaze = newScore;
}
