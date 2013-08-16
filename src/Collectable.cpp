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

#include "Collectable.h"
#include "colors.h"
#include <iostream>



Collectable::Collectable() {
	try {
		x = 0;
		y = 0;
		type = COLLECTABLE_KEY;
	} catch ( std::exception e ) {
		std::wcerr << L"Error in Collectable::Collectable(): " << e.what() << std::endl;
	}
}

Collectable::~Collectable() {
	try {
	} catch ( std::exception e ) {
		std::wcerr << L"Error in Collectable::~Collectable(): " << e.what() << std::endl;
	}
}

void Collectable::setType( uint_least8_t newType ) {
	try {
		type = newType;

		switch( type ) {
			case COLLECTABLE_KEY: {
					setColor( YELLOW );
					break;
				}
			default: break;
		}
	} catch ( std::exception e ) {
		std::wcerr << L"Error in Collectable::setType(): " << e.what() << std::endl;
	}
}

uint_least8_t Collectable::getType() {
	try {
		return type;
	} catch ( std::exception e ) {
		std::wcerr << L"Error in Collectable::getType(): " << e.what() << std::endl;
	}
}

void Collectable::loadTexture( irr::video::IVideoDriver* driver ) {
	try {
		switch( type ) {
			case COLLECTABLE_KEY: {
					texture = driver->getTexture( L"key.png" );

					break;
				}
			default: break;
		}

		if( texture == NULL ) {
			irr::video::IImage* temp = driver->createImage( irr::video::ECF_A1R5G5B5, irr::core::dimension2d< irr::u32 >( 2, 2 ) );
			temp->fill( WHITE );
			texture = imageToTexture( driver, temp, "generic collectable" );
		}
	} catch ( std::exception e ) {
		std::wcerr << L"Error in Collectable::loadTexture(): " << e.what() << std::endl;
	}
}

irr::video::IImage* Collectable::textureToImage( irr::video::IVideoDriver* driver, irr::video::ITexture* texture ) {
	try {
		irr::video::IImage* newImage = driver->createImageFromData( texture->getColorFormat(), texture->getSize(), texture->lock(), false );
		texture->unlock();
		return newImage;
	} catch ( std::exception e ) {
		std::wcerr << L"Error in Collectable::textureToImage(): " << e.what() << std::endl;
		return NULL;
	}
}

irr::video::ITexture* Collectable::imageToTexture( irr::video::IVideoDriver* driver, irr::video::IImage* oldImage, irr::core::stringw name ) {
	try {
		irr::video::ITexture* texture = driver->addTexture( name.c_str(), oldImage );
		texture->grab();
		return texture;
	} catch ( std::exception e ) {
		std::wcerr << L"Error in Collectable::imageToTexture(): " << e.what() << std::endl;
		return NULL;
	}
}

void Collectable::resizeImage( irr::video::IVideoDriver* driver, uint_least16_t width, uint_least16_t height ) {
	try {
		irr::video::IImage* tempImage = textureToImage( driver, texture );
		//driver->removeTexture( texture );
		//texture->drop();
		irr::video::IImage* tempImage2 = driver->createImage( tempImage->getColorFormat(), irr::core::dimension2d< irr::u32 >( width, height ) );
		tempImage->copyToScaling( tempImage2 );
		//tempImage->drop();
		texture = imageToTexture( driver, tempImage2, L"resized" );
		//tempImage2->drop();
	} catch ( std::exception e ) {
		std::wcerr << L"Error in Collectable::resizeImage(): " << e.what() << std::endl;
	}
}

void Collectable::draw( irr::video::IVideoDriver* driver, uint_least16_t width, uint_least16_t height ) {
	try {
		uint_least16_t smaller = height;
		if( smaller > width ) {
			smaller = width;
		}

		//wcout << L"desired size: " << smaller << std::endl;

		if( texture->getSize() != irr::core::dimension2d< irr::u32 >( smaller, smaller ) ) {
			resizeImage( driver, smaller, smaller );
		}

		Object::draw( driver, width, height );
	} catch ( std::exception e ) {
		std::wcerr << L"Error in Collectable::draw(): " << e.what() << std::endl;
	}
}
