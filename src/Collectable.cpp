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
#include <iostream>

using namespace std;

Collectable::Collectable() {
	x = 0;
	y = 0;
	type = COLLECTABLE_KEY;
}

Collectable::~Collectable() {
	//dtor
}

void Collectable::setType( uint8_t newType ) {
	type = newType;

	switch( type ) {
		case COLLECTABLE_KEY: {
				setColor( YELLOW );
				break;
			}
		default: break;
	}
}

uint8_t Collectable::getType() {
	return type;
}

void Collectable::loadImage( irr::video::IVideoDriver* driver ) {
	switch( type ) {
		case COLLECTABLE_KEY: {
				image = driver->getTexture( L"key.png" );

				if( image == NULL ) {
					wcerr << L"Key image not loaded" << endl;
				}

				break;
			}
		default: break;
	}
}

irr::video::IImage* Collectable::textureToImage( irr::video::IVideoDriver* driver, irr::video::ITexture* texture ) {
	irr::video::IImage* newImage = driver->createImageFromData( texture->getColorFormat(), texture->getSize(), texture->lock(), false );
	texture->unlock();
	return newImage;
}

irr::video::ITexture* Collectable::imageToTexture( irr::video::IVideoDriver* driver, irr::video::IImage* oldImage, irr::core::stringw name ) {
	irr::video::ITexture* texture = driver->addTexture( name.c_str(), oldImage );
	texture->grab();
	return texture;
}

void Collectable::resizeImage( irr::video::IVideoDriver* driver, uint32_t width, uint32_t height ) {
	irr::video::IImage* tempImage = textureToImage( driver, image );
	driver->removeTexture( image );
	irr::video::IImage* tempImage2 = driver->createImage( tempImage->getColorFormat(), irr::core::dimension2d< irr::u32 >( width, height ) );
	tempImage->copyToScaling( tempImage2 );
	tempImage->drop();
	image = imageToTexture( driver, tempImage2, L"resized" );
	tempImage2->drop();
}

void Collectable::draw( irr::video::IVideoDriver* driver, uint32_t width, uint32_t height ) {
	uint32_t smaller = height;
	if( smaller > width ) {
		smaller = width;
	}
	if( image->getSize() != irr::core::dimension2d< irr::u32 >( smaller, smaller ) ) {
		//resizeImage( driver, width, height );
	}
	Object::draw( driver, width, height );
}
