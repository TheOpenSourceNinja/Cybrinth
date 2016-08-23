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
 * The MenuOption class represents an item in the in-game pause menu.
 */

#include "CustomException.h"
#include "MenuOption.h"
#include "colors.h"
#include "ImageModifier.h"
#include "StringConverter.h"
#include "XPMImageLoader.h"
#ifdef HAVE_IOSTREAM
	#include <iostream>
#endif //HAVE_IOSTREAM
#include <boost/filesystem.hpp>
#include "MainGame.h"
#include <irrlicht/irrlicht.h>

MenuOption::MenuOption() {
	try {
		x = 0;
		y = 0;
		font = nullptr;
		iconTexture = nullptr;
		setType( nullptr, DO_NOT_USE );
		highlighted = false;
	} catch ( std::exception &e ) {
		std::wcerr << L"Error in MenuOption::MenuOption(): " << e.what() << std::endl;
	}
}

MenuOption::~MenuOption() {
	try {
	} catch ( std::exception &e ) {
		std::wcerr << L"Error in MenuOption::~MenuOption(): " << e.what() << std::endl;
	}
}

void MenuOption::setMainGame( MainGame* mg ) {
	mainGame = mg;
}

void MenuOption::setType( irr::IrrlichtDevice* device, option_t newType ) {
	try {
		irr::core::stringw newText;
		
		type = newType;
		switch( type ) {
			case DO_NOT_USE: {
				newText = L"";
				break;
			}
			case NEW_MAZE: {
				newText = L"New maze";
				fileName = L"new_maze"; //I'm hardcoding these file names so that in the future, the actual button text may be changed (perhaps translated)
				break;
			}
			case RESTART_MAZE: {
				newText = L"Restart maze";
				fileName = L"restart_maze";
				break;
			}
			case JOIN_SERVER: {
				newText = L"Join server";
				fileName = L"join_server";
				break;
			}
			case LOAD_MAZE: {
				newText = L"Load maze";
				fileName = L"load_maze";
				break;
			}
			case SAVE_MAZE: {
				newText = L"Save maze";
				fileName = L"save_maze";
				break;
			}
			case SETTINGS: {
				newText = L"Settings";
				fileName = L"settings";
				break;
			}
			case EXIT_GAME: {
				newText = L"Exit game";
				fileName = L"exit_game";
				break;
			}
			case BACK_TO_GAME: {
				newText = L"Back to game";
				fileName = L"back_to_game";
				break;
			}
			case FREEDOM: {
				newText = L"Freedom";
				fileName = L"freedom";
				break;
			}
			case CANCEL: {
				newText = "Cancel";
				fileName = L"cancel";
				break;
			}
			case OK: {
				newText = "OK";
				fileName = L"ok";
				break;
			}
			case UNDO_CHANGES: {
				newText = "Undo changes";
				fileName = L"undo_changes";
				break;
			}
			case RESET_TO_DEFAULTS: {
				newText = "Reset to defaults";
				fileName = "reset_to_defaults";
				break;
			}
			default: {
				std::wcerr << L"Error in MenuOption::setType(): Type " << ( unsigned int ) type << " not handled in switch statement." << std::endl;
				newText = L"ERROR";
				break;
			}
		}
		
		text = newText;
		
		if( iconTexture not_eq nullptr and device not_eq nullptr ) {
			device->getVideoDriver()->removeTexture( iconTexture );
			iconTexture = nullptr;
		}
		
		if( device not_eq nullptr ) {
			loadTexture( device );
			setDimension( device->getVideoDriver() );
		}
	} catch ( std::exception &e ) {
		std::wcerr << L"Error in MenuOption::setText(): " << e.what() << std::endl;
	}
}

void MenuOption::setFontAndResizeIcon( irr::IrrlichtDevice* device, irr::gui::IGUIFont* newFont ) {
	try {
		font = newFont;
		setDimension( device->getVideoDriver() );
		
		if( not ( iconTexture == nullptr or iconTexture == NULL ) ) {
			loadTexture( device );
		}
	} catch ( std::exception &e ) {
		std::wcerr << L"Error in MenuOption::setFont(): " << e.what() << std::endl;
	}
}

void MenuOption::setDimension( irr::video::IVideoDriver* driver ) {
	try {
		dimension = irr::core::dimension2d< uint_fast16_t >( 0, 0 );
		irr::core::dimension2d< uint_fast16_t > desiredIconDimension = irr::core::dimension2d< uint_fast16_t >( 0, 0 );
		
		if( font not_eq nullptr ) {
			StringConverter sc;
			auto textDimension = font->getDimension( sc.toStdWString( text ).c_str() ); //sc.toWCharArray( text ) );
			dimension.Width = textDimension.Width + textDimension.Height; //Leaves room for a square icon
			dimension.Height = textDimension.Height;
			desiredIconDimension.Height = textDimension.Height;
		} else {
			if( iconTexture not_eq nullptr ) {
				desiredIconDimension.Height = iconTexture->getSize().Height;
			}
		}
		
		desiredIconDimension.Width = desiredIconDimension.Height;
		
		if( iconTexture not_eq nullptr ) {
			ImageModifier im;
			iconTexture = im.resize( iconTexture, desiredIconDimension.Width, desiredIconDimension.Height, driver );
		}
	} catch ( std::exception &e ) {
		std::wcerr << L"Error in MenuOption::setDimension(): " << e.what() << std::endl;
	}
}

void MenuOption::draw( irr::IrrlichtDevice* device ) {
	try {
		if( iconTexture == nullptr ) {
			loadTexture( device );
		}
		
		auto* driver = device->getVideoDriver();
		auto textX = x;
		if( iconTexture not_eq nullptr ) {
			driver->draw2DImage( iconTexture, irr::core::position2d< irr::s32 >( textX, y ) );
			textX += iconTexture->getSize().Width;
		}
		if( font not_eq nullptr ) {
			irr::core::rect< irr::s32 > background( textX, y, textX + dimension.Width - iconTexture->getSize().Width, y + dimension.Height );
			
			irr::video::SColor backgroundColor;
			irr::video::SColor textColor;
			switch( mainGame->settingsManager.colorMode ) {
				case SettingsManager::COLOR_MODE_DO_NOT_USE:
				case SettingsManager::FULLCOLOR: {
					backgroundColor = BLACK;
					
					if( highlighted ) {
						textColor = LIGHTCYAN;
					} else {
						textColor = CYAN;
					}
					
					break;
				}
				case SettingsManager::GRAYSCALE: {
					backgroundColor = BLACK_GRAYSCALE;
					
					if( highlighted ) {
						textColor = LIGHTCYAN_GRAYSCALE;
					} else {
						textColor = CYAN_GRAYSCALE;
					}
					
					break;
				}
				case SettingsManager::GREENSCALE: {
					backgroundColor = BLACK_GREENSCALE;
					
					if( highlighted ) {
						textColor = LIGHTCYAN_GREENSCALE;
					} else {
						textColor = CYAN_GREENSCALE;
					}
					
					break;
				}
				case SettingsManager::AMBERSCALE: {
					backgroundColor = BLACK_AMBERSCALE;
					
					if( highlighted ) {
						textColor = LIGHTCYAN_AMBERSCALE;
					} else {
						textColor = CYAN_AMBERSCALE;
					}
					
					break;
				}
			}
			
			driver->draw2DRectangle( backgroundColor, background );
			
			if( highlighted ) {
				driver->draw2DRectangleOutline( background, textColor );
			}
			
			font->draw( text, irr::core::rect< irr::s32 >( textX, y, dimension.Width, dimension.Height ), textColor );
		} else {
			throw( CustomException( L"Font is null" ) );
		}
	} catch( CustomException &e ) {
		std::wcerr << L"Custom exception thrown in MenuOption::draw(): " << e.what() << std::endl;
	} catch ( std::exception &e ) {
		std::wcerr << L"Error in MenuOption::draw(): " << e.what() << std::endl;
	}
}

void MenuOption::setX( uint_fast16_t val ) {
	try {
		x = val;
	} catch ( std::exception &e ) {
		std::wcerr << L"Error in MenuOption::MenuOption(): " << e.what() << std::endl;
	}
}

void MenuOption::setY( uint_fast16_t val ) {
	try {
		y = val;
	} catch ( std::exception &e ) {
		std::wcerr << L"Error in MenuOption::MenuOption(): " << e.what() << std::endl;
	}
}

uint_fast16_t MenuOption::getX() const {
	return x;
}

uint_fast16_t MenuOption::getMiddleX() const {
	return x + ( dimension.Width / 2 );
}

uint_fast16_t MenuOption::getY() const {
	return y;
}

uint_fast16_t MenuOption::getMiddleY() const {
	return y + ( dimension.Height / 2 );
}

bool MenuOption::contains( irr::core::position2d< uint_fast32_t > test ) const {
	try {
		return ( ( test.X >= x and test.Y >= y ) and ( test.X <= x + dimension.Width and test.Y <= y + dimension.Height ) );
	} catch ( std::exception &e ) {
		std::wcerr << L"Error in MenuOption::contains(): " << e.what() << std::endl;
		return false;
	}
}

bool MenuOption::contains( uint_fast32_t testX, uint_fast32_t testY ) const {
	try {
		return contains( irr::core::position2d< decltype( testX ) >( testX, testY ) );
	} catch ( std::exception &e ) {
		std::wcerr << L"Error in MenuOption::contains(): " << e.what() << std::endl;
		return false;
	}
}

void MenuOption::loadTexture( irr::IrrlichtDevice* device ) {
	try {
		if( device not_eq nullptr ) {
			irr::video::IVideoDriver* driver = device->getVideoDriver();
			if( not ( iconTexture == nullptr or iconTexture == NULL ) ) {
				driver->removeTexture( iconTexture );
				iconTexture = nullptr;
			}
			
			{
				SystemSpecificsManager system;
				auto folderList = system.getImageFolders();
				
				bool fileFound = false;
				
				for( auto listIterator = folderList.begin(); listIterator != folderList.end() and not fileFound; ++listIterator ) {
					boost::filesystem::path path = *listIterator;
					
					//Which is better: system_complete() or absolute()? On my computer they seem to do the same thing. Both are part of Boost Filesystem.
					path = system_complete( path );
					//path = absolute( path );
					
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
			
			iconTexture = driver->getTexture( fileName );
			
			if( iconTexture == nullptr or iconTexture == NULL ) {
				createTexture( device );
			}  else {
				ImageModifier im;
				
				irr::video::IVideoDriver* driver = device->getVideoDriver();
				irr::video::IImage* image = im.textureToImage( driver, iconTexture );
				irr::core::stringw textureName = iconTexture->getName().getInternalName(); //Needed when converting the image back to a texture
				driver->removeTexture( iconTexture );
				iconTexture = nullptr;
				
				mainGame->adjustImageColors( image );
				
				textureName += L"-recolored";
				iconTexture = im.imageToTexture( driver, image, textureName );
			}
			
			setDimension( device->getVideoDriver() );
		}
	} catch ( std::exception &e ) {
		std::wcerr << L"Error in MenuOption::loadTexture(): " << e.what() << std::endl;
	}
}

void MenuOption::createTexture( irr::IrrlichtDevice* device ) {
	if( device not_eq nullptr and iconTexture not_eq nullptr ) {
		device->getVideoDriver()->removeTexture( iconTexture );
		iconTexture = nullptr;
	}
	
	if( device not_eq nullptr ) {
		XPMImageLoader loader;
		setDimension( device->getVideoDriver() );
		irr::video::IImage* tempImage = device->getVideoDriver()->createImage( irr::video::ECF_A8R8G8B8, irr::core::dimension2d< irr::u32 >( dimension.Height, dimension.Height ) );
		loader.loadMenuOptionImage( device->getVideoDriver(), tempImage, type );
		ImageModifier im;
		irr::core::stringw textureName = text;
		textureName.append( L"-xpm" );
		iconTexture = im.imageToTexture( device->getVideoDriver(), tempImage, textureName );
	}
}

uint_fast16_t MenuOption::getHeight() const {
	return dimension.Height;
}

uint_fast16_t MenuOption::getWidth() const {
	return dimension.Width;
}
