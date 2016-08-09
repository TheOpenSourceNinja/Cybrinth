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
 * The Collectable class is a generic class for in-game items, such as keys and acid.
 */

#include "Collectable.h"
#include "colors.h"
#include "XPMImageLoader.h"
#ifdef HAVE_IOSTREAM
	#include <iostream>
#endif //HAVE_IOSTREAM

#include "SettingsManager.h"

Collectable::Collectable() {
	try {
		x = 0;
		y = 0;
		setColorMode( SettingsManager::FULLCOLOR );
		setType( KEY );
		reset();
	} catch( std::exception &e ) {
		std::wcerr << L"Error in Collectable::Collectable(): " << e.what() << std::endl;
	}
}

Collectable::~Collectable() {
	try {
	} catch( std::exception &e ) {
		std::wcerr << L"Error in Collectable::~Collectable(): " << e.what() << std::endl;
	}
}

void Collectable::createTexture( irr::IrrlichtDevice* device, uint_fast16_t size ) {
	try {
		XPMImageLoader loader;
		driver = device->getVideoDriver();
		
		irr::video::IImage* tempImage = driver->createImage( irr::video::ECF_A8R8G8B8, irr::core::dimension2d< irr::u32 >( size, size ) );
		loader.loadCollectableImage( driver, tempImage, type );
		
		irr::core::stringw textureName;
		switch( type ) {
			case KEY: {
				textureName = L"key-xpm";
				break;
			}
			case ACID: {
				textureName = L"acid-xpm";
				break;
			}
			default: {
				textureName = L"generic collectable";
				break;
			}
		}
		
		adjustImageColors( tempImage );
		
		texture = resizer.imageToTexture( driver, tempImage, textureName );

		if( texture == nullptr ) {
			irr::video::IImage* temp = driver->createImage( irr::video::ECF_A1R5G5B5, irr::core::dimension2d< irr::u32 >( size, size ) );
			temp->fill( WHITE );
			texture = resizer.imageToTexture( driver, temp, "generic collectable" );
		}
		
		if( texture not_eq nullptr and texture->getSize() not_eq irr::core::dimension2d< irr::u32 >( size, size ) ) {
			auto newTexture = resizer.resize( texture, size, size, driver );
			driver->removeTexture( texture );
			texture = newTexture;
		}
	} catch( std::exception &e ) {
		std::wcerr << L"Error in Collectable::loadTexture(): " << e.what() << std::endl;
	}
}

void Collectable::draw( irr::IrrlichtDevice* device, uint_fast16_t width, uint_fast16_t height ) {
	try {
		auto smaller = height;
		if( smaller > width ) {
			smaller = width;
		}
		
		if( texture->getSize() not_eq irr::core::dimension2d< decltype( texture->getSize().Height ) >( smaller, smaller ) ) {
			loadTexture( device, smaller );
		}

		Object::draw( device, width, height );
	} catch ( std::exception &e ) {
		std::wcerr << L"Error in Collectable::draw(): " << e.what() << std::endl;
	}
}

Collectable::type_t Collectable::getType() const {
	return type;
}

void Collectable::loadTexture( irr::IrrlichtDevice* device ) {
	loadTexture( device, 1 );
}

void Collectable::loadTexture( irr::IrrlichtDevice* device, uint_fast8_t size ) {
	auto* driver = device->getVideoDriver();
	if( texture not_eq nullptr and texture not_eq NULL and texture not_eq 0 ) {
		driver->removeTexture( texture );
		texture = nullptr;
	}
	switch( type ) {
		case KEY: {
			Object::loadTexture( device, size, L"key" );
			break;
		}
		case ACID: {
			Object::loadTexture( device, size, L"acid" );
			break;
		}
	}
	// cppcheck-suppress duplicateExpression
	if( texture == nullptr or texture == NULL or texture == 0 ) {
		createTexture( device, size );
	} else {
		//std::wcout << L"No need to call createTexture()" << std::endl;
	}
}

/**
 * @brief Sets owned to false.
 */
void Collectable::reset() {
	owned = false;
}

/**
 * @brief Adjusts the item's colors based on colorMode and the particular Collectable's type
 * @param colorMode: SettingsManager's color mode (fullcolor, grayscale, etc.)
 */
void Collectable::setColorMode( uint_fast8_t newColorMode ) {
	colorMode = newColorMode;
	setColors( colorOne, colorTwo );
}

void Collectable::setType( type_t newType ) {
	try {
		type = newType;

		switch( type ) {
			case KEY: {
				switch( colorMode ) {
					case SettingsManager::FULLCOLOR: {
						setColors( BROWN, YELLOW );
						break;
					}
					case SettingsManager::GRAYSCALE: {
						setColors( BROWN_GRAYSCALE, YELLOW_GRAYSCALE );
						break;
					}
					case SettingsManager::GREENSCALE: {
						setColors( BROWN_GREENSCALE, YELLOW_GREENSCALE );
						break;
					}
					case SettingsManager::AMBERSCALE: {
						setColors( BROWN_AMBERSCALE, YELLOW_AMBERSCALE );
						break;
					}
				}
				
				break;
			}
			case ACID: {
				switch( colorMode ) {
					case SettingsManager::FULLCOLOR: {
						setColors( BLUE, LIGHTGREEN );
						break;
					}
					case SettingsManager::GRAYSCALE: {
						setColors( BLUE_GRAYSCALE, LIGHTGREEN_GRAYSCALE );
						break;
					}
					case SettingsManager::GREENSCALE: {
						setColors( BLUE_GREENSCALE, LIGHTGREEN_GREENSCALE );
						break;
					}
					case SettingsManager::AMBERSCALE: {
						setColors( BLUE_AMBERSCALE, LIGHTGREEN_AMBERSCALE );
						break;
					}
				}
				
				break;
			}
		}
	} catch ( std::exception &e ) {
		std::wcerr << L"Error in Collectable::setType(): " << e.what() << std::endl;
	}
}
