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
#include "Player.h"
#include <irrlicht/irrlicht.h>
#ifdef HAVE_IOSTREAM
#include <iostream>
#endif //HAVE_IOSTREAM
#include "colors.h"
#include <boost/filesystem.hpp>
#include "GameManager.h"

// cppcheck-suppress uninitMemberVar
Player::Player() {
	try {
		x = 0;
		y = 0;
		xInterp = 0;
		yInterp = 0;
		moving = false;
		setColor( RED );
		texture = nullptr;
		isHuman = true;
		timeTakenThisMaze = 0;
		stepsTakenThisMaze = 0;
		keysCollectedThisMaze = 0;
		scoreTotal = 0;
		setPlayerNumber( UINT_FAST8_MAX );
		reset();
		setGM( nullptr );
	} catch( std::exception &e ) {
		std::wcerr << L"Error in Player::Player(): " << e.what() << std::endl;
	}
}

Player::~Player() {
	try {
	} catch( std::exception &e ) {
		std::wcerr << L"Error in Player::~Player(): " << e.what() << std::endl;
	}
}

void Player::draw( irr::IrrlichtDevice* device, uint_fast16_t width, uint_fast16_t height ) {
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
	} catch( std::exception &e ) {
		std::wcerr << L"Error in Player::draw(): " << e.what() << std::endl;
	}
}

uint_fast8_t Player::getItem() {
	return heldItem;
}

Collectable::type_t Player::getItemType() {
	return heldItemType;
}

intmax_t Player::getScoreLastMaze() {
	return scoreLastMaze;
}
intmax_t Player::getScoreTotal() {
	return scoreTotal;
}

void Player::giveItem( uint_fast8_t item, Collectable::type_t type ) {
	heldItem = item;
	heldItemType = type;
	if( gm != nullptr ) {
		gm->getCollectable( heldItem )->setX( x );
		gm->getCollectable( heldItem )->setY( y );
		gm->getCollectable( heldItem )->owned = true;
	}
}

bool Player::hasItem() {
	return heldItem != UINT_FAST8_MAX;
}

bool Player::hasItem( uint_fast8_t item ) {
	return heldItem == item;
}

void Player::loadTexture( irr::IrrlichtDevice* device ) {
	try {
		loadTexture( device, 1 );
	} catch( std::exception &e ) {
		std::wcerr << L"Error in Player::loadTexture(): " << e.what() << std::endl;
	}
}

//Draws a filled circle. Somebody please implement a faster algorithm.
void Player::loadTexture( irr::IrrlichtDevice* device, uint_fast16_t size ) {
	try {
		irr::video::IVideoDriver* driver = device->getVideoDriver();
		
		irr::core::stringw fileName = L"player";
		if( playerNumber < 100 ) {
			fileName.append( L"0" );
			if( playerNumber < 10 ) {
				fileName.append( L"0" );
			}
		}
		fileName += static_cast< const unsigned int >( playerNumber );
		
		{
			boost::filesystem::path path( boost::filesystem::current_path()/L"images/players" );
			
			//Which is better: system_complete() or absolute()? On my computer they seem to do the same thing. Both are part of Boost Filesystem.
			path = system_complete( path );
			//path = absolute( path );
			
			while( ( !exists( path ) || !is_directory( path ) ) && path.has_parent_path() ) {
				if( gm != nullptr && gm->getDebugStatus() ) {
					std::wcout << L"Path " << path.wstring() << L" does not exist or is not a directory. Checking parent path " << path.parent_path().wstring() << std::endl;
				}

				path = path.parent_path();
			}
			
			if( exists( path ) ) {
				boost::filesystem::recursive_directory_iterator end;
				bool fileFound = false;
				
				for( boost::filesystem::recursive_directory_iterator i( path ); i != end && !fileFound; ++i ) {
					if( !is_directory( i->path() ) ) { //We've found a file
						if( gm != nullptr && gm->getDebugStatus() ) {
							std::wcout << i->path().wstring() << std::endl;
						}
						
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
			tempImage->fill( irr::video::SColor( 0, 0, 0, 0 ) ); //Fills the image with invisibility!
			tempImage->setPixel( size - 1, size - 1, irr::video::SColor( 0, 0, 0, 0 ) ); //Workaround for a bug in Irrlicht's software renderer
			
			irr::core::position2d< decltype( size ) > origin( size / 2, size / 2 );

			{
				int_fast16_t radius = size / 2;
				float rSquared = pow( static_cast< float >( radius ), 2.0f );
				for( auto x = -radius; x <= 0; ++x ) {
					auto height = static_cast< decltype( radius ) >( sqrt( rSquared - static_cast< float >( pow( static_cast< float >( x ), 2.0f ) ) ) );
					for( auto y = -height; y <= 0; ++y ) {
						tempImage->setPixel( x + origin.X, y + origin.Y, colorOne );
						tempImage->setPixel( x + origin.X, -y + origin.Y, colorOne );
						tempImage->setPixel( -x + origin.X, y + origin.Y, colorOne );
						tempImage->setPixel( -x + origin.X, -y + origin.Y, colorOne );
					}
				}
			}
			
			{
				size /= 2;
				int_fast16_t radius = size / 2;
				float rSquared = pow( static_cast< float >( radius ), 2.0f );
				for( auto x = -radius; x <= 0; ++x ) {
					auto height = static_cast< decltype( radius ) >( sqrt( rSquared - static_cast< float >( pow( static_cast< float >( x ), 2.0f ) ) ) );
					for( auto y = -height; y <= 0; ++y ) {
						tempImage->setPixel( x + origin.X, y + origin.Y, colorTwo );
						tempImage->setPixel( x + origin.X, -y + origin.Y, colorTwo );
						tempImage->setPixel( -x + origin.X, y + origin.Y, colorTwo );
						tempImage->setPixel( -x + origin.X, -y + origin.Y, colorTwo );
					}
				}
			}


			driver->removeTexture( texture );
			texture = driver->addTexture( L"playerCircle", tempImage );
		} else if( texture->getSize() != irr::core::dimension2d< irr::u32 >( size, size ) ) {
			texture = resizer.resize( texture, size, size, driver );
		}
	} catch( std::exception &e ) {
		std::wcerr << L"Error in Player::loadTexture(): " << e.what() << std::endl;
	}
}

void Player::moveX( int_fast8_t val ) {
	try {
		Object::moveX( val );
		stepsTakenThisMaze += 1;
		if( hasItem() && gm != nullptr ) {
			gm->getCollectable( heldItem )->moveX( val );
		}
	} catch( std::exception &e ) {
		std::wcerr << L"Error in Player::moveX(): " << e.what() << std::endl;
	}
}

void Player::moveY( int_fast8_t val ) {
	try {
		Object::moveY( val );
		stepsTakenThisMaze += 1;
		if( hasItem() && gm != nullptr ) {
			gm->getCollectable( heldItem )->moveY( val );
		}
	} catch( std::exception &e ) {
		std::wcerr << L"Error in Player::moveY(): " << e.what() << std::endl;
	}
}

void Player::removeItem() {
	if( hasItem() ) {
		gm->eraseCollectable( heldItem );
		heldItem = UINT_FAST8_MAX;
	}
}

void Player::reset() {
	try {
		stepsTakenLastMaze = stepsTakenThisMaze;
		stepsTakenThisMaze = 0;
		timeTakenLastMaze = timeTakenThisMaze;
		timeTakenThisMaze = 0;
		keysCollectedLastMaze = keysCollectedThisMaze;
		keysCollectedThisMaze = 0;
		heldItem = UINT_FAST8_MAX;
		scoreLastMaze = 0;
	} catch( std::exception &e ) {
		std::wcerr << L"Error in Player::reset(): " << e.what() << std::endl;
	}
}

void Player::setColorBasedOnNum() {
	Object::setColorBasedOnNum( playerNumber );
}

void Player::setGM( GameManager* newGM ) {
	gm = newGM;
}

void Player::setPlayerNumber( uint_fast8_t newNumber ) {
	playerNumber = newNumber;
}

void Player::setScore( intmax_t newScore ) {
	scoreTotal += newScore;
	scoreLastMaze = newScore;
}
