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
#include "colors.h"
#include "Goal.h"
#include <iostream>



Goal::Goal() {
	try {
		setColor( WHITE );
	} catch ( std::exception e ) {
		std::wcerr << L"Error in Goal::Goal(): " << e.what() << std::endl;
	}
}

Goal::~Goal() {
	try {
	} catch ( std::exception e ) {
		std::wcerr << L"Error in Goal::~Goal(): " << e.what() << std::endl;
	}
}

void Goal::draw( irr::video::IVideoDriver* driver, uint_least16_t width, uint_least16_t height ) {
	try {
		uint_least16_t size;

		if( width < height ) {
			size = width;
		} else {
			size = height;
		}

		if( texture->getSize().Width != size ) {
			loadTexture( driver, size );
		}

		Object::draw( driver, width, height );
	} catch ( std::exception e ) {
		std::wcerr << L"Error in Goal::draw(): " << e.what() << std::endl;
	}
}


void Goal::loadTexture( irr::video::IVideoDriver* driver ) {
	try {
		//texture = driver->getTexture( "diamond.png" );
		loadTexture( driver, 1 );
	} catch ( std::exception e ) {
		std::wcerr << L"Error in Goal::loadTexture(): " << e.what() << std::endl;
	}
}

void Goal::loadTexture( irr::video::IVideoDriver* driver, uint_least16_t size ) {
	try {
		irr::video::IImage *tempImage = driver->createImage( irr::video::ECF_A8R8G8B8, irr::core::dimension2d< irr::u32 >( size, size ) ); //Colorspace should be irr::video::A1R5G5B5 but that causes a bug on my current laptop.
		tempImage->fill( irr::video::SColor( 0, 0, 0, 0) ); //Fills the image with invisibility!

		for( uint_fast16_t x = 0; x <= ( size / 2 ); ++x ) {
			for( uint_fast16_t y = 0; y <= x; ++y ) {
				tempImage->setPixel( x, y + ( size / 2 ), colorOne );
				tempImage->setPixel( size - x, y + ( size / 2 ), colorOne );
				tempImage->setPixel( x, size - ( y + ( size / 2 ) ), colorOne );
				tempImage->setPixel( size - x, size - ( y + ( size / 2 ) ), colorOne );
			}
		}

		size /= 2;

		for( uint_fast16_t x = ( size / 2 ); x <= size; ++x ) {
			for( uint_fast16_t y = ( size / 2 ); y <= x; ++y ) {
				tempImage->setPixel( x, y + ( size / 2 ), colorTwo );
				tempImage->setPixel( ( size * 2 ) - x, y + ( size / 2 ), colorTwo );
				tempImage->setPixel( x, ( size * 2 ) - ( y + ( size / 2 ) ), colorTwo );
				tempImage->setPixel( ( size * 2 ) - x, ( size * 2 ) - ( y + ( size / 2 ) ), colorTwo );
			}
		}

		driver->removeTexture( texture );
		texture = driver->addTexture( L"goalDiamond", tempImage );
	} catch ( std::exception e ) {
		std::wcerr << L"Error in Goal::loadTexture(): " << e.what() << std::endl;
	}
}

void Goal::reset() {
}
