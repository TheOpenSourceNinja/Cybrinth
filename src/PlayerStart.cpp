/**
 * Copyright © 2013 James Dearing.
 * This file is part of Cybrinth.
 *
 * Cybrinth is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 *
 * Cybrinth is distributed in the hope that it will be fun, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License along with Cybrinth. If not, see <http://www.gnu.org/licenses/>.
*/
#include "PlayerStart.h"
#include "colors.h"
#include <iostream>

PlayerStart::PlayerStart() {
	try {
		reset();
	} catch ( std::exception &e ) {
		std::wcerr << L"Error in PlayerStart::PlayerStart(): " << e.what() << std::endl;
	}
}

PlayerStart::~PlayerStart() {
	try {
	} catch ( std::exception &e ) {
		std::wcerr << L"Error in PlayerStart::~PlayerStart(): " << e.what() << std::endl;
	}
}

void PlayerStart::reset() {
	try {
		texture = nullptr;
		x = 0;
		y = 0;
		distanceFromExit = 0;
	} catch ( std::exception &e ) {
		std::wcerr << L"Error in PlayerStart::reset(): " << e.what() << std::endl;
	}
}

void PlayerStart::loadTexture( irr::video::IVideoDriver* driver ) {
	try {
		loadTexture( driver, 1 );
	} catch ( std::exception &e ) {
		std::wcerr << L"Error in PlayerStart::loadTexture(): " << e.what() << std::endl;
	}
}

void PlayerStart::loadTexture( irr::video::IVideoDriver* driver, uint_fast16_t size ) {
	try {
		irr::video::IImage *tempImage = driver->createImage( irr::video::ECF_A8R8G8B8, irr::core::dimension2d< irr::u32 >( size, size ) );
		tempImage->fill( WHITE );
		setColor( BLACK );

		driver->removeTexture( texture );
		texture = driver->addTexture( L"playerStart", tempImage );
	} catch ( std::exception &e ) {
		std::wcerr << L"Error in PlayerStart::loadTexture(): " << e.what() << std::endl;
	}
}

void PlayerStart::draw( irr::video::IVideoDriver* driver, uint_fast16_t width, uint_fast16_t height ) {
	try {
		uint_fast16_t size;

		if( width < height ) {
			size = width;
		} else {
			size = height;
		}

		if( texture == nullptr || ( texture != nullptr && texture->getSize().Width != size ) ) {
			loadTexture( driver, size );
		}

		Object::draw( driver, width, height );
	} catch ( std::exception &e ) {
		std::wcerr << L"Error in PlayerStart::draw(): " << e.what() << std::endl;
	}
}
