/**
 * @file
 * @author James Dearing <dearingj@lifetime.oregonstate.edu>
 * 
 * @section LICENSE
 * Copyright © 2012-2016.
 * This file is part of Cybrinth.
 *
 * Cybrinth is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 *
 * Cybrinth is distributed 'as is' in the hope that it will be fun, but WITHOUT ANY WARRANTY; without even the implied warranty of TITLE, MERCHANTABILITY, COMPLETE DESTRUCTION OF EVIL MONSTERS, or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
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

#include "SystemSpecificsManager.h"

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
		//We should be doing the following stuff, I think, but if we do it crashes the program.
		/*if( not ( driver == nullptr or driver == NULL ) and not ( texture == nullptr or texture == NULL ) ) {
			driver->removeTexture( texture );
			texture = nullptr;
		}*/
	} catch ( std::exception &e ) {
		std::wcerr << L"Error in Object::~Object(): " << e.what() << std::endl;
	}
}

void Object::draw( irr::IrrlichtDevice* device, uint_fast16_t width, uint_fast16_t height ) {
	try {
		driver = device->getVideoDriver();
		
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

			if( ( x >= ( xInterp - delta ) ) and ( x <= ( xInterp + delta ) ) and ( y >= ( yInterp - delta ) ) and ( y <= ( yInterp + delta ) ) ) {
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

		if( texture not_eq nullptr ) {
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
		if( not ( texture == nullptr or texture == NULL ) ) {
			driver->removeTexture( texture );
			texture = nullptr;
		}
		
		texture = driver->getTexture( fileName );
		
		if( texture == NULL or texture == nullptr ) {
			std::vector< boost::filesystem::path > loadableTextures;
			
			SystemSpecificsManager system;
			auto textureSearchLocations = system.getImageFolders();
			bool fileFound = false;
			for( auto locationIterator = textureSearchLocations.begin(); locationIterator != textureSearchLocations.end() and not fileFound; ++locationIterator ) {
				boost::filesystem::path path = *locationIterator / L"items";
				
				if( exists( path ) ) {
					boost::filesystem::recursive_directory_iterator end;
					
					for( boost::filesystem::recursive_directory_iterator i( path ); i not_eq end and not fileFound; ++i ) {
						if( not is_directory( i->path() ) ) { //We've found a file
							irr::io::IFileSystem* fileSystem = device->getFileSystem();
							StringConverter stringConverter;
							irr::io::path filePath = stringConverter.toIrrlichtStringW( i->path().wstring() );
							if( fileSystem->getFileBasename( filePath, false ) == fileName ) {
								//Asks Irrlicht if the file is loadable. This way the game is certain to accept any file formats the library can use.
								for( decltype( driver->getImageLoaderCount() ) loaderNum = 0; loaderNum < driver->getImageLoaderCount() and not fileFound; ++loaderNum ) { //Irrlicht uses a different image loader for each file type. Loop through them all, ask each if it can load the file.
									irr::video::IImageLoader* loader = driver->getImageLoader( loaderNum );
								
									if( loader->isALoadableFileExtension( filePath ) ) {
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
			}
			
		}
		
		texture = driver->getTexture( fileName );
		
		if( texture == nullptr or texture == NULL ) {
			return;
		}  else {
			if( texture->getSize() not_eq irr::core::dimension2d< irr::u32 >( size, size ) ) {
				auto newTexture = resizer.resize( texture, size, size, driver );
				driver->removeTexture( texture );
				texture = newTexture;
			}
			
			irr::video::IVideoDriver* driver = device->getVideoDriver();
			irr::video::IImage* image = resizer.textureToImage( driver, texture );
			irr::core::stringw textureName = texture->getName().getInternalName(); //Needed when converting the image back to a texture
			driver->removeTexture( texture );
			texture = nullptr;
			
			adjustImageColors( image );
			
			textureName += L"-recolored";
			texture = resizer.imageToTexture( driver, image, textureName );
		}
	} catch ( std::exception &e ) {
		std::wcerr << L"fileName: \"" << fileName.c_str() << L"\"" << std::endl;
		std::wcerr << L"Error in Object::loadTexture(): " << e.what() << std::endl;
	}
}

void Object::adjustImageColors( irr::video::IImage* image ) {
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
				} else if( luminance < lightestLuminance and luminance > darkestLuminance ) {
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
}

void Object::moveX( int_fast8_t val ) {
	try {
		if( not moving ) {
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
		if( not moving ) {
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
