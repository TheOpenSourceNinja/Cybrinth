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
 * The ImageModifier class does various things related to image manipulation and conversion.
 */

#include "ImageModifier.h"
#include "StringConverter.h"

#ifdef HAVE_IOSTREAM
#include <iostream>
#endif //HAVE_IOSTREAM

ImageModifier::ImageModifier() {
	//ctor
}

ImageModifier::~ImageModifier() {
	//dtor
}

irr::video::ITexture* ImageModifier::imageToTexture( irr::video::IVideoDriver* driver, irr::video::IImage* oldImage, irr::core::stringw name ) {
	try {
		irr::video::ITexture* texture = driver->addTexture( name.c_str(), oldImage );
		texture->grab();
		return texture;
	} catch ( std::exception &e ) {
		std::wcerr << L"Error in ImageModifier::imageToTexture(): " << e.what() << std::endl;
		return nullptr;
	}
}

irr::video::ITexture* ImageModifier::resize( irr::video::ITexture* image, uint_fast32_t width, uint_fast32_t height, irr::video::IVideoDriver* driver ) {
	try {
		irr::video::IImage* tempImage = textureToImage( driver, image );
		auto name = image->getName().getInternalName() + L"-resized";
		driver->removeTexture( image ); //NOTE: This line used to make the program crash. No idea why it crashed or why it no longer crashes.
		irr::video::IImage* tempImage2 = driver->createImage( tempImage->getColorFormat(), irr::core::dimension2d< irr::u32 >( width, height ) );
		tempImage->copyToScaling( tempImage2 );
		tempImage->drop();
		image = imageToTexture( driver, tempImage2, name );
		tempImage2->drop();
		return image;
	} catch ( std::exception &e ) {
		std::wcerr << L"Error in ImageModifier::resize(): " << e.what() << std::endl;
	}
	return nullptr;
}

irr::video::IImage* ImageModifier::resize( irr::video::IImage* image, uint_fast32_t width, uint_fast32_t height, irr::video::IVideoDriver* driver ) {
	try {
		irr::video::IImage* tempImage2 = driver->createImage( image->getColorFormat(), irr::core::dimension2d< irr::u32 >( width, height ) );
		image->copyToScaling( tempImage2 );
		image->drop();
		image = tempImage2;
		return image;
	} catch ( std::exception &e ) {
		std::wcerr << L"Error in ImageModifier::resize(): " << e.what() << std::endl;
	}
	return nullptr;
}

irr::video::IImage* ImageModifier::textureToImage( irr::video::IVideoDriver* driver, irr::video::ITexture* texture ) {
	try {
		irr::video::IImage* newImage = driver->createImageFromData( texture->getColorFormat(), texture->getSize(), texture->lock(), false );
		texture->unlock();
		return newImage;
	} catch ( std::exception &e ) {
		std::wcerr << L"Error in ImageModifier::textureToImage(): " << e.what() << std::endl;
		return nullptr;
	}
}
