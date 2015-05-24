#include "XPMImageLoader.h"
#ifdef HAVE_STRING
	#include <string>
#endif //HAVE_STRING
#ifdef HAVE_IOSTREAM
	#include <iostream>
#endif //HAVE_IOSTREAM
#ifdef HAVE_VECTOR
	#include <vector>
#endif //HAVE_VECTOR
#include <assert.h>
#include <map>

XPMImageLoader::XPMImageLoader() {
	//ctor
}

void XPMImageLoader::loadImage( irr::video::IVideoDriver* driver, irr::video::IImage* storage, Collectable::type_t type ) {
	
	char ** xpm = nullptr;
	switch( type ) {
		case Collectable::KEY: {
			#include "compiled-images/key.xpm"
			xpm = key_xpm;
			break;
		}
		case Collectable::ACID: {
			#include "compiled-images/acid.xpm"
			xpm = acid_xpm;
			break;
		}
		default: {
			break;
		}
	}
	
	if( xpm != nullptr ) {
		std::string temp = std::string( xpm[ 0 ] );
		std::string widthString = temp.substr( 0, temp.find( ' ' ) );
		temp = temp.substr( temp.find( ' ' ) + 1 );
		std::string heightString = temp.substr( 0, temp.find( ' ' ) );
		temp = temp.substr( temp.find( ' ' ) + 1 );
		std::string numberOfColorsString = temp.substr( 0, temp.find( ' ' ) );
		temp = temp.substr( temp.find( ' ' ) + 1 );
		std::string charPerPixelString = temp;
		
		std::cout << widthString << std::endl;
		std::cout << heightString << std::endl;
		std::cout << numberOfColorsString << std::endl;
		std::cout << charPerPixelString << std::endl;
		
		irr::u32 width = std::stoul( widthString );
		irr::u32 height = std::stoul( heightString );
		uint32_t numberOfColors = std::stoul( numberOfColorsString );
		uint8_t charPerPixel = std::stoul( charPerPixelString );
		
		irr::video::ECOLOR_FORMAT format = irr::video::ECF_UNKNOWN;
		if( numberOfColors <= UINT16_MAX ) {
			format = irr::video::ECF_A1R5G5B5;
		} else {
			format = irr::video::ECF_A8R8G8B8;
		}
		
		std::map< std::string, irr::video::SColor > colorMap;
		
		irr::video::IImage* newImage = driver->createImage( format, irr::core::dimension2d< decltype( width ) >( width, height ) );
		for( decltype( numberOfColors ) i = 1; i <= numberOfColors; ++i ) {
			std::string kxpm = xpm[ i ];
			std::string id = kxpm.substr( 0, charPerPixel );
			
			colorType_t colorType;
			auto pos = kxpm.find( 'c' ); //c means the image is in color
			if( pos == std::string::npos ) {
				pos = kxpm.find( 'g' ); //g means grayscale
				if( pos == std::string::npos ) {
					pos = kxpm.find( 'm' ); //m means monochrome
					if( pos == std::string::npos ) {
						pos = kxpm.find( 's' ); //s means "symbolic", which I guess means the names of colors.
						assert( pos != std::string::npos ); //Since I don't know for sure how to deal with symbolic colors, just produce an error for now. TODO: Deal with this properly
					} else {
						colorType = MONOCHROME;
					}
				} else {
					colorType = GRAYSCALE;
				}
			} else {
				colorType = COLOR;
			}
			std::string colorString = kxpm.substr( pos + 2 );
			
			irr::video::SColor color;
			
			if( colorString == "None" ) {
				color.set( 0, 0, 0, 0 ); //Invisible
			} else {
				colorString = colorString.substr( colorString.find( '#' ) + 1 );
				std::string redString = colorString.substr( 0, 2 );
				std::string greenString = colorString.substr( 2, 2 );
				std::string blueString = colorString.substr( 4, 2 );
				
				irr::u32 red = stoul( redString, nullptr, 16 );
				irr::u32 green = stoul( greenString, nullptr, 16 );
				irr::u32 blue = stoul( blueString, nullptr, 16 );
				
				color.set( 255, red, green, blue );
			}
			colorMap[ id ] = color;
		}
		
		decltype( height ) lineNumberStart = numberOfColors + 1;
		for( decltype( height ) lineNumber = lineNumberStart; lineNumber < height + lineNumberStart; ++lineNumber ) {
			std::string line = xpm[ lineNumber ];
			for( decltype( width ) x = 0; x < width; ++x ) {
				std::string id = line.substr( x * charPerPixel, charPerPixel );
				newImage->setPixel( x, lineNumber - lineNumberStart, colorMap[ id ] );
			}
		}
		
		newImage->copyToScaling( storage );
	}
}
