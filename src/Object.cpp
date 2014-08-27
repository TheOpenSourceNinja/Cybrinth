/**
 * @file
 * @author James Dearing <dearingj@lifetime.oregonstate.edu>
 * 
 * @section LICENSE
 * Copyright © 2012-2014.
 * This file is part of Cybrinth.
 *
 * Cybrinth is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 *
 * Cybrinth is distributed in the hope that it will be fun, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License along with Cybrinth. If not, see <http://www.gnu.org/licenses/>.
 * 
 * @section DESCRIPTION
 * The Object class contains those properties common to players, goals, and collectables.
 */
 
#include "Object.h"
#include "colors.h"
#ifdef HAVE_IOSTREAM
	#include <iostream>
#endif //HAVE_IOSTREAM
#include <boost/filesystem.hpp>
#include "StringConverter.h"



Object::Object() {
	try {
		x = 0;
		y = 0;
		xInterp = 0;
		yInterp = 0;
		moving = false;
		distanceFromExit = 0;
		texture = nullptr;
		driver = nullptr;
		setColors( BLACK, GREEN );
	} catch ( std::exception &e ) {
		std::wcerr << L"Error in Object::Object(): " << e.what() << std::endl;
	}
}

Object::~Object() {
	try {
		if( !( driver == nullptr || driver == NULL ) && !( texture == nullptr || texture == NULL ) ) {
			driver->removeTexture( texture );
			texture = nullptr;
		}
	} catch ( std::exception &e ) {
		std::wcerr << L"Error in Object::~Object(): " << e.what() << std::endl;
	}
}

void Object::draw( irr::IrrlichtDevice* device, uint_fast16_t width, uint_fast16_t height ) {
	try {
		irr::video::IVideoDriver* driver = device->getVideoDriver();
		if( moving ) {
			float delta = 0.2; //Magic number! The value doesn't matter, except it should be between 0.0 and 1.0. Affects how quickly players appear to move between locations (how fast xInterp and yInterp approach the real x and y).

			if( x > xInterp ) {
				xInterp += delta;
			} else if( x < xInterp ) {
				xInterp -= delta;
			}

			if( y > yInterp ) {
				yInterp += delta;
			} else if( y < yInterp ) {
				yInterp -= delta;
			}

			if( ( x >= ( xInterp - delta ) ) && ( x <= ( xInterp + delta ) ) && ( y >= ( yInterp - delta ) ) && ( y <= ( yInterp + delta ) ) ) {
				moving = false;
				xInterp = x;
				yInterp = y;
			}
		} else {
			xInterp = x;
			yInterp = y;
		}

		uint_fast16_t size;

		if( width < height ) {
			size = width;
		} else {
			size = height;
		}

		if( texture != nullptr ) {
			int_fast16_t cornerX = ( xInterp * width ) + ( ( width / 2 ) - ( size / 2 ) );
			int_fast16_t cornerY = ( yInterp * height ) + ( ( height / 2 ) - ( size / 2 ) );
			driver->draw2DImage( texture,
								 irr::core::rect< irr::s32 >( cornerX, cornerY, cornerX + size, cornerY + size ),
								 irr::core::rect< irr::s32 >( irr::core::position2d< irr::s32 >( 0, 0 ), texture->getSize() ),
								 0, //The clipping rectangle, so we can draw only part of the texture if we want. Zero means draw the whole thing.
								 0, //"Array of 4 colors denoting the color values of the corners of the destRect". Zero means use the texture's own colors.
								 true ); //Whether to use the texture's alpha channel
		}
	} catch ( std::exception &e ) {
		std::wcerr << L"Error in Object::draw(): " << e.what() << std::endl;
	}
}

irr::video::SColor Object::getColorOne() {
	try {
		return colorOne;
	} catch ( std::exception &e ) {
		std::wcerr << L"Error in Object::getColorOne(): " << e.what() << std::endl;
		irr::video::SColor c;
		return c;
	}
}

irr::video::SColor Object::getColorTwo() {
	try {
		return colorTwo;
	} catch ( std::exception &e ) {
		std::wcerr << L"Error in Object::getColorTwo(): " << e.what() << std::endl;
		irr::video::SColor c;
		return c;
	}
}

uint_fast8_t Object::getX() {
	try {
		return x;
	} catch ( std::exception &e ) {
		std::wcerr << L"Error in Object::getX(): " << e.what() << std::endl;
		return UINT_FAST8_MAX;
	}
}

uint_fast8_t Object::getY() {
	try {
		return y;
	} catch ( std::exception &e ) {
		std::wcerr << L"Error in Object::getY(): " << e.what() << std::endl;
		return UINT_FAST8_MAX;
	}
}

void Object::loadTexture( irr::IrrlichtDevice* device, uint_fast16_t size, irr::core::stringw fileName ) {
	try {
		driver = device->getVideoDriver();
		
		if( !( texture == nullptr || texture == NULL ) ) {
			driver->removeTexture( texture );
			texture = nullptr;
		}
		
		{
			boost::filesystem::path path( boost::filesystem::current_path()/L"images" );
			
			//Which is better: system_complete() or absolute()? On my computer they seem to do the same thing. Both are part of Boost Filesystem.
			path = system_complete( path );
			//path = absolute( path );
			
			while( ( !exists( path ) || !is_directory( path ) ) && path.has_parent_path() ) {
				path = path.parent_path();
			}
			
			if( exists( path ) ) {
				boost::filesystem::recursive_directory_iterator end;
				bool fileFound = false;
				
				for( boost::filesystem::recursive_directory_iterator i( path ); i != end && !fileFound; ++i ) {
					if( !is_directory( i->path() ) ) { //We've found a file
						irr::io::IFileSystem* fileSystem = device->getFileSystem();
						StringConverter stringConverter;
						irr::io::path filePath = stringConverter.toIrrlichtStringW( i->path().wstring() );
						if( fileSystem->getFileBasename( filePath, false ) == fileName ) {
							//Asks Irrlicht if the file is loadable. This way the game is certain to accept any file formats the library can use.
							for( decltype( driver->getImageLoaderCount() ) loaderNum = 0; loaderNum < driver->getImageLoaderCount() && !fileFound; ++loaderNum ) { //Irrlicht uses a different image loader for each file type. Loop through them all, ask each if it can load the file.
								irr::video::IImageLoader* loader = driver->getImageLoader( loaderNum );
							
								//if( loader->isALoadableFileExtension( filePath ) ) { //Commenting this out because extensions don't always reflect the file's contents. Uncomment it for a minor speed improvement since not all files would need to be opened.
								irr::io::IReadFile* file = fileSystem->createAndOpenFile( filePath );
								if( loader->isALoadableFileFormat( file ) ) {
									fileName = filePath;
									fileFound = true;
									file->drop();
									break;
								}
								file->drop();
							}
						}
					}
				}
			}
		}
		
		texture = driver->getTexture( fileName );
		
		if( texture == nullptr || texture == NULL ) {
			return;
		}  else {
			if( texture->getSize() != irr::core::dimension2d< irr::u32 >( size, size ) ) {
				auto newTexture = resizer.resize( texture, size, size, driver );
				driver->removeTexture( texture );
				texture = newTexture;
			}
			
			irr::video::IVideoDriver* driver = device->getVideoDriver();
			irr::video::IImage* image = resizer.textureToImage( driver, texture );
			irr::core::stringw textureName = texture->getName().getInternalName(); //Needed when converting the image back to a texture
			driver->removeTexture( texture );
			texture = nullptr;
			
			//Find the darkest and lightest colors
			irr::video::SColor darkestColor = WHITE;
			auto darkestLuminance = darkestColor.getLuminance();
			irr::video::SColor lightestColor = BLACK;
			auto lightestLuminance = lightestColor.getLuminance();
			for( decltype( image->getDimension().Width ) x = 0; x < image->getDimension().Width; ++x ) {
				for( decltype( image->getDimension().Height ) y = 0; y < image->getDimension().Height; ++y ) {
					auto pixel = image->getPixel( x, y );
					if( pixel.getAlpha() > 0 ) {
						if( pixel.getLuminance() < darkestLuminance ) {
							darkestColor = pixel;
							darkestLuminance = darkestColor.getLuminance();
						}
						
						if( pixel.getLuminance() > lightestLuminance ) { //This is a separate if, not an else if, because there's a tiny chance the lightest and darkest colors might be the same
							lightestColor = pixel;
							lightestLuminance = lightestColor.getLuminance();
						}
					}
				}
			}
			
			//Now, set pixels to their desired colors (interpolate between colorOne and colorTwo instead of the lightest and darkest colors in the original file)
			for( decltype( image->getDimension().Width ) x = 0; x < image->getDimension().Width; ++x ) {
				for( decltype( image->getDimension().Height ) y = 0; y < image->getDimension().Height; ++y ) {
					auto pixel = image->getPixel( x, y );
					if( pixel.getAlpha() > 0 ) {
						auto luminance = pixel.getLuminance();
						if( luminance == lightestLuminance ) {
							auto newColor = colorTwo;
							newColor.setAlpha( pixel.getAlpha() );
							image->setPixel( x, y, newColor );
						} else if( luminance < lightestLuminance && luminance > darkestLuminance ) {
							auto interpolation = ( lightestLuminance - luminance ) / 255.0f;
							auto newColor = colorOne.getInterpolated( colorTwo, interpolation );
							image->setPixel( x, y , newColor );
						} else { // if( luminance == darkestLuminance ) {
							auto newColor = colorOne;
							newColor.setAlpha( pixel.getAlpha() );
							image->setPixel( x, y, newColor );
						}
					}
				}
			}
			
			textureName += L"-recolored";
			texture = resizer.imageToTexture( driver, image, textureName );
		}
	} catch ( std::exception &e ) {
		std::wcerr << L"Error in Object::loadTexture(): " << e.what() << std::endl;
	}
}

void Object::moveX( int_fast8_t val ) {
	try {
		if( !moving ) {
			x += val;
			moving = true;
		} else {
			xInterp = x;
			yInterp = y;
			x += val;
			moving = true;
		}
	} catch ( std::exception &e ) {
		std::wcerr << L"Error in Object::moveX(): " << e.what() << std::endl;
	}
}

void Object::moveY( int_fast8_t val ) {
	try {
		if( !moving ) {
			y += val;
			moving = true;
		} else {
			yInterp = y;
			xInterp = x;
			y += val;
			moving = true;
		}
	} catch ( std::exception &e ) {
		std::wcerr << L"Error in Object::moveY(): " << e.what() << std::endl;
	}
}

void Object::setColors( irr::video::SColor newColorOne, irr::video::SColor newColorTwo ) {
	try {
		colorOne = newColorOne;
		colorTwo = newColorTwo;
	} catch ( std::exception &e ) {
		std::wcerr << L"Error in Object::setColor(): " << e.what() << std::endl;
	}
}

void Object::setColorBasedOnNum( uint_fast8_t num ) {
	try {
		uint_fast8_t maxNum = NUMCOLORS - 1; //Numbers can start at zero, thus the maximum is one less than the total
		uint_fast8_t numOne = num % maxNum;
		uint_fast8_t numTwo = ( num / NUMCOLORS ) % maxNum;
		
		numTwo = maxNum - numTwo;
		if( numOne == numTwo ) {
			numTwo = ( numTwo + 1 ) % maxNum;
		}
		
		switch( numOne ) {
			case 0: {
				colorOne = BLACK;
				break;
			}
			case 1: {
				colorOne = BLUE;
				break;
			}
			case 2: {
				colorOne = GREEN;
				break;
			}
			case 3: {
				colorOne = CYAN;
				break;
			}
			case 4: {
				colorOne = RED;
				break;
			}
			case 5: {
				colorOne = MAGENTA;
				break;
			}
			case 6: {
				colorOne = BROWN;
				break;
			}
			case 7: {
				colorOne = GRAY;
				break;
			}
			case 8: {
				colorOne = LIGHTGRAY;
				break;
			}
			case 9: {
				colorOne = LIGHTBLUE;
				break;
			}
			case 10: {
				colorOne = LIGHTGREEN;
				break;
			}
			case 11: {
				colorOne = LIGHTCYAN;
				break;
			}
			case 12: {
				colorOne = LIGHTRED;
				break;
			}
			case 13: {
				colorOne = LIGHTMAGENTA;
				break;
			}
			case 14: {
				colorOne = YELLOW;
				break;
			}
			case 15: {
				colorOne = WHITE;
				break;
			}
			default: { //Just adding this to be extra careful
				colorOne = BLACK;
				break;
			}
		}
		
		switch( numTwo ) {
			case 0: {
				colorTwo = BLACK;
				break;
			}
			case 1: {
				colorTwo = BLUE;
				break;
			}
			case 2: {
				colorTwo = GREEN;
				break;
			}
			case 3: {
				colorTwo = CYAN;
				break;
			}
			case 4: {
				colorTwo = RED;
				break;
			}
			case 5: {
				colorTwo = MAGENTA;
				break;
			}
			case 6: {
				colorTwo = BROWN;
				break;
			}
			case 7: {
				colorTwo = GRAY;
				break;
			}
			case 8: {
				colorTwo = LIGHTGRAY;
				break;
			}
			case 9: {
				colorTwo = LIGHTBLUE;
				break;
			}
			case 10: {
				colorTwo = LIGHTGREEN;
				break;
			}
			case 11: {
				colorTwo = LIGHTCYAN;
				break;
			}
			case 12: {
				colorTwo = LIGHTRED;
				break;
			}
			case 13: {
				colorTwo = LIGHTMAGENTA;
				break;
			}
			case 14: {
				colorTwo = YELLOW;
				break;
			}
			case 15: {
				colorTwo = WHITE;
				break;
			}
			default: { //Just adding this to be extra careful
				colorTwo = BLACK;
				break;
			}
		}
	} catch ( std::exception &e ) {
		std::wcerr << L"Error in Object::setColorBasedOnNum(): " << e.what() << std::endl;
	}
	/*try {
		switch( num % ( NUMCOLORS - 3 ) ) { //Subtract 3 because there are some colors we're not using
			case 0: {
				setColor( RED ); //Special case: We don't want the player to be black (color 0) against a black background;
				//setColor( BLACK );
				break;
			}
			case 1: {
				setColor( BLUE );
				break;
			}
			case 2: {
				setColor( GREEN );
				break;
			}
			case 3: {
				setColor( CYAN );
				break;
			}
			case 4: {
				setColor( LIGHTRED ); //Normally red would be used here
				break;
			}
			case 5: {
				setColor( MAGENTA );
				break;
			}
			case 6: {
				setColor( BROWN );
				break;
			}
			case 7: {
				setColor( LIGHTGRAY );
				break;
			}
			case 8: {
				setColor( LIGHTBLUE ); //Skipping over gray because its alternate color is black
				break;
			}
			case 9: {
				setColor( LIGHTGREEN );
				break;
			}
			case 10: {
				setColor( LIGHTCYAN );
				break;
			}
			case 11: {
				setColor( LIGHTMAGENTA ); //Skipping over light red here since we've already used it earlier
				break;
			}
			case 12: {
				setColor( YELLOW );
				break;
			}
		}
	} catch ( std::exception &e ) {
		std::wcerr << L"Error in Object::setColorBasedOnNum(): " << e.what() << std::endl;
	}*/
}

void Object::setPos( uint_fast8_t newX, uint_fast8_t newY ) {
	try {
		setX( newX );
		setY( newY );
	} catch ( std::exception &e ) {
		std::wcerr << L"Error in Object::setPos(): " << e.what() << std::endl;
	}
}

void Object::setX( uint_fast8_t val ) {
	try {
		x = val;
		xInterp = x;
	} catch ( std::exception &e ) {
		std::wcerr << L"Error in Object::setX(): " << e.what() << std::endl;
	}
}

void Object::setY( uint_fast8_t val ) {
	try {
		y = val;
		yInterp = y;
	} catch ( std::exception &e ) {
		std::wcerr << L"Error in Object::setY(): " << e.what() << std::endl;
	}
}
