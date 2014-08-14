/**
 * Copyright © 2012-2014 James Dearing.
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
#ifdef HAVE_IOSTREAM
#include <iostream>
#endif //HAVE_IOSTREAM
#include <boost/filesystem.hpp>
#include "StringConverter.h"


Goal::Goal() {
	try {
		setColor( WHITE );
	} catch ( std::exception &e ) {
		std::wcerr << L"Error in Goal::Goal(): " << e.what() << std::endl;
	}
}

Goal::~Goal() {
	try {
	} catch ( std::exception &e ) {
		std::wcerr << L"Error in Goal::~Goal(): " << e.what() << std::endl;
	}
}

void Goal::draw( irr::IrrlichtDevice* device, uint_fast16_t width, uint_fast16_t height ) {
	try {
		uint_fast16_t size;

		if( width < height ) {
			size = width;
		} else {
			size = height;
		}

		if( texture->getSize().Width != size ) {
			loadTexture( device, size );
		}

		Object::draw( device, width, height );
	} catch ( std::exception &e ) {
		std::wcerr << L"Error in Goal::draw(): " << e.what() << std::endl;
	}
}


void Goal::loadTexture( irr::IrrlichtDevice* device ) {
	try {
		loadTexture( device, 1 );
	} catch ( std::exception &e ) {
		std::wcerr << L"Error in Goal::loadTexture(): " << e.what() << std::endl;
	}
}

void Goal::loadTexture( irr::IrrlichtDevice* device, uint_fast16_t size ) {
	try {
		irr::video::IVideoDriver* driver = device->getVideoDriver();
		
		irr::core::stringw fileName = L"goal";
		
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
		
		if( texture == nullptr ) {
			irr::video::IImage *tempImage = driver->createImage( irr::video::ECF_A8R8G8B8, irr::core::dimension2d< irr::u32 >( size, size ) ); //Colorspace should be irr::video::A1R5G5B5 but that causes a bug on my current laptop.
			tempImage->fill( irr::video::SColor( 0, 0, 0, 0) ); //Fills the image with invisibility!

			for( uint_fast16_t x = 0; x <= ( size / 2 ); ++x ) {
				tempImage->setPixel( x, ( size / 2 ), colorOne );
				tempImage->setPixel( size - x, ( size / 2 ), colorOne );

				for( uint_fast16_t y = 1; y <= x; ++y ) {
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
		}
		
		if( texture->getSize() != irr::core::dimension2d< irr::u32 >( size, size ) ) {
			auto textureSize = texture->getSize();
			auto desiredSize = irr::core::dimension2d< irr::u32 >( size, size );
			std::wcout << L"Texture size (" << textureSize.Width << L"x" << textureSize.Height << L") is not equal to (" << desiredSize.Width << L"x" << desiredSize.Height << L")" << std::endl;
			auto newTexture = resizer.resize( texture, size, size, driver );
			driver->removeTexture(texture);
			texture = newTexture;
			textureSize = texture->getSize();
			std::wcout << L"New texture size (" << textureSize.Width << L"x" << textureSize.Height << L")" << std::endl;
		}
	} catch ( std::exception &e ) {
		std::wcerr << L"Error in Goal::loadTexture(): " << e.what() << std::endl;
	}
}

void Goal::reset() {
}
