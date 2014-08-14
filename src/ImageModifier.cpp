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
		StringConverter sc;
		irr::video::ITexture* texture = driver->addTexture( name.c_str(), oldImage ); //sc.toWCharArray( name ), oldImage );
		texture->grab();
		return texture;
	} catch ( std::exception &e ) {
		std::wcerr << L"Error in ImageModifier::imageToTexture(): " << e.what() << std::endl;
		return nullptr;
	}
}

irr::video::ITexture* ImageModifier::resize( irr::video::ITexture* image, uint_fast32_t width, uint_fast32_t height, irr::video::IVideoDriver* driver ) {
	try {
		/*if( image == nullptr || image->getOriginalSize().Width < width || image->getOriginalSize().Height < height ) {
			switch( type ) {
				case KEY: { //COLLECTABLE_KEY loads the texture from an image, so load the image before resizing
						loadTexture( driver );
						break;
					}
				default: {
					if( image == nullptr ) {
						loadTexture( driver );
					}
					break;
				}
			}
		}*/
		irr::video::IImage* tempImage = textureToImage( driver, image );
		//driver->removeTexture( image ); //Why does this make the program crash?
		//image->drop();
		irr::video::IImage* tempImage2 = driver->createImage( tempImage->getColorFormat(), irr::core::dimension2d< irr::u32 >( width, height ) );
		tempImage->copyToScaling( tempImage2 );
		//tempImage->drop();
		image = imageToTexture( driver, tempImage2, L"resized" );
		//tempImage2->drop();
		return image;
	} catch ( std::exception &e ) {
		std::wcerr << L"Error in ImageModifier::resize(): " << e.what() << std::endl;
	}
	return nullptr;
}

irr::video::IImage* ImageModifier::resize( irr::video::IImage* image, uint_fast32_t width, uint_fast32_t height, irr::video::IVideoDriver* driver ) {
	try {
		/*if( image == nullptr || image->getOriginalSize().Width < width || image->getOriginalSize().Height < height ) {
			switch( type ) {
				case KEY: { //COLLECTABLE_KEY loads the texture from an image, so load the image before resizing
						loadTexture( driver );
						break;
					}
				default: {
					if( image == nullptr ) {
						loadTexture( driver );
					}
					break;
				}
			}
		}*/
		irr::video::IImage* tempImage = image;//textureToImage( driver, image );
		//driver->removeTexture( image );
		//image->drop();
		irr::video::IImage* tempImage2 = driver->createImage( tempImage->getColorFormat(), irr::core::dimension2d< irr::u32 >( width, height ) );
		tempImage->copyToScaling( tempImage2 );
		//tempImage->drop();
		image = tempImage2;//imageToTexture( driver, tempImage2, L"resized" );
		//tempImage2->drop();
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
