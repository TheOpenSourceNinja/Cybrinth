/**
 * @file
 * @author James Dearing <dearingj@lifetime.oregonstate.edu>
 * 
 * @section LICENSE
 * Copyright © 2012-2017.
 * This file is part of Cybrinth.
 *
 * Cybrinth is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 *
 * Cybrinth is distributed 'as is' in the hope that it will be fun, but WITHOUT ANY WARRANTY; without even the implied warranty of TITLE, MERCHANTABILITY, COMPLETE DESTRUCTION OF EVIL MONSTERS, or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License along with Cybrinth. If not, see <http://www.gnu.org/licenses/>.
 * 
 * @section DESCRIPTION
 * The MainGame class is the overseer of all other classes. It's also where the game's main loop is.
 */

#include "CustomException.h"
#include "MainGame.h"
#include "MazeManager.h"
#include <boost/filesystem/fstream.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/trim_all.hpp>
#include <boost/lexical_cast.hpp>
#include <SDL.h>
#include <fileref.h>
#include <tag.h>
#include <algorithm>

//TODO: Implement an options screen (working on it: see SettingsScreen.h/.cpp)
//TODO: Add control switcher item (icon: yin-yang using players' colors?)
//TODO: Get multiplayer working online
//TODO: Improve AI. Add any solving algorithms we can think of.
//TODO: Add shader to simulate old monitor?
//TODO: Add more backgrounds.
//TODO: Support video or APNG as backgrounds?
//TODO: Add theme support (theme = (zipped?) set of backgrounds, player images, collectable images)
//TODO: If we ever add achievements, players should get an achievement for a September score (where a player's score = the current day of Eternal September)
//TODO: Add an option to use only the built-in font. This should greatly speed up loading on underpowered systems like the Pi.

/**
 * @brief Colorizes a given image according to settingsManager's colorMode
 * @param image: the image to be colorized.
 */
void MainGame::adjustImageColors( irr::video::IImage* image ) {
	
	//Now, set pixels to their desired colors (interpolate between lowPoint and colorTwo instead of the lightest and darkest colors in the original file)
	for( decltype( image->getDimension().Width ) x = 0; x < image->getDimension().Width; ++x ) {
		for( decltype( image->getDimension().Height ) y = 0; y < image->getDimension().Height; ++y ) {
			auto pixel = image->getPixel( x, y );
			if( pixel.getAlpha() > 0 ) {
				/*auto luminance = pixel.getLuminance();
				if( luminance == lightestLuminance ) {
					auto newColor = colorTwo;
					newColor.setAlpha( pixel.getAlpha() );
					image->setPixel( x, y, newColor );
				} else if( luminance < lightestLuminance and luminance > darkestLuminance ) {
					auto interpolation = ( lightestLuminance - luminance ) / 255.0f;
					auto newColor = lowPoint.getInterpolated( colorTwo, interpolation );
					image->setPixel( x, y , newColor );
				} else { // if( luminance == darkestLuminance ) {
					auto newColor = lowPoint;
					newColor.setAlpha( pixel.getAlpha() );
					image->setPixel( x, y, newColor );
				}*/
				switch( settingsManager.colorMode ) {
					case SettingsManager::COLOR_MODE_DO_NOT_USE:
					case SettingsManager::FULLCOLOR: {
						return; //no modification done
					}
					case SettingsManager::GRAYSCALE: {
						auto luminance = pixel.getLuminance();
						pixel.set( pixel.getAlpha(), luminance, luminance, luminance );
						break;
					}
					case SettingsManager::GREENSCALE: {
						auto luminance = pixel.getLuminance();
						pixel.set( pixel.getAlpha(), 0, luminance, 0 );
						break;
					}
					case SettingsManager::AMBERSCALE: {
						auto luminance = pixel.getLuminance();
						pixel.set( pixel.getAlpha(), luminance, luminance, 0 );
						break;
					}
				}
				
				image->setPixel( x, y, pixel );
			}
		}
	}
}

/**
 * Figures out which players are human, then figures out whether they're all at the goal.
 * Returns: True if there is at least one human player and all humans are at the goal, false otherwise.
 */
bool MainGame::allHumansAtGoal() {
	try {
		std::vector< uint_fast8_t > humanPlayers; //Get a list of players
		
		for( decltype( settingsManager.getNumPlayers() ) p = 0; p < settingsManager.getNumPlayers(); ++p ) {
			humanPlayers.push_back( p );
		}

		bool result = false;

		for( decltype( settingsManager.getNumBots() ) b = 0; b < settingsManager.getNumBots(); ++b ) { //Remove bots from the list
			decltype( settingsManager.getNumPlayers() ) botPlayer = bot.at( b ).getPlayer(); //changed decltype( bot.at( b ).getPlayer() ) to decltype( numPlayers ) because getPlayer can never exceed numPlayers, and this avoids needless function calls.

			for( decltype( settingsManager.getNumPlayers() ) p = 0; p < humanPlayers.size(); ++p ) { //changed decltype( humanPlayers.size() ) to decltype( numPlayers ) because humanPlayers.size() can never exceed numPlayers but can be stored in a needlessly slow integer type.
				if( humanPlayers.at( p ) == botPlayer ) {
					humanPlayers.erase( humanPlayers.begin() + p );
				}
			}
		}

		if( humanPlayers.size() > 0 ) {
			result = true;

			for( decltype( settingsManager.getNumPlayers() ) p = 0; ( p < humanPlayers.size() and result == true ); ++p ) { //changed decltype( humanPlayers.size() ) to decltype( numPlayers ) because humanPlayers.size() can never exceed numPlayers but can be stored in a needlessly slow integer type.
				if( not ( player.at( humanPlayers.at( p ) ).getX() == goal.getX() and player.at( humanPlayers.at( p ) ).getY() == goal.getY() ) ) {
					result = false;
				}
			}

		}
		return result;
	} catch( std::exception &e ) {
		std::wcerr << L"Error in MainGame::allHumansAtGoal(): " << e.what() << std::endl;
		return false;
	}
}

/**
 * If not all players are ready, displays a screen saying so.
 * @param bool tf: indicates whether all players are ready.
 */
void MainGame::allPlayersReady( bool tf ) {
	if( tf ) {
		currentScreen = MAINSCREEN;
	} else {
		currentScreen = WAITINGFORPLAYERSSCREEN;
	}
}

/**
 * Draws everything onto the screen. Calls other draw functions, including those of objects.
 */
void MainGame::drawAll() {
	try {
		
		{
			decltype( backgroundColor ) fillColor;
			switch( currentScreen ) {
				case LOADINGSCREEN: {
					fillColor = BLACK;
					break;
				}
				case SETTINGSSCREEN: {
					fillColor = settingsScreen.backgroundColor;
					break;
				}
				default: {
					fillColor = backgroundColor;
					break;
				}
			}
			
			driver->beginScene( true, true, fillColor );
		}
		
		switch( currentScreen ) {
			case SETTINGSSCREEN: {
				settingsScreen.draw( device );
				break;
			}
			case WAITINGFORPLAYERSSCREEN: {
				irr::core::stringw waitingNotice( L"Waiting for other players to finish loading" );
				irr::core::dimension2d< irr::u32 > tempDimensions = textFont->getDimension( waitingNotice.c_str() );
				irr::core::rect< irr::s32 > tempRectangle( 0, 0, tempDimensions.Width + 0, tempDimensions.Height + 0 );
				textFont->draw( waitingNotice, tempRectangle, WHITE, true, true );
				break;
			}
			case MENUSCREEN: {
				menuManager.draw( device );
				gui->drawAll();
				break;
			}
			case LOADINGSCREEN: {
				drawLoadingScreen();
				break;
			}
			case MAINSCREEN: {
				if( settingsManager.showBackgrounds ) {
					drawBackground();
				}
				
				//Draws player trails ("footprints")
				if( settingsManager.markTrails ) {
					for( decltype( mazeManager.cols ) x = 0; x < mazeManager.cols; ++x ) { //It's inefficient to do this here and have similar nested loops below drawing the walls, but I want these drawn before the players, and the players drawn before the walls.
						for( decltype( mazeManager.rows ) y = 0; y < mazeManager.rows; ++y ) {
							if( mazeManager.maze[ x ][ y ].visited ) {
								auto dotSize = cellWidth / 5;
								
								if( dotSize < 1 ) { //No point drawing these if they're less than a pixel big!
									dotSize = 1;
								}
								
								driver->draw2DRectangle( mazeManager.maze[ x ][ y ].getVisitorColor() , irr::core::rect< irr::s32 >( irr::core::position2d< irr::s32 >(( x * cellWidth ) + ( 0.5 * cellWidth ) - ( 0.5 * dotSize ), ( y * cellHeight ) + ( 0.5 * cellHeight ) - ( 0.5 * dotSize ) ), irr::core::dimension2d< irr::s32 >( dotSize, dotSize ) ) );
							}
						}
					}
				}
				
				for( decltype( settingsManager.getNumPlayers() ) ps = 0; ps < playerStart.size(); ++ps ) { //Put this in a separate loop from the players (below) so that the players would all be drawn after the playerStarts. Changed decltype( playerStart.size() ) to decltype( numPlayers ) because playerStart.size() can never exceed numPlayers but can be stored in a needlessly slow integer type.
					playerStart.at( ps ).draw( device, cellWidth, cellHeight );
				}
				
				//Drawing bots before human players makes it easier to play against large numbers of bots
				for( decltype( settingsManager.getNumBots() ) i = 0; i < settingsManager.getNumBots(); ++i ) {
					player.at( bot.at( i ).getPlayer() ).draw( device, cellWidth, cellHeight );
				}
				
				//Now we draw the players
				for( decltype( settingsManager.getNumPlayers() ) p = 0; p < settingsManager.getNumPlayers(); ++p ) {
					if( player.at( p ).isHuman ) {
						player.at( p ).draw( device, cellWidth, cellHeight );
					}
				}
				
				//We used to draw Collectables before the players due to a texture resizing bug in Irrlicht's software renderer (the bug still exists AFAIK). Collectables generally use pre-created images whereas players generally use dynamically generated images. This made players potentially get covered by Collectables and thus invisible. Now that players can hold Collectables, we want them drawn on top of the players.
				for( decltype( stuff.size() ) i = 0; i < stuff.size(); ++i ) {
					stuff.at( i ).draw( device, cellWidth, cellHeight );
				}
				
				goal.draw( device, cellWidth, cellHeight );
				
				mazeManager.draw( device, cellWidth, cellHeight );
				
				drawSidebarText();

				gui->drawAll();
			}
		}
		
		driver->endScene();
	} catch ( CustomException &e ) {
		std::wcerr << L"Error in MainGame::drawAll(): " << e.what() << std::endl;
	} catch( std::exception &e ) {
		std::wcerr << L"Error in MainGame::drawAll(): " << e.what() << std::endl;
	}
}

/**
 * Draws background animations. Assumes that driver->beginScene() has already been called. Should be called by drawAll().
 */
void MainGame::drawBackground() {
	try {
		switch( backgroundChosen ) {
			case ORIGINAL_STARFIELD:
			case ROTATING_STARFIELD: {
				backgroundSceneManager->drawAll();
				irr::core::rect< irr::s32 > pos( viewportSize.Width, 0, screenSize.Width, screenSize.Height );
				irr::core::rect< irr::s32 > clipRect = irr::core::rect< irr::s32 >( 0, 0, screenSize.Width, screenSize.Height );
				driver->draw2DRectangle( BLACK, pos, &clipRect );
				break;
			}
			case IMAGES: {
				if( backgroundTexture not_eq 0 ) {
					if( backgroundTexture->getSize() not_eq screenSize ) {
						backgroundTexture = resizer.resize( backgroundTexture, screenSize.Width, screenSize.Height, driver );
					}
					driver->draw2DImage( backgroundTexture, irr::core::position2d< irr::s32 >( 0, 0 ) );
				}
				break;
			}
			case STAR_TRAILS: {
				driver->setRenderTarget( backgroundTexture, ( fillBackgroundTextureAfterLoading and not haveFilledBackgroundTextureAfterLoading ), true, backgroundColor );
				haveFilledBackgroundTextureAfterLoading = true;
				backgroundSceneManager->drawAll();
				driver->setRenderTarget( 0, false, false, backgroundColor ); //From Irrlicht's documentation: "If set to 0, it sets the previous render target which was set before the last setRenderTarget() call."
				driver->draw2DImage( backgroundTexture, irr::core::position2d< irr::s32 >( 0, 0 ) );
				irr::core::rect< irr::s32 > pos( viewportSize.Width, 0, screenSize.Width, screenSize.Height );
				irr::core::rect< irr::s32 > clipRect = irr::core::rect< irr::s32 >( 0, 0, screenSize.Width, screenSize.Height );
				driver->draw2DRectangle( BLACK, pos, &clipRect );
				break;
			}
		}
	} catch( std::exception &e ) {
		std::wcerr << L"Error in MainGame::drawBackground(): " << e.what() << std::endl;
	}
}

/**
 * Draws the loading screen. Assumes that driver->beginScene() has already been called. Should be called by drawAll().
 */
void MainGame::drawLoadingScreen() {
	try {
		if( not haveShownLogo ) { //This is an ugly hack, but it works and I can't think of a better way to do it.
			drawLogo();
		} else {
			if( isNull( loadingFont ) ) {
				loadingFont = gui->getBuiltInFont();
			}
			if( isNull( textFont ) ) {
				textFont = gui->getBuiltInFont();
			}
			if( isNull( statsFont ) ) {
				statsFont = gui->getBuiltInFont();
			}
			
			int_fast32_t Y = 0;
			{
				auto loadingDimensions = loadingFont->getDimension( stringConverter.toStdWString( loading ).c_str() ); //stringConverter.toWCharArray( loading ) );
				int_fast32_t textX = ( screenSize.Width / 2 ) - ( loadingDimensions.Width / 2 );
				irr::core::rect< irr::s32 > tempRectangle( textX, Y, ( screenSize.Width / 2 ) + ( loadingDimensions.Width / 2 ), loadingDimensions.Height + Y );
				
				{
					irr::video::SColor color;
					
					switch( settingsManager.colorMode ) {
						case SettingsManager::COLOR_MODE_DO_NOT_USE:
						case SettingsManager::FULLCOLOR: {
							color = YELLOW;
							break;
						}
						case SettingsManager::GRAYSCALE: {
							color = YELLOW_GRAYSCALE;
							break;
						}
						case SettingsManager::GREENSCALE: {
							color = YELLOW_GREENSCALE;
							break;
						}
						case SettingsManager::AMBERSCALE: {
							color = YELLOW_AMBERSCALE;
							break;
						}
					}
					
					loadingFont->draw( loading, tempRectangle, color, true, true, &tempRectangle );
				}
				
				Y += loadingDimensions.Height + 1;
			}
			
			{
				std::wstring percentString = stringConverter.toStdWString( loadingProgress, L"%05.1f%%", 7 ); //7 is the length that L"%05.1f%%" expands to plus one extra to terminate the resulting string with a null
				auto percentDimensions = loadingFont->getDimension( percentString.c_str() );
				irr::core::recti progressBarOutline( 0, Y, screenSize.Width, Y + percentDimensions.Height );
				
				{
					irr::video::SColor color;
					
					switch( settingsManager.colorMode ) {
						case SettingsManager::COLOR_MODE_DO_NOT_USE:
						case SettingsManager::FULLCOLOR: {
							color = GRAY;
							break;
						}
						case SettingsManager::GRAYSCALE: {
							color = GRAY_GRAYSCALE;
							break;
						}
						case SettingsManager::GREENSCALE: {
							color = GRAY_GREENSCALE;
							break;
						}
						case SettingsManager::AMBERSCALE: {
							color = GRAY_AMBERSCALE;
							break;
						}
					}
					
					driver->draw2DRectangleOutline( progressBarOutline, color );
				}
				
				irr::core::recti progressBarFilled( 0, Y, screenSize.Width * loadingProgress / 100, Y + percentDimensions.Height );
				
				{
					irr::video::SColor color;
					
					switch( settingsManager.colorMode ) {
						case SettingsManager::COLOR_MODE_DO_NOT_USE:
						case SettingsManager::FULLCOLOR: {
							color = LIGHTGRAY;
							break;
						}
						case SettingsManager::GRAYSCALE: {
							color = LIGHTGRAY_GRAYSCALE;
							break;
						}
						case SettingsManager::GREENSCALE: {
							color = LIGHTGRAY_GREENSCALE;
							break;
						}
						case SettingsManager::AMBERSCALE: {
							color = LIGHTGRAY_AMBERSCALE;
							break;
						}
					}
					
					driver->draw2DRectangle( color, progressBarFilled );
				}
				
				int_fast32_t textX = ( screenSize.Width / 2 ) - ( percentDimensions.Width / 2 );
				irr::core::recti percentRectangle( textX, Y, ( screenSize.Width / 2 ) + ( percentDimensions.Width / 2 ), percentDimensions.Height + Y );
				
				{
					irr::video::SColor color;
					
					switch( settingsManager.colorMode ) {
						case SettingsManager::COLOR_MODE_DO_NOT_USE:
						case SettingsManager::FULLCOLOR: {
							color = YELLOW;
							break;
						}
						case SettingsManager::GRAYSCALE: {
							color = YELLOW_GRAYSCALE;
							break;
						}
						case SettingsManager::GREENSCALE: {
							color = YELLOW_GREENSCALE;
							break;
						}
						case SettingsManager::AMBERSCALE: {
							color = YELLOW_AMBERSCALE;
							break;
						}
					}
					
					loadingFont->draw( stringConverter.toIrrlichtStringW( percentString ), percentRectangle, color, true, true, &percentRectangle );
				}
				
				Y += percentDimensions.Height + 1;
			}
			
			if( proTips.size() > 0 ) {
				if( isNull( tipFont ) ) {
					tipFont = gui->getBuiltInFont();
				}
				
				irr::u32 proTipHeight = 0;
				
				{
					int_fast32_t textX = 0;
					irr::core::dimension2d< irr::u32 > proTipPrefixDimensions = tipFont->getDimension( stringConverter.toStdWString( proTipPrefix ).c_str() ); //stringConverter.toWCharArray( proTipPrefix ) );
					irr::core::dimension2d< irr::u32 > proTipDimensions = tipFont->getDimension( stringConverter.toStdWString( proTips.at( currentProTip ) ).c_str() ); //stringConverter.toWCharArray( proTips.at( currentProTip ) ) );
					proTipHeight = std::max( proTipDimensions.Height, proTipPrefixDimensions.Height );
					
					{
						irr::core::rect< irr::s32 > tempRectangle( textX, Y, proTipPrefixDimensions.Width + textX, proTipPrefixDimensions.Height + Y );
						irr::video::SColor color;
						
						switch( settingsManager.colorMode ) {
							case SettingsManager::COLOR_MODE_DO_NOT_USE:
							case SettingsManager::FULLCOLOR: {
								color = LIGHTCYAN;
								break;
							}
							case SettingsManager::GRAYSCALE: {
								color = LIGHTCYAN_GRAYSCALE;
								break;
							}
							case SettingsManager::GREENSCALE: {
								color = LIGHTCYAN_GREENSCALE;
								break;
							}
							case SettingsManager::AMBERSCALE: {
								color = LIGHTCYAN_AMBERSCALE;
								break;
							}
						}
						
						tipFont->draw( proTipPrefix, tempRectangle, color, true, true, &tempRectangle );
					}
					
					{
						irr::core::rect< irr::s32 > tempRectangle( textX + proTipPrefixDimensions.Width, Y, proTipDimensions.Width + textX + proTipPrefixDimensions.Width, proTipDimensions.Height + Y );
						irr::video::SColor color;
						
						switch( settingsManager.colorMode ) {
							case SettingsManager::COLOR_MODE_DO_NOT_USE:
							case SettingsManager::FULLCOLOR: {
								color = WHITE;
								break;
							}
							case SettingsManager::GRAYSCALE: {
								color = WHITE_GRAYSCALE;
								break;
							}
							case SettingsManager::GREENSCALE: {
								color = WHITE_GREENSCALE;
								break;
							}
							case SettingsManager::AMBERSCALE: {
								color = WHITE_AMBERSCALE;
								break;
							}
						}
						
						tipFont->draw( proTips.at( currentProTip ), tempRectangle, color, true, true, &tempRectangle );
					}
					
					Y += proTipHeight;
				}
			}
			
			drawStats( Y );
		}
	} catch( std::exception &e ) {
		std::wcerr << L"Error in MainGame::drawLoadingScreen(): " << e.what() << std::endl;
	}
}

/**
 * draws the logo (already loaded) on screen. Should NOT be called by drawAll().
 */
 void MainGame::drawLogo() {
	try {
		if( not isNull( logoTexture ) and logoTexture->getSize() not_eq screenSize ) {
			ImageModifier resizer;
			
			if( logoTexture->getSize() not_eq screenSize ) {
				auto newTexture = resizer.resize( logoTexture, screenSize.Width, screenSize.Height, driver );
				driver->removeTexture( logoTexture );
				logoTexture = newTexture;
			}
		}
		
		if( not isNull( logoTexture ) ) {
			driver->draw2DImage( logoTexture, irr::core::position2d< irr::s32 >( 0, 0 ) );
		}
	} catch( std::exception &error ) {
		std::wcerr << L"Error in drawLogo(): " << error.what() << std::endl;
	}
 }


/**
 * @brief Draws the sidebar text. Should only be called by drawAll().
 */
 void MainGame::drawSidebarText() {
	uint_fast32_t spaceBetween = screenSize.Height / 30;
	uint_fast32_t textY = spaceBetween;
	irr::core::dimension2d< irr::u32 > tempDimensions;
	
	{
		time_t currentTime = time( nullptr );
		size_t maxSize = std::max( settingsManager.timeFormat.length() * 2, ( size_t ) UINT_FAST8_MAX );
		wchar_t clockTime[ maxSize ];
		auto numCharsConverted = wcsftime( clockTime, maxSize, settingsManager.timeFormat.c_str(), localtime( &currentTime ) );
		
		if( numCharsConverted == 0 ) {
			numCharsConverted = wcsftime( clockTime, maxSize, settingsManager.timeFormatDefault.c_str(), localtime( &currentTime ) );
		
			if( numCharsConverted == 0 ) {
				numCharsConverted = wcsftime( clockTime, maxSize, L"%T", localtime( &currentTime ) );
				
				if( numCharsConverted == 0 ) {
					throw( CustomException( std::wstring( L"Could not convert the time to either the specified format, the default format, nor to ISO 8601." ) ) );
				}
			}
		}
		
		tempDimensions = clockFont->getDimension( clockTime );
		irr::core::rect< irr::s32 > tempRectangle( viewportSize.Width + 1, textY, tempDimensions.Width + ( viewportSize.Width + 1 ), tempDimensions.Height + textY );
		
		if( tempRectangle.LowerRightCorner.X > screenSize.Width ) {
			//If using a variable-width font, the clock size may become too big to display, so we reload the font at a smaller size
			loadClockFont();
		}
		
		irr::video::SColor color;
		switch( settingsManager.colorMode ) {
			case SettingsManager::COLOR_MODE_DO_NOT_USE:
			case SettingsManager::FULLCOLOR: {
				color = LIGHTMAGENTA;
				break;
			}
			case SettingsManager::GRAYSCALE: {
				color = LIGHTMAGENTA_GRAYSCALE;
				break;
			}
			case SettingsManager::GREENSCALE: {
				color = LIGHTMAGENTA_GREENSCALE;
				break;
			}
			case SettingsManager::AMBERSCALE: {
				color = LIGHTMAGENTA_AMBERSCALE;
				break;
			}
		}
		
		clockFont->draw( clockTime, tempRectangle, color, true, true, &tempRectangle );
	}
	
	{
		irr::core::stringw timeLabel( L"Time:" );
		textY += tempDimensions.Height;
		tempDimensions = textFont->getDimension( stringConverter.toStdWString( timeLabel ).c_str() ); //stringConverter.toWCharArray( timeLabel ) );
		irr::core::rect< irr::s32 > tempRectangle( viewportSize.Width + 1, textY, tempDimensions.Width + ( viewportSize.Width + 1 ), tempDimensions.Height + textY );
		
		irr::video::SColor color;
		switch( settingsManager.colorMode ) {
			case SettingsManager::COLOR_MODE_DO_NOT_USE:
			case SettingsManager::FULLCOLOR: {
				color = YELLOW;
				break;
			}
			case SettingsManager::GRAYSCALE: {
				color = YELLOW_GRAYSCALE;
				break;
			}
			case SettingsManager::GREENSCALE: {
				color = YELLOW_GREENSCALE;
				break;
			}
			case SettingsManager::AMBERSCALE: {
				color = YELLOW_AMBERSCALE;
				break;
			}
		}
		
		textFont->draw( L"Time:", tempRectangle, color, true, true, &tempRectangle );
	}
	
	{
		irr::core::stringw timerStr( "" );
		timerStr += ( timer->getTime() / 1000 );
		timerStr += L" seconds";
		textY += tempDimensions.Height;
		tempDimensions = textFont->getDimension( stringConverter.toStdWString( timerStr ).c_str() ); //stringConverter.toWCharArray( timerStr ) );
		irr::core::rect< irr::s32 > tempRectangle( viewportSize.Width + 1, textY, tempDimensions.Width + ( viewportSize.Width + 1 ), tempDimensions.Height + textY );
		
		
		irr::video::SColor color;
		switch( settingsManager.colorMode ) {
			case SettingsManager::COLOR_MODE_DO_NOT_USE:
			case SettingsManager::FULLCOLOR: {
				color = YELLOW;
				break;
			}
			case SettingsManager::GRAYSCALE: {
				color = YELLOW_GRAYSCALE;
				break;
			}
			case SettingsManager::GREENSCALE: {
				color = YELLOW_GREENSCALE;
				break;
			}
			case SettingsManager::AMBERSCALE: {
				color = YELLOW_AMBERSCALE;
				break;
			}
		}
		
		textFont->draw( timerStr, tempRectangle, color, true, true, &tempRectangle );
	}
	
	{
		irr::core::stringw keysFoundStr( L"Keys found:" );
		textY += tempDimensions.Height;
		tempDimensions = textFont->getDimension( stringConverter.toStdWString( keysFoundStr ).c_str() ); //stringConverter.toWCharArray( keysFoundStr ) );
		irr::core::rect< irr::s32 > tempRectangle( viewportSize.Width + 1, textY, tempDimensions.Width + ( viewportSize.Width + 1 ), tempDimensions.Height + textY );
		
		irr::video::SColor color;
		switch( settingsManager.colorMode ) {
			case SettingsManager::COLOR_MODE_DO_NOT_USE:
			case SettingsManager::FULLCOLOR: {
				color = YELLOW;
				break;
			}
			case SettingsManager::GRAYSCALE: {
				color = YELLOW_GRAYSCALE;
				break;
			}
			case SettingsManager::GREENSCALE: {
				color = YELLOW_GREENSCALE;
				break;
			}
			case SettingsManager::AMBERSCALE: {
				color = YELLOW_AMBERSCALE;
				break;
			}
		}
		
		textFont->draw( keysFoundStr, tempRectangle, color, true, true, &tempRectangle );
	}
	
	{
		irr::core::stringw keyStr;
		keyStr += numKeysFound;
		keyStr += L"/";
		keyStr += numLocks;
		textY += tempDimensions.Height;
		tempDimensions = textFont->getDimension( stringConverter.toStdWString( keyStr ).c_str() ); //stringConverter.toWCharArray( keyStr ) );
		irr::core::rect< irr::s32 > tempRectangle( viewportSize.Width + 1, textY, tempDimensions.Width + ( viewportSize.Width + 1 ), tempDimensions.Height + textY );
		
		irr::video::SColor color;
		switch( settingsManager.colorMode ) {
			case SettingsManager::COLOR_MODE_DO_NOT_USE:
			case SettingsManager::FULLCOLOR: {
				color = YELLOW;
				break;
			}
			case SettingsManager::GRAYSCALE: {
				color = YELLOW_GRAYSCALE;
				break;
			}
			case SettingsManager::GREENSCALE: {
				color = YELLOW_GREENSCALE;
				break;
			}
			case SettingsManager::AMBERSCALE: {
				color = YELLOW_AMBERSCALE;
				break;
			}
		}
		
		textFont->draw( keyStr, tempRectangle, color, true, true, &tempRectangle );
	}
	
	{
		irr::core::stringw seedLabel( L"Random seed:" );
		textY += tempDimensions.Height;
		tempDimensions = textFont->getDimension( stringConverter.toStdWString( seedLabel ).c_str() );// stringConverter.toWCharArray( seedLabel ) );
		irr::core::rect< irr::s32 > tempRectangle( viewportSize.Width + 1, textY, tempDimensions.Width + ( viewportSize.Width + 1 ), tempDimensions.Height + textY );
		
		irr::video::SColor color;
		switch( settingsManager.colorMode ) {
			case SettingsManager::COLOR_MODE_DO_NOT_USE:
			case SettingsManager::FULLCOLOR: {
				color = YELLOW;
				break;
			}
			case SettingsManager::GRAYSCALE: {
				color = YELLOW_GRAYSCALE;
				break;
			}
			case SettingsManager::GREENSCALE: {
				color = YELLOW_GREENSCALE;
				break;
			}
			case SettingsManager::AMBERSCALE: {
				color = YELLOW_AMBERSCALE;
				break;
			}
		}
		
		textFont->draw( seedLabel, tempRectangle, color, true, true, &tempRectangle );
	}
	
	{
		irr::core::stringw seedStr( randomSeed );
		textY += tempDimensions.Height;
		tempDimensions = textFont->getDimension( stringConverter.toStdWString( seedStr ).c_str() ); //stringConverter.toWCharArray( seedStr ) );
		irr::core::rect< irr::s32 > tempRectangle( viewportSize.Width + 1, textY, tempDimensions.Width + ( viewportSize.Width + 1 ), tempDimensions.Height + textY );
		
		irr::video::SColor color;
		switch( settingsManager.colorMode ) {
			case SettingsManager::COLOR_MODE_DO_NOT_USE:
			case SettingsManager::FULLCOLOR: {
				color = YELLOW;
				break;
			}
			case SettingsManager::GRAYSCALE: {
				color = YELLOW_GRAYSCALE;
				break;
			}
			case SettingsManager::GREENSCALE: {
				color = YELLOW_GREENSCALE;
				break;
			}
			case SettingsManager::AMBERSCALE: {
				color = YELLOW_AMBERSCALE;
				break;
			}
		}
		
		textFont->draw( seedStr, tempRectangle, color, true, true, &tempRectangle );
	}
	
	{
		irr::core::stringw headfor( L"Head for" );
		textY += tempDimensions.Height;
		tempDimensions = textFont->getDimension( stringConverter.toStdWString( headfor ).c_str() ); //stringConverter.toWCharArray( headfor ) );
		
		if( numKeysFound >= numLocks ) {
			irr::core::rect< irr::s32 > tempRectangle( viewportSize.Width + 1, textY, tempDimensions.Width + ( viewportSize.Width + 1 ), tempDimensions.Height + textY );
			
			irr::video::SColor color;
			switch( settingsManager.colorMode ) {
				case SettingsManager::COLOR_MODE_DO_NOT_USE:
				case SettingsManager::FULLCOLOR: {
					color = LIGHTMAGENTA;
					break;
				}
				case SettingsManager::GRAYSCALE: {
					color = LIGHTMAGENTA_GRAYSCALE;
					break;
				}
				case SettingsManager::GREENSCALE: {
					color = LIGHTMAGENTA_GREENSCALE;
					break;
				}
				case SettingsManager::AMBERSCALE: {
					color = LIGHTMAGENTA_AMBERSCALE;
					break;
				}
			}
			
			textFont->draw( headfor, tempRectangle, color, true, true, &tempRectangle );
		}
	}
	
	{
		irr::core::stringw theexit( L"the exit!" );
		textY += tempDimensions.Height;
		tempDimensions = textFont->getDimension( stringConverter.toStdWString( theexit ).c_str() ); //stringConverter.toWCharArray( theexit ) );
		
		if( numKeysFound >= numLocks ) {
			irr::core::rect< irr::s32 > tempRectangle( viewportSize.Width + 1, textY, tempDimensions.Width + ( viewportSize.Width + 1 ), tempDimensions.Height + textY );
			
			irr::video::SColor color;
			switch( settingsManager.colorMode ) {
				case SettingsManager::COLOR_MODE_DO_NOT_USE:
				case SettingsManager::FULLCOLOR: {
					color = LIGHTCYAN;
					break;
				}
				case SettingsManager::GRAYSCALE: {
					color = LIGHTCYAN_GRAYSCALE;
					break;
				}
				case SettingsManager::GREENSCALE: {
					color = LIGHTCYAN_GREENSCALE;
					break;
				}
				case SettingsManager::AMBERSCALE: {
					color = LIGHTCYAN_AMBERSCALE;
					break;
				}
			}
			
			textFont->draw( theexit, tempRectangle, color, true, true, &tempRectangle );
		}
	}
	
	if( settingsManager.getPlayMusic() ) {
		{
			irr::core::stringw nowplaying( L"Music:" );
			textY += tempDimensions.Height;
			tempDimensions = textFont->getDimension( stringConverter.toStdWString( nowplaying ).c_str() ); //stringConverter.toWCharArray( nowplaying ) );
			irr::core::rect< irr::s32 > tempRectangle( viewportSize.Width + 1, textY, tempDimensions.Width + ( viewportSize.Width + 1 ), tempDimensions.Height + textY );
			
			irr::video::SColor color;
			switch( settingsManager.colorMode ) {
				case SettingsManager::COLOR_MODE_DO_NOT_USE:
				case SettingsManager::FULLCOLOR: {
					color = YELLOW;
					break;
				}
				case SettingsManager::GRAYSCALE: {
					color = YELLOW_GRAYSCALE;
					break;
				}
				case SettingsManager::GREENSCALE: {
					color = YELLOW_GREENSCALE;
					break;
				}
				case SettingsManager::AMBERSCALE: {
					color = YELLOW_AMBERSCALE;
					break;
				}
			}
			
			textFont->draw( nowplaying, tempRectangle, color, true, true, &tempRectangle );
		}
		
		{
			textY += tempDimensions.Height;
			tempDimensions = musicTagFont->getDimension( stringConverter.toStdWString( musicTitle ).c_str() ); //stringConverter.toWCharArray( musicTitle ) );
			irr::core::rect< irr::s32 > tempRectangle( viewportSize.Width + 1, textY, tempDimensions.Width + ( viewportSize.Width + 1 ), tempDimensions.Height + textY );
			
			irr::video::SColor color;
			switch( settingsManager.colorMode ) {
				case SettingsManager::COLOR_MODE_DO_NOT_USE:
				case SettingsManager::FULLCOLOR: {
					color = LIGHTGREEN;
					break;
				}
				case SettingsManager::GRAYSCALE: {
					color = LIGHTGREEN_GRAYSCALE;
					break;
				}
				case SettingsManager::GREENSCALE: {
					color = LIGHTGREEN_GREENSCALE;
					break;
				}
				case SettingsManager::AMBERSCALE: {
					color = LIGHTGREEN_AMBERSCALE;
					break;
				}
			}
			
			musicTagFont->draw( musicTitle, tempRectangle, color, true, true, &tempRectangle );
		}
		
		{
			irr::core::stringw by( L"by" );
			textY += tempDimensions.Height;
			tempDimensions = textFont->getDimension( stringConverter.toStdWString( by ).c_str() ); //stringConverter.toWCharArray( by ) );
			irr::core::rect< irr::s32 > tempRectangle( viewportSize.Width + 1, textY, tempDimensions.Width + ( viewportSize.Width + 1 ), tempDimensions.Height + textY );
			
			irr::video::SColor color;
			switch( settingsManager.colorMode ) {
				case SettingsManager::COLOR_MODE_DO_NOT_USE:
				case SettingsManager::FULLCOLOR: {
					color = YELLOW;
					break;
				}
				case SettingsManager::GRAYSCALE: {
					color = YELLOW_GRAYSCALE;
					break;
				}
				case SettingsManager::GREENSCALE: {
					color = YELLOW_GREENSCALE;
					break;
				}
				case SettingsManager::AMBERSCALE: {
					color = YELLOW_AMBERSCALE;
					break;
				}
			}
			
			textFont->draw( by, tempRectangle, color, true, true, &tempRectangle );
		}
		
		{
			textY += tempDimensions.Height;
			tempDimensions = musicTagFont->getDimension( stringConverter.toStdWString( musicArtist ).c_str() ); //stringConverter.toWCharArray( musicArtist ) );
			irr::core::rect< irr::s32 > tempRectangle( viewportSize.Width + 1, textY, tempDimensions.Width + ( viewportSize.Width + 1 ), tempDimensions.Height + textY );
			
			irr::video::SColor color;
			switch( settingsManager.colorMode ) {
				case SettingsManager::COLOR_MODE_DO_NOT_USE:
				case SettingsManager::FULLCOLOR: {
					color = LIGHTGREEN;
					break;
				}
				case SettingsManager::GRAYSCALE: {
					color = LIGHTGREEN_GRAYSCALE;
					break;
				}
				case SettingsManager::GREENSCALE: {
					color = LIGHTGREEN_GREENSCALE;
					break;
				}
				case SettingsManager::AMBERSCALE: {
					color = LIGHTGREEN_AMBERSCALE;
					break;
				}
			}
			
			musicTagFont->draw( musicArtist, tempRectangle, color, true, true, &tempRectangle );
		}
		
		{
			irr::core::stringw fromalbum( L"from album" );
			textY += tempDimensions.Height;
			tempDimensions = textFont->getDimension( stringConverter.toStdWString( fromalbum ).c_str() ); //stringConverter.toWCharArray( fromalbum ) );
			irr::core::rect< irr::s32 > tempRectangle( viewportSize.Width + 1, textY, tempDimensions.Width + ( viewportSize.Width + 1 ), tempDimensions.Height + textY );
			
			irr::video::SColor color;
			switch( settingsManager.colorMode ) {
				case SettingsManager::COLOR_MODE_DO_NOT_USE:
				case SettingsManager::FULLCOLOR: {
					color = YELLOW;
					break;
				}
				case SettingsManager::GRAYSCALE: {
					color = YELLOW_GRAYSCALE;
					break;
				}
				case SettingsManager::GREENSCALE: {
					color = YELLOW_GREENSCALE;
					break;
				}
				case SettingsManager::AMBERSCALE: {
					color = YELLOW_AMBERSCALE;
					break;
				}
			}
			
			textFont->draw( fromalbum, tempRectangle, color, true, true, &tempRectangle );
		}
		
		{
			textY += tempDimensions.Height;
			tempDimensions = musicTagFont->getDimension( stringConverter.toStdWString( musicAlbum ).c_str() ); //stringConverter.toWCharArray( musicAlbum ) );
			irr::core::rect< irr::s32 > tempRectangle( viewportSize.Width + 1, textY, tempDimensions.Width + ( viewportSize.Width + 1 ), tempDimensions.Height + textY );
			
			irr::video::SColor color;
			switch( settingsManager.colorMode ) {
				case SettingsManager::COLOR_MODE_DO_NOT_USE:
				case SettingsManager::FULLCOLOR: {
					color = LIGHTGRAY;
					break;
				}
				case SettingsManager::GRAYSCALE: {
					color = LIGHTGRAY_GRAYSCALE;
					break;
				}
				case SettingsManager::GREENSCALE: {
					color = LIGHTGRAY_GREENSCALE;
					break;
				}
				case SettingsManager::AMBERSCALE: {
					color = LIGHTGRAY_AMBERSCALE;
					break;
				}
			}
			
			musicTagFont->draw( musicAlbum, tempRectangle, color, true, true, &tempRectangle );
		}
		
		{
			irr::core::stringw volume( L"Volume:" );
			textY += tempDimensions.Height;
			tempDimensions = textFont->getDimension( stringConverter.toStdWString( volume ).c_str() ); //stringConverter.toWCharArray( volume ) );
			irr::core::rect< irr::s32 > tempRectangle( viewportSize.Width + 1, textY, tempDimensions.Width + ( viewportSize.Width + 1 ), tempDimensions.Height + textY );
			
			irr::video::SColor color;
			switch( settingsManager.colorMode ) {
				case SettingsManager::COLOR_MODE_DO_NOT_USE:
				case SettingsManager::FULLCOLOR: {
					color = YELLOW;
					break;
				}
				case SettingsManager::GRAYSCALE: {
					color = YELLOW_GRAYSCALE;
					break;
				}
				case SettingsManager::GREENSCALE: {
					color = YELLOW_GREENSCALE;
					break;
				}
				case SettingsManager::AMBERSCALE: {
					color = YELLOW_AMBERSCALE;
					break;
				}
			}
			
			textFont->draw( volume, tempRectangle, color, true, true, &tempRectangle );
		}
		
		{
			irr::core::stringw volumeNumber( settingsManager.getMusicVolume() );
			volumeNumber.append( L"%" );
			textY += tempDimensions.Height;
			tempDimensions = textFont->getDimension( stringConverter.toStdWString( volumeNumber ).c_str() ); //stringConverter.toWCharArray( volumeNumber ) );
			irr::core::rect< irr::s32 > tempRectangle( viewportSize.Width + 1, textY, tempDimensions.Width + ( viewportSize.Width + 1 ), tempDimensions.Height + textY );
			
			irr::video::SColor color;
			switch( settingsManager.colorMode ) {
				case SettingsManager::COLOR_MODE_DO_NOT_USE:
				case SettingsManager::FULLCOLOR: {
					color = LIGHTRED;
					break;
				}
				case SettingsManager::GRAYSCALE: {
					color = LIGHTRED_GRAYSCALE;
					break;
				}
				case SettingsManager::GREENSCALE: {
					color = LIGHTRED_GREENSCALE;
					break;
				}
				case SettingsManager::AMBERSCALE: {
					color = LIGHTRED_AMBERSCALE;
					break;
				}
			}
			
			textFont->draw( volumeNumber, tempRectangle, color, true, true, &tempRectangle );
		}
	}
 }

/**
 * Should only be called from drawLoadingScreen(). Just putting it here for code separation/readability.
 * //TODO: Add more stats (estimated difficulty of maze, number of cells backtracked, etc) to loading screen.
 */
void MainGame::drawStats( uint_fast32_t textY ) {
	try {
		if( isNull( statsFont ) ) {
			statsFont = gui->getBuiltInFont();
		}

		uint_fast16_t textX = 0;
		decltype( textX ) textXOriginal = textX;
		decltype( textY ) textYOriginal = textY;
		decltype( textYOriginal ) textYSteps;// = textYOriginal;
		decltype( textYOriginal ) textYTimes;// = textYOriginal;
		decltype( textYOriginal ) textYKeys;// = textYOriginal;
		decltype( textYOriginal ) textYScores;// = textYOriginal;
		decltype( textYOriginal ) textYScoresTotal;// = textYOriginal;
		//To determine how tall each row of text is, we draw the row labels first (their text could conceivably have hangy-down bits like a lower-case y)
		{
			decltype( statsFont->getDimension( L"" ) ) tempDimensions = statsFont->getDimension( stringConverter.toStdWString( winnersLabel ).c_str() ); //stringConverter.toWCharArray( winnersLabel ) );
			irr::core::rect< irr::s32 > tempRectangle( textXOriginal, textY, tempDimensions.Width + textXOriginal, tempDimensions.Height + textY );
			
			irr::video::SColor color;
			switch( settingsManager.colorMode ) {
				case SettingsManager::COLOR_MODE_DO_NOT_USE:
				case SettingsManager::FULLCOLOR: {
					color = WHITE;
					break;
				}
				case SettingsManager::GRAYSCALE: {
					color = WHITE_GRAYSCALE;
					break;
				}
				case SettingsManager::GREENSCALE: {
					color = WHITE_GREENSCALE;
					break;
				}
				case SettingsManager::AMBERSCALE: {
					color = WHITE_AMBERSCALE;
					break;
				}
			}
			
			statsFont->draw( winnersLabel, tempRectangle, color, true, true, &tempRectangle );

			if( tempDimensions.Width + textXOriginal > textX ) {
				textX = tempDimensions.Width + textXOriginal;
			}
			textYSteps = textYOriginal + tempDimensions.Height;
		}

		{
			decltype( statsFont->getDimension( L"" ) ) tempDimensions = statsFont->getDimension( stringConverter.toStdWString( steps ).c_str() );// stringConverter.toWCharArray( steps ) );
			irr::core::rect< irr::s32 > tempRectangle( textXOriginal, textYSteps, tempDimensions.Width + textXOriginal, tempDimensions.Height + textYSteps );
			
			irr::video::SColor color;
			switch( settingsManager.colorMode ) {
				case SettingsManager::COLOR_MODE_DO_NOT_USE:
				case SettingsManager::FULLCOLOR: {
					color = WHITE;
					break;
				}
				case SettingsManager::GRAYSCALE: {
					color = WHITE_GRAYSCALE;
					break;
				}
				case SettingsManager::GREENSCALE: {
					color = WHITE_GREENSCALE;
					break;
				}
				case SettingsManager::AMBERSCALE: {
					color = WHITE_AMBERSCALE;
					break;
				}
			}
			
			statsFont->draw( steps, tempRectangle, color, true, true, &tempRectangle );

			if( tempDimensions.Width + textXOriginal > textX ) {
				textX = tempDimensions.Width + textXOriginal;
			}
			textYTimes = textYSteps + tempDimensions.Height;
		}

		{
			decltype( statsFont->getDimension( L"" ) ) tempDimensions = statsFont->getDimension( stringConverter.toStdWString( times ).c_str() ); //stringConverter.toWCharArray( times ) );
			irr::core::rect< irr::s32 > tempRectangle( textXOriginal, textYTimes, tempDimensions.Width + textXOriginal, tempDimensions.Height + textYTimes );
			
			irr::video::SColor color;
			switch( settingsManager.colorMode ) {
				case SettingsManager::COLOR_MODE_DO_NOT_USE:
				case SettingsManager::FULLCOLOR: {
					color = WHITE;
					break;
				}
				case SettingsManager::GRAYSCALE: {
					color = WHITE_GRAYSCALE;
					break;
				}
				case SettingsManager::GREENSCALE: {
					color = WHITE_GREENSCALE;
					break;
				}
				case SettingsManager::AMBERSCALE: {
					color = WHITE_AMBERSCALE;
					break;
				}
			}
			
			statsFont->draw( times, tempRectangle, color, true, true, &tempRectangle );

			if( tempDimensions.Width + textXOriginal > textX ) {
				textX = tempDimensions.Width + textXOriginal;
			}
			textYKeys = textYTimes + tempDimensions.Height;
		}

		{
			decltype( statsFont->getDimension( L"" ) ) tempDimensions = statsFont->getDimension( stringConverter.toStdWString( keysFoundPerPlayer ).c_str() ); //stringConverter.toWCharArray( keysFoundPerPlayer ) );
			irr::core::rect< irr::s32 > tempRectangle( textXOriginal, textYKeys, tempDimensions.Width + textXOriginal, tempDimensions.Height + textYKeys );
			
			irr::video::SColor color;
			switch( settingsManager.colorMode ) {
				case SettingsManager::COLOR_MODE_DO_NOT_USE:
				case SettingsManager::FULLCOLOR: {
					color = WHITE;
					break;
				}
				case SettingsManager::GRAYSCALE: {
					color = WHITE_GRAYSCALE;
					break;
				}
				case SettingsManager::GREENSCALE: {
					color = WHITE_GREENSCALE;
					break;
				}
				case SettingsManager::AMBERSCALE: {
					color = WHITE_AMBERSCALE;
					break;
				}
			}
			
			statsFont->draw( keysFoundPerPlayer, tempRectangle, color, true, true, &tempRectangle );

			if( tempDimensions.Width + textXOriginal > textX ) {
				textX = tempDimensions.Width + textXOriginal;
			}
			textYScores = textYKeys + tempDimensions.Height;
		}

		{
			decltype( statsFont->getDimension( L"" ) ) tempDimensions = statsFont->getDimension( stringConverter.toStdWString( scores ).c_str() ); //stringConverter.toWCharArray( scores ) );
			irr::core::rect< irr::s32 > tempRectangle( textXOriginal, textYScores, tempDimensions.Width + textXOriginal, tempDimensions.Height + textYScores );
			
			irr::video::SColor color;
			switch( settingsManager.colorMode ) {
				case SettingsManager::COLOR_MODE_DO_NOT_USE:
				case SettingsManager::FULLCOLOR: {
					color = WHITE;
					break;
				}
				case SettingsManager::GRAYSCALE: {
					color = WHITE_GRAYSCALE;
					break;
				}
				case SettingsManager::GREENSCALE: {
					color = WHITE_GREENSCALE;
					break;
				}
				case SettingsManager::AMBERSCALE: {
					color = WHITE_AMBERSCALE;
					break;
				}
			}
			
			statsFont->draw( scores, tempRectangle, color, true, true, &tempRectangle );

			if( tempDimensions.Width + textXOriginal > textX ) {
				textX = tempDimensions.Width + textXOriginal;
			}
			textYScoresTotal = textYScores + tempDimensions.Height;
		}

		{
			decltype( statsFont->getDimension( L"" ) ) tempDimensions = statsFont->getDimension( stringConverter.toStdWString( scoresTotal ).c_str() ); //stringConverter.toWCharArray( scoresTotal ) );
			irr::core::rect< irr::s32 > tempRectangle( textXOriginal, textYScoresTotal, tempDimensions.Width + textXOriginal, tempDimensions.Height + textYScoresTotal );
			
			irr::video::SColor color;
			switch( settingsManager.colorMode ) {
				case SettingsManager::COLOR_MODE_DO_NOT_USE:
				case SettingsManager::FULLCOLOR: {
					color = WHITE;
					break;
				}
				case SettingsManager::GRAYSCALE: {
					color = WHITE_GRAYSCALE;
					break;
				}
				case SettingsManager::GREENSCALE: {
					color = WHITE_GREENSCALE;
					break;
				}
				case SettingsManager::AMBERSCALE: {
					color = WHITE_AMBERSCALE;
					break;
				}
			}
			
			statsFont->draw( scoresTotal, tempRectangle, color, true, true, &tempRectangle );

			if( tempDimensions.Width + textXOriginal > textX ) {
				textX = tempDimensions.Width + textXOriginal;
			}
			//textYScoresTotal = textYScores + tempDimensions.Height;
		}

		textY = textYOriginal;
		//Now we go through and draw the actual player stats
		for( decltype( settingsManager.getNumPlayers() ) p = 0; p < winnersLoadingScreen.size(); ++p ) { //changed decltype( winnersLoadingScreen.size() ) to decltype( numPlayers ) because winnersLoadingScreen.size() can never exceed numPlayers but can be stored in a needlessly slow integer type.
			auto textXOld = textX;
			
			auto backgroundColor = player.at( winnersLoadingScreen.at( p ) ).getColorOne();
			auto textColor = player.at( winnersLoadingScreen.at( p ) ).getColorTwo();
			
			//Text wrapping!
			{
				irr::core::stringw dummy( winnersLoadingScreen.size() );
				dummy.append( L".P" );
				dummy.append( irr::core::stringw( winnersLoadingScreen.size() ) );
				dummy.append( L" " );
				auto tempDimensions = statsFont->getDimension( stringConverter.toStdWString( dummy ).c_str() );
				if( textX >= screenSize.Width - tempDimensions.Width ) {
					textX = 0;
					textXOld = 0;
					decltype( textY ) separatorY = textYScoresTotal + tempDimensions.Height + 1;
					driver->draw2DLine( irr::core::position2d< irr::s32  >( 0, separatorY ), irr::core::position2d< irr::s32 >( screenSize.Width, separatorY ) );
					textY = separatorY + 1;
					textYSteps += ( textY - textYOriginal );
					textYTimes += ( textY - textYOriginal );
					textYKeys += ( textY - textYOriginal );
					textYScores += ( textY - textYOriginal );
					textYScoresTotal += ( textY - textYOriginal );
					textYOriginal = textY;
				}
			}
			
			{ //First we identify the players
				irr::core::stringw text( p );
				text.append( L".P" );
				text.append( irr::core::stringw( winnersLoadingScreen.at( p ) ) );
				text.append( L" " );
				auto tempDimensions = statsFont->getDimension( stringConverter.toStdWString( text ).c_str() ); //stringConverter.toWCharArray( text ) );
				irr::core::rect< irr::s32 > tempRectangle( textXOld, textY, tempDimensions.Width + textXOld, tempDimensions.Height + textY );
				driver->draw2DRectangle( backgroundColor, tempRectangle );
				statsFont->draw( text, tempRectangle, textColor, true, true, &tempRectangle );
				if( tempDimensions.Width + textXOld > textX ) {
					textX = tempDimensions.Width + textXOld;
				}
			}
			{ //Now we show how many steps each player took
				irr::core::stringw text( player.at( winnersLoadingScreen.at( p ) ).stepsTakenLastMaze );
				auto tempDimensions = statsFont->getDimension( stringConverter.toStdWString( text ).c_str() ); //stringConverter.toWCharArray( text ) );
				irr::core::rect< irr::s32 > tempRectangle( textXOld, textYSteps, tempDimensions.Width + textXOld, tempDimensions.Height + textYSteps );
				driver->draw2DRectangle( backgroundColor, tempRectangle );
				statsFont->draw( text, tempRectangle, textColor, true, true, &tempRectangle );
				if( tempDimensions.Width + textXOld > textX ) {
					textX = tempDimensions.Width + textXOld;
				}
			}
			{ //Now we show how long each player took in seconds
				irr::core::stringw text( player.at( winnersLoadingScreen.at( p ) ).timeTakenLastMaze / 1000 );
				auto tempDimensions = statsFont->getDimension( stringConverter.toStdWString( text ).c_str() ); //stringConverter.toWCharArray( text ) );
				irr::core::rect< irr::s32 > tempRectangle( textXOld, textYTimes, tempDimensions.Width + textXOld, tempDimensions.Height + textYTimes );
				driver->draw2DRectangle( backgroundColor, tempRectangle );
				statsFont->draw( text, tempRectangle, textColor, true, true, &tempRectangle );
				if( tempDimensions.Width + textXOld > textX ) {
					textX = tempDimensions.Width + textXOld;
				}
			}
			{ //Now we show how many keys each player collected
				irr::core::stringw text( player.at( winnersLoadingScreen.at( p ) ).keysCollectedLastMaze );
				auto tempDimensions = statsFont->getDimension( stringConverter.toStdWString( text ).c_str() ); //stringConverter.toWCharArray( text ) );
				irr::core::rect< irr::s32 > tempRectangle( textXOld, textYKeys, tempDimensions.Width + textXOld, tempDimensions.Height + textYKeys );
				driver->draw2DRectangle( backgroundColor, tempRectangle );
				statsFont->draw( text, tempRectangle, textColor, true, true, &tempRectangle );
				if( tempDimensions.Width + textXOld > textX ) {
					textX = tempDimensions.Width + textXOld;
				}
			}
			{ //Finally, each player's score is shown...
				irr::core::stringw text = stringConverter.toIrrlichtStringW( player.at( winnersLoadingScreen.at( p ) ).getScoreLastMaze() );
				//text.append( player.at( winnersLoadingScreen.at( p ) ).getScoreLastMaze() );
				auto tempDimensions = statsFont->getDimension( stringConverter.toStdWString( text ).c_str() ); //stringConverter.toWCharArray( text ) );
				irr::core::rect< irr::s32 > tempRectangle( textXOld, textYScores, tempDimensions.Width + textXOld, tempDimensions.Height + textYScores );
				driver->draw2DRectangle( backgroundColor, tempRectangle );
				statsFont->draw( text, tempRectangle, textColor, true, true, &tempRectangle );
				if( tempDimensions.Width + textXOld > textX ) {
					textX = tempDimensions.Width + textXOld;
				}
			}
			{ //...followed by the totals.
				irr::core::stringw text = stringConverter.toIrrlichtStringW( player.at( winnersLoadingScreen.at( p ) ).getScoreTotal() );
				//text.append( player.at( winnersLoadingScreen.at( p ) ).getScoreTotal() );
				auto tempDimensions = statsFont->getDimension( stringConverter.toStdWString( text ).c_str() ); //stringConverter.toWCharArray( text ) );
				irr::core::rect< irr::s32 > tempRectangle( textXOld, textYScoresTotal, tempDimensions.Width + textXOld, tempDimensions.Height + textYScoresTotal );
				driver->draw2DRectangle( backgroundColor, tempRectangle );
				statsFont->draw( text, tempRectangle, textColor, true, true, &tempRectangle );
				if( tempDimensions.Width + textXOld > textX ) {
					textX = tempDimensions.Width + textXOld;
				}
			}
		}
	} catch ( std::exception &error ) {
		std::wcerr << L"Error in drawStats(): " << error.what() << std::endl;
	}
}

/**
 * Removes one item from stuff.
 */
void MainGame::eraseCollectable( uint_fast8_t item ) {
	try {
		if( settingsManager.debug ) {
			std::wcout << L"eraseCollectable() called" << std::endl;
		}
		
		if( item < stuff.size() ) {
			stuff.erase( stuff.begin() + item );
			for( decltype( settingsManager.getNumPlayers() ) p = 0; p < player.size(); ++p ) {
				if( player.at( p ).hasItem() and player.at( p ).getItem() > item ) {
					player.at( p ).giveItem( player.at( p ).getItem() - 1, player.at( p ).getItemType() );
				}
			}
		}
		if( settingsManager.debug ) {
			std::wcout << L"end of eraseCollectable()" << std::endl;
		}
	} catch ( std::exception &error ) {
		std::wcerr << L"Error in eraseCollectable(): " << error.what() << std::endl;
	}
}

/**
 * Lets other objects get a pointer to one of the collectables, probably to see if a player has touched one.
 * @param uint_fast8_t collectable: The number of the item desired.
 * @return A pointer to a Collectable.
 */
Collectable* MainGame::getCollectable( uint_fast8_t collectable ) {
	try {
		return &stuff.at( collectable );
	} catch( std::exception &e ) {
		std::wcerr << L"Error in MainGame::getCollectable(): " << e.what() << std::endl;
	}
	return nullptr;
}

/**
 * Lets other objects know whether we're in debug mode.
 * Returns: True if debug is true, false otherwise.
 */
bool MainGame::getDebugStatus() {
	try {
		return settingsManager.debug;
	} catch( std::exception &e ) {
		std::wcerr << L"Error in MainGame::getDebugStatus(): " << e.what() << std::endl;
		return true;
	}
}

/**
 * Lets other objects get a pointer to the goal, perhaps to get its location.
 * Returns: A pointer to the goal object.
 */
Goal* MainGame::getGoal() {
	return &goal;
}

/**
 * Lets other objects get a pointer to one of the keys, probably to see if a player has touched one.
 * @param uint_fast8_t key: The number of the key desired.
 * @return A pointer to a key.
 */
Collectable* MainGame::getKey( uint_fast8_t key ) {
	try {
		if( settingsManager.debug ) {
			std::wcout << L"getKey() called" << std::endl;
		}
		
		if( key <= getNumKeys() ) {
			Collectable* result = nullptr;
			decltype( key ) currentKey = 0;

			for( decltype( stuff.size() ) s = 0; s < stuff.size(); ++s ) {
				if( stuff.at( s ).getType() == Collectable::KEY ) {
					if( currentKey == key ) {
						result = &stuff.at( s );
					}
					currentKey++;
				}
			}
			
			if( settingsManager.debug ) {
				std::wcout << L"end of getKey() (key is valid)" << std::endl;
			}
			
			return result;
		} else {
			std::wstring error = L"Collectable ";
			error += static_cast< unsigned int >( key );
			error += L" is not a key.";
			throw CustomException( error );
		}
	} catch( std::exception &e ) {
		std::wcerr << L"Error in MainGame::getKey(): " << e.what() << std::endl;
	}
	
	if( settingsManager.debug ) {
		std::wcout << L"end of getKey() (key is not valid)" << std::endl;
	}
	return nullptr;
}

/**
 * Split this into its own function because it's needed in multiple places now
 */
std::vector< boost::filesystem::path > MainGame::getLoadableTexturesList( boost::filesystem::path searchLocation ) {
	//Which is better: system_complete() or absolute()? On my computer they seem to do the same thing. Both are part of Boost Filesystem.
	searchLocation = system_complete( searchLocation );
	
	std::vector< boost::filesystem::path > textureList;
	
	if( exists( searchLocation ) ) {
		boost::filesystem::recursive_directory_iterator end;
	
		for( boost::filesystem::recursive_directory_iterator i( searchLocation ); i not_eq end; ++i ) {
			if( not is_directory( i->path() ) ) { //We've found a file
				if( settingsManager.debug ) {
					std::wcout << i->path().wstring() << std::endl;
				}
				
				//Asks Irrlicht if the file is loadable. This way the game is certain to accept any file formats the library can use.
				for( decltype( driver->getImageLoaderCount() ) loaderNum = 0; loaderNum < driver->getImageLoaderCount(); ++loaderNum ) { //Irrlicht uses a different image loader for each file type. Loop through them all, ask each if it can load the file.
					
					irr::video::IImageLoader* loader = driver->getImageLoader( loaderNum );
					irr::io::IFileSystem* fileSystem = device->getFileSystem();
					irr::io::path filePath = stringConverter.toIrrlichtStringW( i->path().wstring() );
					
					//if( loader->isALoadableFileExtension( filePath ) ) { //Commenting this out because extensions don't always reflect the file's contents. Uncomment it for a minor speed improvement since not all files would need to be opened.
						irr::io::IReadFile* file = fileSystem->createAndOpenFile( filePath );
						if( not isNull( file ) ) {
							if( loader->isALoadableFileFormat( file ) ) {
								textureList.push_back( i->path() );
								file->drop();
								break;
							}
							file->drop();
						}
					//}
				}
			}
		}
	}
	
	return textureList;
}

/**
 * Other objects can't properly add to the loading percentage if they can't see what it is first
 */
float MainGame::getLoadingPercentage() {
	return loadingProgress;
}

/**
 * Returns the highest number that the random number generator can output
 **/
std::minstd_rand::result_type MainGame::getMaxRandomNumber() {
	return randomNumberGenerator.max();
}

/**
 * Lets other objects get a pointer to the maze manager, perhaps to get the maze.
 * Returns: A pointer to the mazeManager object.
 */
MazeManager* MainGame::getMazeManager() {
	try {
		return &mazeManager;
	} catch( std::exception &e ) {
		std::wcerr << L"Error in MainGame::getMazeManager(): " << e.what() << std::endl;
	}
	return nullptr;
}

/**
 * Lets other objects know how many collectables there are.
 * Returns: the number of Collectables.
 */
 uint_fast8_t MainGame::getNumCollectables() {
	return stuff.size();
 }

/**
 * Lets other objects know how many locks there are.
 * Returns: the number of keys.
 */
uint_fast8_t MainGame::getNumKeys() {
	try {
		
		uint_fast8_t result = 0;
		for( decltype( stuff.size() ) s = 0; s < stuff.size(); ++s ) {
			if( stuff.at( s ).getType() == Collectable::KEY ) {
				result++;
			}
		}
		
		return result;
	} catch( std::exception &e ) {
		std::wcerr << L"Error in MainGame::getNumKeys(): " << e.what() << std::endl;
		return UINT_FAST8_MAX;
	}
}

/**
 * Lets other objects get a pointer to a player object.
 * Arguments:
 * --- uint_fast8_t p: the desired player
 * Returns: A pointer to the desired player if that player exists and if no exception is caught, nullptr otherwise.
 **/
Player* MainGame::getPlayer( uint_fast8_t p ) {
	try {
		if( p < settingsManager.getNumPlayers() ) {
			return &player.at( p );
		} else {
			throw( CustomException( std::wstring( L"Request for player (" ) + stringConverter.toStdWString( p ) + L") >= settingsManager.getNumPlayers() (" + stringConverter.toStdWString( settingsManager.getNumPlayers() ) + L")" ) );
		}
	} catch( std::exception &e ) {
		std::wcerr << L"Error in MainGame::getPlayer(): " << e.what() << std::endl;
		std::wcerr << L"p is: " << (int) p << L" settingsManager.getNumPlayers(): " << (int) settingsManager.getNumPlayers() << L" player.size(): " << (int) player.size() << std::endl;
		return nullptr;
	}
}

/**
 * Lets objects get random numbers using this object's generator.
 **/
std::minstd_rand::result_type MainGame::getRandomNumber() {
	return randomNumberGenerator();
}

/**
 * Lets objects see what the RNG was seeded with.
 **/
std::minstd_rand::result_type MainGame::getRandomSeed() {
	return randomSeed;
}

/**
 * Lets objects see what the screen size is.
 **/
irr::core::dimension2d< irr::u32 > MainGame::getScreenSize() {
	return screenSize;
}

/**
 * Lets other objects get a pointer to the player start objects.
 * Arguments:
 * --- uint_fast8_t ps: The desired player start
 * Returns: A pointer to the desired player start object if it exists, nullptr otherwise.
 */
PlayerStart* MainGame::getStart( uint_fast8_t ps ) {
	try {
		if( settingsManager.debug ) {
			std::wcout << L"getStart() called" << std::endl;
		}
		
		return &playerStart.at( ps );
	} catch( std::exception &e ) {
		std::wcerr << L"Error in MainGame::getStart(): " << e.what() << std::endl;
		return nullptr;
	}
}

/**
 * @brief Does what the name says: initializes a bunch of variables. I made this function because I'm trying to shorten the MainGame() constructor.
 */
void MainGame::initializeVariables( bool runAsScreenSaver ) {
	#ifdef DEBUGFLAG //Not the last place debug is set to true or false; look at readPrefs()
		settingsManager.debug = true;
	#else
		settingsManager.debug = false;
	#endif
	if( settingsManager.debug ) {
		std::wcout << L"MainGame constructor called" << std::endl;
	}
	//Just wanted to be totally sure that these point to nullptr before.
	clockFont = nullptr;
	loadingFont = nullptr;
	musicTagFont = nullptr;
	statsFont = nullptr;
	textFont = nullptr;
	tipFont = nullptr;
	backgroundTexture = nullptr;
	loadMazeDialog = nullptr;
	saveMazeDialog = nullptr;
	exitConfirmation = nullptr;
	logoTexture = nullptr;
	device = nullptr;
	gui = nullptr;
	currentScreen = LOADINGSCREEN;
	loading = L"Loading...";
	proTipPrefix = L"Pro tip: ";
	winnersLabel = L"Winners: ";
	steps = L"Steps: ";
	times = L"Times: ";
	keysFoundPerPlayer = L"Keys found: ";
	scores = L"Scores: ";
	scoresTotal = L"Total scores: ";
	mazeManager.setPointers( this, &settingsManager );
	settingsManager.isServer = false;
	antiAliasFonts = true;
	currentExitConfirmation = 0;
	currentProTip = 0;
	sideDisplaySizeDenominator = 6; //What fraction of the screen's width is set aside for displaying text, statistics, etc. during play.
	currentDirectory = boost::filesystem::current_path();
	haveShownLogo = false;
	donePlaying = false;
	fillBackgroundTextureAfterLoading = false;
	haveFilledBackgroundTextureAfterLoading = false;
	lastTimeControlsProcessed = 0;
	controlProcessDelay = 100;
	backgroundColor = BLACK; //Every background should set this in setupBackground(); putting it here just in case.
	backgroundFilePath = L"";
	music = nullptr;
	isScreenSaver = runAsScreenSaver;
	enableController = false; //This gets set in setControls(), but only if that function gets called.
}

/**
 * Checks whether a pointer is equal to any of the values likely to represent null.
 * Arguments:
 * --- void* ptr: Some pointer.
 * Returns: Guess.
 */
bool MainGame::isNull( void* ptr ) {
	// cppcheck-suppress duplicateExpression
	return ( ptr == 0 or ptr == NULL or ptr == nullptr );
}

/**
 * @brief Loads the clock font.
 */
void MainGame::loadClockFont() { //Load clockFont
	irr::core::dimension2d< uint_fast32_t > fontDimensions;
	if( fontFile not_eq "" ) {
		uint_fast32_t size = ( screenSize.Width / sideDisplaySizeDenominator );
		
		std::wstring timeDummy = L"";
		
		{
			time_t currentTime = time( nullptr );
			size_t maxSize = std::max( settingsManager.timeFormat.length() * 2, ( size_t ) UINT_FAST8_MAX );
			wchar_t clockTime[ maxSize ];
			auto numCharsConverted = wcsftime( clockTime, maxSize, settingsManager.timeFormat.c_str(), localtime( &currentTime ) );
			
			if( numCharsConverted == 0 ) {
				numCharsConverted = wcsftime( clockTime, maxSize, settingsManager.timeFormatDefault.c_str(), localtime( &currentTime ) );
				
				if( numCharsConverted == 0 ) {
					numCharsConverted = wcsftime( clockTime, maxSize, L"%T", localtime( &currentTime ) );
				}
			}
			
			if( numCharsConverted != 0 ) {
				timeDummy = clockTime;
			} else {
				timeDummy = L"00:00:00";
			}
		}
		
		if( timeDummy.empty() ) {
			timeDummy = L"00:00:00";
		}
		
		do {
			clockFont = fontManager.GetTtFont( driver, fontFile, size, antiAliasFonts );
			if( not isNull( clockFont ) ) {
				fontDimensions = clockFont->getDimension( timeDummy.c_str() );
				size -= 2;
			}
		} while( not isNull( clockFont ) and ( fontDimensions.Width + viewportSize.Width > screenSize.Width  or fontDimensions.Height > ( screenSize.Height / 5 ) ) );
		
		size += 3;
		
		do {
			clockFont = fontManager.GetTtFont( driver, fontFile, size, antiAliasFonts );
			if( not isNull( clockFont ) ) {
				fontDimensions = clockFont->getDimension( timeDummy.c_str() );
				size -= 1;
			}
		} while( not isNull( clockFont ) and ( fontDimensions.Width + viewportSize.Width > screenSize.Width  or fontDimensions.Height > ( screenSize.Height / 5 ) ) );
	}
	
	if( fontFile == "" or isNull( clockFont ) or clockFont->getDimension( heightTestString.c_str() ).Height <= gui->getBuiltInFont()->getDimension( heightTestString.c_str() ).Height ) {
		clockFont = gui->getBuiltInFont();
	}
	
	if( settingsManager.debug ) {
		//std::wcout << L"clockFont is loaded" << std::endl;
	}
}

/**
 * @brief Loads exit confirmation questions from exitConfirmations file
 */
 void MainGame::loadExitConfirmations() {
	try {
		 if( settingsManager.debug ) {
			std::wcout << L"loadExitConfirmations() called" << std::endl;
		}
		
		exitConfirmations.clear(); //This line is unnecessary because loadExitConfirmations() is only called once, but I just feel safer clearing this anyway.
		
		auto folderList = system.getDataFolders();
		
		for( auto folderListIterator = folderList.begin(); folderListIterator != folderList.end(); ++folderListIterator ) {
			auto exitConfirmationsPath = *folderListIterator;
		
			exitConfirmationsPath = exitConfirmationsPath/L"exitConfirmations.txt";
			
			if( exists( exitConfirmationsPath ) ) {
				if( not is_directory( exitConfirmationsPath ) ) {
					if( settingsManager.debug ) {
						std::wcout << L"Loading exit confirmations from file " << exitConfirmationsPath.wstring() << std::endl;
					}
					
					boost::filesystem::wfstream exitConfirmationsFile;
					exitConfirmationsFile.open( exitConfirmationsPath, boost::filesystem::wfstream::in );
					exitConfirmationsFile.imbue( std::locale() ); //Can't tell if this line is necessary. main.cpp sets the global C++ locale to use codecvt_utf8, and we want to read and write UTF-8 files so we imbue the file stream with the global locale.
					
					if( exitConfirmationsFile.is_open() ) {
						uintmax_t lineNum = 0; //This used to be a uint_fast8_t, which should be good enough. However, when dealing with user input (such as a file), we don't want to make assumptions.
					
						while( exitConfirmationsFile.good() ) {
							++lineNum;
							std::wstring line;
							getline( exitConfirmationsFile, line );
							
							//if( !isNull( linePointer ) ) {
								//std::wstring line = linePointer;
								
								if( not line.empty() ) {
									line = line.substr( 0, line.find( L"//" ) ); //Filters out comments
									boost::algorithm::trim_all( line ); //Removes trailing and leading spaces, and spaces in the middle are reduced to one character
									
									if( not line.empty() ) {
										exitConfirmations.push_back( stringConverter.toIrrlichtStringW( line ) ); //StringConverter converts between wstring (which is what getLine needs) and core::stringw (which is what Irrlicht needs)
										
										if( settingsManager.debug ) {
											std::wcout << line << std::endl;
										}
									}
								}
							//}
						}// while( !isNull( linePointer ) );
						
						exitConfirmationsFile.close();
						//fclose( exitConfirmationsFile );
						
						//The random number generator has already been seeded
						shuffle( exitConfirmations.begin(), exitConfirmations.end(), randomNumberGenerator );
					} else {
						//throw( CustomException( std::wstring( L"Unable to open exit confirmations file even though it exists. Check its access permissions." ) ) );
					}
				} else {
					//throw( CustomException( std::wstring( L"Exit confirmations file is a directory. Cannot load exit confirmations." ) ) );
				}
			} else {
				//throw( CustomException( std::wstring( L"Exit confirmations file does not exist. Cannot load exit confirmations." ) ) );
			}
		}
		
	} catch( CustomException &e ) {
		std::wcerr << L"Error in MainGame::loadExitConfirmations(): " << e.what() << std::endl;
		return;
	} catch( std::exception &e ) {
		std::wcerr << L"Error in MainGame::loadExitConfirmations(): " << e.what() << std::endl;
		return;
	}
	
	if( settingsManager.debug ) {
		std::wcout << L"end of loadExitConfirmations()" << std::endl;
	}
 }

/**
 * Loads fonts. Calls loadMusicFont(), loadTipFont(), and loadClockFont().
 */
void MainGame::loadFonts() {
	try {
		if( settingsManager.debug ) {
			std::wcout << L"loadFonts() called" << std::endl;
		}
		//TODO: When a font is loaded, use its size ratio to estimate what the next size to try should be
		fontFile = "";
		boost::filesystem::recursive_directory_iterator end;
		std::vector< boost::filesystem::path > fontFolders = system.getFontFolders(); // Flawfinder: ignore

		if( settingsManager.debug ) {
			std::wcout << L"fontFolders.size(): " << fontFolders.size() << std::endl;
		}

		//for( std::vector< boost::filesystem::path >::iterator o = fontFolders.begin(); o not_eq fontFolders.end(); o++ ) {
		for( decltype( fontFolders.size() ) o = 0; o < fontFolders.size(); ++o ) {
			//for( boost::filesystem::recursive_directory_iterator i( *o ); i not_eq end; ++i ) {
			if( settingsManager.debug ) {
				std::wcout << L"Looking for fonts in folder " << fontFolders.at( o ) << std::endl;
			}
			
			if( exists( fontFolders.at( o ) ) ) {
				for( boost::filesystem::recursive_directory_iterator i( fontFolders.at( o ) ); i not_eq end; ++i ) {
					if( fontManager.canLoadFont( i->path() ) ) {
						if( settingsManager.debug ) {
							std::wcout << L"SUCCESS: " << i->path() << L" is a loadable font." << std::endl;
						}
						fontFile = stringConverter.toIrrlichtStringW( i->path().wstring() );
						break;
					}
				}
			}
			if( fontFile not_eq "" ) { //We've found a loadable file
				break;
			}
		}
		
		heightTestString = L"()*^&v.ygj|\U0001F5FB\u222B"; //Just a bunch of characters that usually tend to have high tops or low bottoms
		//Lol: "The tallest Unicode character in the current standard (Unicode 6.1) is [Mount Fuji emoji], U+1F5FB MOUNT FUJI, which is 3776 meters tall." https://stackoverflow.com/questions/9208489/tallest-unicode-character#9746990
		
		//These were split off into separate functions because they are needed more often than loadFonts()
		loadTipFont();
		
		if( settingsManager.getPlayMusic() ) {
			loadMusicFont();
		}
		
		loadClockFont();

		{ //Load loadingFont
			irr::core::dimension2d< uint_fast32_t > fontDimensions;
			uint_fast32_t size = screenSize.Width / 30; //30 found through experimentation: much larger and it takes too long to load fonts, much smaller and the font doesn't get as big as it should. Feel free to change at will if your computer's faster than mine.

			if( fontFile not_eq "" ) {
				do { //TODO: Repeatedly loading fonts like this seems like a waste of time. Is there a way we could load the font only once and still get this kind of size adjustment?
					loadingFont = fontManager.GetTtFont( driver, fontFile, size, antiAliasFonts );
					if( not isNull( loadingFont ) ) {
						fontDimensions = loadingFont->getDimension( stringConverter.toStdWString( loading ).c_str() ); //stringConverter.toWCharArray( loading ) );
						size -= 2;
					}
				} while( not isNull( loadingFont ) and ( fontDimensions.Width > ( screenSize.Width / sideDisplaySizeDenominator ) or fontDimensions.Height > ( screenSize.Height / 5 ) ) );

				size += 3;

				do {
					loadingFont = fontManager.GetTtFont( driver, fontFile, size, antiAliasFonts );
					if( not isNull( loadingFont ) ) {
						fontDimensions = loadingFont->getDimension( stringConverter.toStdWString( loading ).c_str() ); //stringConverter.toWCharArray( loading ) );
						size -= 1;
					}
				} while( not isNull( loadingFont ) and ( fontDimensions.Width > ( screenSize.Width / sideDisplaySizeDenominator ) or fontDimensions.Height > ( screenSize.Height / 5 ) ) );
			}

			if( fontFile == "" or isNull( loadingFont ) or size <= gui->getBuiltInFont()->getDimension( heightTestString.c_str() ).Height ) {
				loadingFont = gui->getBuiltInFont();
			}

			if( settingsManager.debug ) {
				std::wcout << L"loadingFont is loaded" << std::endl;
			}
		}


		{ //load textFont
			irr::core::dimension2d< uint_fast32_t > fontDimensions;
			if( fontFile not_eq "" ) {
				uint_fast32_t size = ( screenSize.Width / sideDisplaySizeDenominator ) / 6; //found through experimentation, adjust it however you like and see how many times the font gets loaded

				do {
					textFont = fontManager.GetTtFont( driver, fontFile, size, antiAliasFonts );

					if( not isNull( textFont ) ) {
						if( settingsManager.debug ) {
							std::wcout << L"Loaded textFont in loop (size " << size << L")" << std::endl;
						}
						fontDimensions = textFont->getDimension( L"Random seed: " );
						if( settingsManager.debug ) {
							std::wcout << L"fontDimensions set to " << fontDimensions.Width << L"x" << fontDimensions.Height << std::endl;
						}
						size -= 2;
					}
				} while( not isNull( textFont ) and ( fontDimensions.Width + viewportSize.Width > screenSize.Width ) );

				size += 3;

				do {
					textFont = fontManager.GetTtFont( driver, fontFile, size, antiAliasFonts );

					if( not isNull( textFont ) ) {
						if( settingsManager.debug ) {
							std::wcout << L"Loaded textFont in loop (size " << size << L")" << std::endl;
						}
						fontDimensions = textFont->getDimension( L"Random seed: " );
						if( settingsManager.debug ) {
							std::wcout << L"fontDimensions set to " << fontDimensions.Width << L"x" << fontDimensions.Height << std::endl;
						}
						size -= 1;
					}
				} while( not isNull( textFont ) and ( fontDimensions.Width + viewportSize.Width > screenSize.Width ) );
			}

			if( fontFile == "" or isNull( textFont ) or textFont->getDimension( heightTestString.c_str() ).Height <= gui->getBuiltInFont()->getDimension( heightTestString.c_str() ).Height ) {
				textFont = gui->getBuiltInFont();
			}

			if( settingsManager.debug ) {
				std::wcout << L"textFont is loaded" << std::endl;
			}
		}

		
		
		{ //Load statsFont
			irr::core::dimension2d< uint_fast32_t > fontDimensions;
			
			if( fontFile not_eq "" ) {
				
				decltype( loadingFont->getDimension( L"" ).Height ) aboveStats;
				
				{
					decltype( tipFont->getDimension( L"" ).Height ) proTipHeight;
					if( proTips.empty() ) {
						proTipHeight = 0;
					} else {
						
						if( currentProTip >= proTips.size() ) {
							//This code should be unreachable - currentProTip is always less than proTips.size() - I'm just adding this because I'm paranoid:
							currentProTip = 0;
						}
						
						proTipHeight = tipFont->getDimension( proTips.at( currentProTip ).c_str() ).Height;
					}
					
					aboveStats = loadingFont->getDimension( loading.c_str() ).Height * 2 + std::max( tipFont->getDimension( proTipPrefix.c_str() ).Height, proTipHeight );
				}
				
				uint_fast32_t size = screenSize.Width / settingsManager.getNumPlayers() / 3; //A quick approximation of the size we'll need the text to be. This is not exact because size is actually an indicator of font height, but numPlayers and hence the needed width are more likely to vary.
				uint_fast8_t builtInFontHeight = gui->getBuiltInFont()->getDimension( heightTestString.c_str() ).Height;
				if( size > builtInFontHeight ) { //If the text needs to be that small, go with the built-in font because it's readable at that size.
					do {
						size -= 2;
						statsFont = fontManager.GetTtFont( driver, fontFile, size, antiAliasFonts );
						if( not isNull( statsFont ) ) {
							irr::core::dimension2d< uint_fast32_t > tempDimensions;
							std::wstring tempString;
							if( settingsManager.getNumPlayers() <= 10 ) {
								tempString = L"0.P";
							} else if( settingsManager.getNumPlayers() <= 100 ) {
								tempString = L"00.P";
							} else {
								tempString = L"000.P";
							}
							tempString += stringConverter.toStdWString( settingsManager.getNumPlayers() );
							
							tempDimensions = statsFont->getDimension( tempString.c_str() );
							fontDimensions = irr::core::dimension2d< uint_fast32_t >( tempDimensions.Width * settingsManager.getNumPlayers(), tempDimensions.Height );
							tempDimensions = statsFont->getDimension( keysFoundPerPlayer.c_str() ); //stringConverter.toWCharArray( winnersLabel ) );
							fontDimensions = irr::core::dimension2d< uint_fast32_t >( fontDimensions.Width + tempDimensions.Width, std::max( fontDimensions.Height, tempDimensions.Height ) * 6 ); //6 = the number of rows of stats displayed on the loading screen
						}
					} while( size > builtInFontHeight and not isNull( statsFont ) and ( fontDimensions.Width >= screenSize.Width  or fontDimensions.Height + aboveStats >= screenSize.Height ) );

					size += 3;

					do {
						size -= 1;
						statsFont = fontManager.GetTtFont( driver, fontFile, size, antiAliasFonts );
						if( not isNull( statsFont ) ) {
							irr::core::dimension2d< uint_fast32_t > tempDimensions;
							std::wstring tempString;
							if( settingsManager.getNumPlayers() <= 10 ) {
								tempString = L"0.P";
							} else if( settingsManager.getNumPlayers() <= 100 ) {
								tempString = L"00.P";
							} else {
								tempString = L"000.P";
							}
							tempString += stringConverter.toStdWString( settingsManager.getNumPlayers() );
							
							tempDimensions = statsFont->getDimension( tempString.c_str() );
							fontDimensions = irr::core::dimension2d< uint_fast32_t >( tempDimensions.Width * settingsManager.getNumPlayers(), tempDimensions.Height );
							tempDimensions = statsFont->getDimension( keysFoundPerPlayer.c_str() ); //stringConverter.toWCharArray( winnersLabel ) );
							fontDimensions = irr::core::dimension2d< uint_fast32_t >( fontDimensions.Width + tempDimensions.Width, std::max( fontDimensions.Height, tempDimensions.Height ) * 6 ); //6 = the number of rows of stats displayed on the loading screen
						}
					} while( size > builtInFontHeight and not isNull( statsFont ) and ( fontDimensions.Width >= screenSize.Width  or fontDimensions.Height + aboveStats >= screenSize.Height ) );
				}
			}
			
			
			if( fontFile == "" or isNull( statsFont ) or statsFont->getDimension( heightTestString.c_str() ).Height <= gui->getBuiltInFont()->getDimension( heightTestString.c_str() ).Height ) {
				statsFont = gui->getBuiltInFont();
			}

			if( settingsManager.debug ) {
				std::wcout << L"statsFont is loaded" << std::endl;
			}
		}
		
		if( not isScreenSaver ) {
			menuManager.setFontAndResizeIcons( device, clockFont ); //Why use clockFont? Because I'm too lazy to implement loading another font.
			
			settingsScreen.setButtonFont( clockFont );
			settingsScreen.setTextFont( textFont );
			settingsScreen.setupIconsAndStuff();
			
			uint_fast32_t size = 12; //The GUI adjusts window sizes based on the text within them, so no need (hopefully) to use different font sizes for different window sizes. May affect readability on large or small screens, but it's better on large screens than the built-in font.
			gui->getSkin()->setFont( fontManager.GetTtFont( driver, fontFile, size, antiAliasFonts ) );
		}
		
		if( settingsManager.debug ) {
			std::wcout << L"end of loadFonts()" << std::endl;
		}
	} catch( std::exception &e ) {
		std::wcerr << L"Error in MainGame::loadFonts(): " << e.what() << std::endl;
	}
}

/**
 * Loads the music font. Like loadTipFont() below, this guesses a good font size, then repeatedly adjusts the size and reloads the font until everything fits.
 */
void MainGame::loadMusicFont() {
	try {
		if( settingsManager.debug ) {
			std::wcout << L"loadMusicFont() called" << std::endl;
		}
		
		if( settingsManager.getPlayMusic() ) {
			if( fontFile not_eq "" ) {
				uint_fast32_t size = 0; //The height (I think) of the font to be loaded
			
				uint_fast32_t maxWidth = ( screenSize.Width / sideDisplaySizeDenominator );
				
				uint_fast32_t numerator = 2.5 * maxWidth; //2.5 is an arbitrarily chosen number, it has no special meaning. Change it to anything you want.

				//I felt it looked best if all three (artist, album, and title) had the same font size, so we're picking the longest of the three and basing the font size on its length.
				if( musicArtist.size() >= musicAlbum.size() and musicArtist.size() > 0 ) {
					if( musicArtist.size() >= musicTitle.size() ) {
						size = numerator / musicArtist.size();
					} else if( musicTitle.size() > 0 ) {
						size = numerator / musicTitle.size();
					}
				} else {
					if( musicAlbum.size() >= musicTitle.size() and musicAlbum.size() > 0 ) {
						size = numerator / musicAlbum.size();
					} else if( musicTitle.size() > 0 ) {
						size = numerator / musicTitle.size();
					}
				}

				if( size == 0 ) { //If none of album, artist, and title had lengths greater than zero: maybe they haven't been read yet, or maybe the file doesn't contain those tags.
					size = numerator;
				}

				irr::core::dimension2d< uint_fast32_t > artistDimensions;
				irr::core::dimension2d< uint_fast32_t > albumDimensions;
				irr::core::dimension2d< uint_fast32_t > titleDimensions;

				do {
					musicTagFont = fontManager.GetTtFont( driver, fontFile, size, antiAliasFonts );
					if( not isNull( musicTagFont ) ) {
						artistDimensions = musicTagFont->getDimension( stringConverter.toStdWString( musicArtist ).c_str() ); //stringConverter.toWCharArray( musicArtist ) );
						albumDimensions = musicTagFont->getDimension( stringConverter.toStdWString( musicAlbum ).c_str() ); //stringConverter.toWCharArray( musicAlbum ) );
						titleDimensions = musicTagFont->getDimension( stringConverter.toStdWString( musicTitle ).c_str() ); //stringConverter.toWCharArray( musicTitle ) );
						size -= 2; //Initially I had it going down by 1 each time. It's faster to go down by 2 until we've gotten close to our desired size...
					}
				} while( not isNull( musicTagFont ) and ( artistDimensions.Width > maxWidth or albumDimensions.Width > maxWidth or titleDimensions.Width > maxWidth ) );

				size += 3; //...then we up the size a little...

				do {
					musicTagFont = fontManager.GetTtFont( driver, fontFile, size, antiAliasFonts );
					if( not isNull( musicTagFont ) ) {
						artistDimensions = musicTagFont->getDimension( stringConverter.toStdWString( musicArtist ).c_str() ); //stringConverter.toWCharArray( musicArtist ) );
						albumDimensions = musicTagFont->getDimension( stringConverter.toStdWString( musicAlbum ).c_str() ); //stringConverter.toWCharArray( musicAlbum ) );
						titleDimensions = musicTagFont->getDimension( stringConverter.toStdWString( musicTitle ).c_str() ); //stringConverter.toWCharArray( musicTitle ) );
						size -= 1; //...and start going down by 1.
					}
				} while( not isNull( musicTagFont ) and ( artistDimensions.Width > maxWidth or albumDimensions.Width > maxWidth or titleDimensions.Width > maxWidth ) );
			}

			if( fontFile == "" or isNull( musicTagFont ) or musicTagFont->getDimension( heightTestString.c_str() ).Height <= gui->getBuiltInFont()->getDimension( heightTestString.c_str() ).Height ) {
				if( !isNull( gui ) ) { //When this function is first called, gui hasn't been set yet.
					musicTagFont = gui->getBuiltInFont();
				}
			}
		}
		
		if( settingsManager.debug ) {
			std::wcout << L"end of loadMusicFont()" << std::endl;
		}
	} catch( std::exception &e ) {
		std::wcerr << L"Error in MainGame::loadMusicFont(): " << e.what() << std::endl;
	}
}

/**
 * Loads the next song on the list.
 */
void MainGame::loadNextSong() {
	try {
		if( settingsManager.debug ) {
			std::wcout << L"loadNextSong() called" << std::endl;
		}
		
		if( musicList.empty() ) {
			makeMusicList();
		}
		
		if( !musicList.empty() ) {
			//Figure out where we are in the music list
			std::vector<boost::filesystem::path>::size_type positionInList = 0;
			for( decltype( musicList.size() ) i = 0; i < musicList.size(); ++i ) {
				if( musicList.at( i ) == currentMusic ) {
					positionInList = i;
					break;
				}
			}

			//Move on to the next position
			positionInList += 1;
			if( positionInList >= musicList.size() ) {
				positionInList -= musicList.size();
			}

			currentMusic = musicList[ positionInList ];
			music = Mix_LoadMUS( currentMusic.c_str() );

			if( isNull( music ) ) {
				throw( CustomException( std::wstring( L"Unable to load music file: " ) + stringConverter.toStdWString( Mix_GetError() ) ) );
			} else {
				auto musicStatus = Mix_PlayMusic( music, 0 ); //The second argument tells how many times to *repeat* the music. -1 means infinite, 0 means don't repeat.

				if( musicStatus == -1 ) {
					throw( CustomException( std::wstring( L"Unable to play music file: " ) + stringConverter.toStdWString( Mix_GetError() ) ) );
				} else {
					if( settingsManager.debug ) {
						switch( Mix_GetMusicType( nullptr ) ) {
							case MUS_CMD:
								std::wcout << L"Command based";
								break;
							case MUS_WAV:
								std::wcout << L"WAVE";
								break;
							case MUS_MOD:
								std::wcout << L"MOD";
								break;
							case MUS_MID:
								std::wcout << L"MIDI";
								break;
							case MUS_OGG:
								std::wcout << L"OGG";
								break;
							case MUS_MP3:
							case MUS_MP3_MAD:
								std::wcout << L"MP3";
								break;
							case MUS_NONE:
								std::wcout << L"No";
								break;
							case MUS_FLAC:
								std::wcout << L"FLAC";
								break;
							default:
								std::wcout << L"An unknown type of";
								break;
						}

						std::wcout << " music is playing." << std::endl;
					}

					//Now playing
					TagLib::FileRef musicFile( currentMusic.c_str() ); //TagLib doesn't accept wstrings as file names, but it apparently can read tags as wstrings

					if( not musicFile.isNull() and not isNull( musicFile.tag() ) ) {
						musicTitle = stringConverter.toIrrlichtStringW( musicFile.tag()->title().toWString() ); //toWString() alone doesn't work here even though these are wide character strings because Irrlicht doesn't like accepting TagLib's wstrings.
						musicArtist = stringConverter.toIrrlichtStringW( musicFile.tag()->artist().toWString() );
						musicAlbum = stringConverter.toIrrlichtStringW( musicFile.tag()->album().toWString() );

						if( musicTitle.size() == 0 ) {
							musicTitle = L"Unknown Title";
						}

						if( musicArtist.size() == 0 ) {
							musicArtist = L"Unknown Artist";
						}

						if( musicAlbum.size() == 0 ) {
							musicAlbum = L"Unknown Album";
						}

						if( settingsManager.debug ) {
							std::wcout << L"Now playing: " << stringConverter.toStdWString( musicTitle ) << L" by " << stringConverter.toStdWString( musicArtist ) << L" from album " << stringConverter.toStdWString( musicAlbum ) << std::endl;
						}
					}

					loadMusicFont();
				}

				musicVolumeChanged();//Mix_VolumeMusic( settingsManager.getMusicVolume() * MIX_MAX_VOLUME / 100 );
			}
		}
		if( settingsManager.debug ) {
			std::wcout << L"end of loadNextSong()" << std::endl;
		}
	} catch( CustomException e ) {
		std::wcerr << L"Error in MainGame::loadNextSong(): " << e.what() << std::endl;
	} catch( std::exception &e ) {
		std::wcerr << L"Error in MainGame::loadNextSong(): " << e.what() << std::endl;
	}
}

/**
 * Loads "pro tips" from file proTips.txt if that file exists.
 */
void MainGame::loadProTips() {
	try {
		if( settingsManager.debug ) {
			std::wcout << L"loadProTips() called" << std::endl;
		}
		
		proTips.clear(); //This line is unnecessary because loadProTips() is only called once, but I just feel safer clearing this anyway.
		
		auto folderList = system.getDataFolders();
		
		for( auto folderListIterator = folderList.begin(); folderListIterator != folderList.end(); ++folderListIterator ) {
			auto proTipsPath = *folderListIterator;
		
			proTipsPath = proTipsPath/L"protips.txt";
			
			if( exists( proTipsPath ) ) {
				if( not is_directory( proTipsPath ) ) {
					if( settingsManager.debug ) {
						std::wcout << L"Loading pro tips from file " << proTipsPath.wstring() << std::endl;
					}
					
					boost::filesystem::wfstream proTipsFile;
					proTipsFile.open( proTipsPath, boost::filesystem::wfstream::in );
					proTipsFile.imbue( std::locale() ); //Can't tell if this line is necessary. main.cpp sets the global C++ locale to use codecvt_utf8, and we want to read and write UTF-8 files so we imbue the file stream with the global locale.
					
					if( proTipsFile.is_open() ) {
						uintmax_t lineNum = 0; //This used to be a uint_fast8_t, which should be good enough. However, when dealing with user input (such as a file), we don't want to make assumptions.
					
						while( proTipsFile.good() ) {
							++lineNum;
							std::wstring line;
							getline( proTipsFile, line );
								
							if( not line.empty() ) {
								line = line.substr( 0, line.find( L"//" ) ); //Filters out comments
								boost::algorithm::trim_all( line ); //Removes trailing and leading spaces, and spaces in the middle are reduced to one character
								
								if( not line.empty() ) {
									proTips.push_back( stringConverter.toIrrlichtStringW( line ) ); //StringConverter converts between wstring (which is what getLine needs) and core::stringw (which is what Irrlicht needs)
									
									if( settingsManager.debug ) {
										std::wcout << line << std::endl;
									}
								}
							}
						} // while( !isNull( linePointer ) );
						
						proTipsFile.close();
						//fclose( proTipsFile );
						
						//setRandomSeed( time( nullptr ) ); //Initializing the random number generator here allows shuffle() to use it. A new random seed will be chosen, or loaded from a file, before the first maze gets generatred.
						shuffle( proTips.begin(), proTips.end(), randomNumberGenerator );
					} else {
						//throw( CustomException( std::wstring( L"Unable to open pro tips file even though it exists. Check its access permissions." ) ) );
					}
				} else {
					//throw( CustomException( std::wstring( L"Pro tips file is a directory. Cannot load pro tips." ) ) );
				}
			} else {
				//throw( CustomException( std::wstring( L"Pro tips file does not exist. Cannot load pro tips." ) ) );
			}
		}
		
	} catch( CustomException &e ) {
		std::wcerr << L"Error in MainGame::loadProTips(): " << e.what() << std::endl;
		return;
	} catch( std::exception &e ) {
		std::wcerr << L"Error in MainGame::loadProTips(): " << e.what() << std::endl;
		return;
	}
	
	if( settingsManager.debug ) {
		std::wcout << L"end of loadProTips()" << std::endl;
	}
}


/**
 * @brief Loads the random seed (used to generate mazes) from a file
 * @param src: the file to load
 * @return whether the file was read successfully
 */
bool MainGame::loadSeedFromFile( boost::filesystem::path src ) {
	try {
		if( not src.empty() ) {
			if( getDebugStatus() ) {
				std::wcout << L"Trying to load from file " << src.wstring() << std::endl;
			}
			
			if( not exists( src ) ) {
				throw( CustomException( std::wstring( L"File not found: " ) + src.wstring() ) );
			} else if( is_directory( src ) ) {
				throw( CustomException( std::wstring( L"Directory specified, file needed: " ) + src.wstring() ) );
			}
			
			boost::filesystem::wifstream file; //Identical to a standard C++ fstream, except it takes Boost paths
			file.open( src, boost::filesystem::wifstream::binary );

			if( file.is_open() ) {
				decltype( randomSeed ) newRandomSeed;
				file >> newRandomSeed;
				file.close();
				setRandomSeed( newRandomSeed );
				return true;
			} else {
				throw( CustomException( std::wstring( L"Cannot open file: \"" ) + src.wstring() + L"\"" ) );
			}
		}
	} catch( const boost::filesystem::filesystem_error &e ) {
		std::wcerr << L"Boost Filesystem error in MainGame::loadSeedFromFile(): " << e.what() << std::endl;
	} catch( CustomException &e ) {
		std::wcerr << L"non-Boost-Filesystem error in MainGame::loadSeedFromFile(): " << e.what() << std::endl;
	} catch( std::exception &e ) {
		std::wcerr << L"non-Boost-Filesystem error in MainGame::loadSeedFromFile(): " << e.what() << std::endl;
	}
	return false;
}

void MainGame::loadTextures() {
	//auto textureSearchLocation = boost::filesystem::path( boost::filesystem::current_path()/L"Images/players" );
	
	//std::vector< boost::filesystem::path > loadableTextures = getLoadableTexturesList( textureSearchLocation );
	
	std::vector< boost::filesystem::path > loadableTextures;
	auto textureSearchLocations = system.getImageFolders();
	for( auto locationIterator = textureSearchLocations.begin(); locationIterator != textureSearchLocations.end(); ++locationIterator ) {
		auto textureList = getLoadableTexturesList( *locationIterator / L"players" );
		for( auto textureIterator = textureList.begin(); textureIterator != textureList.end(); ++textureIterator ) {
			loadableTextures.push_back( *textureIterator );
		}
	}
	
	
	for( decltype( settingsManager.getNumPlayers() ) p = 0; p < settingsManager.getNumPlayers(); ++p ) {
		
		player.at( p ).setPlayerNumber( p );
		
		setObjectColorBasedOnNum( &( player.at( p ) ), p );
		
		player.at( p ).setMG( this );
		
		
		player.at( p ).loadTexture( device, 100, loadableTextures );
		
		playerAssigned.at( p ) = false;
		
		
		
		switch( settingsManager.colorMode ) {
			case SettingsManager::GRAYSCALE: {
				playerStart.at( p ).setColors( GRAY_GRAYSCALE, LIGHTRED_GRAYSCALE );
				break;
			}
			case SettingsManager::COLOR_MODE_DO_NOT_USE:
			case SettingsManager::FULLCOLOR: {
				playerStart.at( p ).setColors( GRAY, LIGHTRED );
				break;
			}
			case SettingsManager::GREENSCALE: {
				playerStart.at( p ).setColors( GRAY_GREENSCALE, LIGHTRED_GREENSCALE );
				break;
			}
			case SettingsManager::AMBERSCALE: {
				playerStart.at( p ).setColors( GRAY_AMBERSCALE, LIGHTRED_AMBERSCALE );
				break;
			}
		}
	}
	
	switch( settingsManager.colorMode ) {
		case SettingsManager::GRAYSCALE: {
			goal.setColors( GRAY_GRAYSCALE, LIGHTGREEN_GRAYSCALE );
			break;
		}
		case SettingsManager::COLOR_MODE_DO_NOT_USE:
		case SettingsManager::FULLCOLOR: {
			goal.setColors( GRAY, LIGHTGREEN );
			break;
		}
		case SettingsManager::GREENSCALE: {
			goal.setColors( GRAY_GREENSCALE, LIGHTGREEN_GREENSCALE );
			break;
		}
		case SettingsManager::AMBERSCALE: {
			goal.setColors( GRAY_AMBERSCALE, LIGHTGREEN_AMBERSCALE );
			break;
		}
	}
	goal.loadTexture( device );
}

/**
 * Loads the tip font. Guesses a size that will work, keeps adjusting the size and reloading the font until everything fits.
 */
void MainGame::loadTipFont() {
	try {
		if( settingsManager.debug ) {
			std::wcout << L"loadTipFont() called" << std::endl;
		}
		
		if( fontFile not_eq "" ) {
			uint_fast32_t size; //The height (I think) of the font to be loaded
			uint_fast32_t maxWidth = screenSize.Width;
			
			irr::core::stringw tipIncludingPrefix = proTipPrefix;

			if( proTips.size() > 0 ) { //If pro tips have been loaded, guess size based on tip length.
				tipIncludingPrefix.append( proTips.at( currentProTip ) );
				size = 2.5 * maxWidth / tipIncludingPrefix.size(); //2.5 is an arbitrarily chosen number, it has no special meaning. Change it to anything you want.
			} else {
				size = maxWidth / 10; //10 is also arbitrarily chosen.
			}

			irr::core::dimension2d< uint_fast32_t > tipDimensions;

			do {
				tipFont = fontManager.GetTtFont( driver, fontFile, size, antiAliasFonts );
				if( not isNull( tipFont ) ) {
					tipDimensions = tipFont->getDimension( stringConverter.toStdWString( tipIncludingPrefix ).c_str() ); //stringConverter.toWCharArray( tipIncludingPrefix ) );
					size -= 2;
				}
			} while( not isNull( tipFont ) and ( tipDimensions.Width > maxWidth ) );

			size += 3;

			do {
				tipFont = fontManager.GetTtFont( driver, fontFile, size, antiAliasFonts );
				if( not isNull( tipFont ) ) {
					tipDimensions = tipFont->getDimension( stringConverter.toStdWString( tipIncludingPrefix ).c_str() ); //stringConverter.toWCharArray( tipIncludingPrefix ) );
					size -= 1;
				}
			} while( not isNull( tipFont ) and ( tipDimensions.Width > maxWidth ) );
		}

		if( fontFile == "" or isNull( tipFont ) or tipFont->getDimension( heightTestString.c_str() ).Height <= gui->getBuiltInFont()->getDimension( heightTestString.c_str() ).Height ) {
			tipFont = gui->getBuiltInFont();
		}
		
		if( settingsManager.debug ) {
			std::wcout << L"end of loadTipFont()" << std::endl;
		}
	} catch( std::exception &e ) {
		std::wcerr << L"Error in MainGame::loadTipFont(): " << e.what() << std::endl;
	}
}


/**
 * This object's destructor. Destroys stuff.
 * I... am... DESTRUCTOOOOOOORRRRR!!!!!!!!!
 */
MainGame::~MainGame() {
	try {
		if( settingsManager.debug ) {
			std::wcout << L"MainGame destructor called" << std::endl;
		}
		
		if( not isNull( loadMazeDialog ) ) {
			delete loadMazeDialog;
		}
		if( not isNull( saveMazeDialog ) ) {
			delete saveMazeDialog;
		}
		
		stuff.clear(); //Calling this before removeAllTextures() because object destructors will remove their own textures
		player.clear();
		playerStart.clear();
		
		driver->removeAllHardwareBuffers();
		driver->removeAllTextures();

		device->closeDevice();
		device->run(); //Sometimes there are problems if we don't call run() after closeDevice(). Don't remember what they are at the moment.
		device->drop();

		if( settingsManager.getPlayMusic() ) {
			Mix_HaltMusic();
			Mix_FreeMusic( music );
			Mix_CloseAudio();

			while( Mix_Init( 0 ) ) {
				Mix_Quit();
			}

			SDL_Quit();
		}
		
		if( settingsManager.debug ) {
			std::wcout << L"end of MainGame destructor" << std::endl;
		}
	} catch( std::exception &e ) {
		std::wcerr << L"Error in MainGame::~MainGame(): " << e.what() << std::endl;
	}
}

/**
 * This object's constructor. Does lots of very important stuff.
 * Since this constructor is so big, maybe some parts should be split off into separate functions for readability.
 */
MainGame::MainGame( std::wstring fileToLoad = L"", bool runAsScreenSaver = false ) {
	try {
		initializeVariables( runAsScreenSaver );
		
		device = irr::createDevice( irr::video::EDT_NULL ); //Must create a device before calling readPrefs();
		
		if( isNull( device ) ) {
			throw( CustomException( std::wstring( L"Cannot create null device. Something is definitely wrong here!" ) ) );
		}
		
		settingsManager.setPointers( device, this, &mazeManager, &network, &spellChecker, &system);
		
		settingsManager.readPrefs();
		
		if( not isScreenSaver ) {
			//Set up networking
			network.setPort( settingsManager.networkPort );
			network.setup( this, settingsManager.isServer );
		}
		
		//Initializing the random number generator here allows makeMusicList() (called by musicSettingChanged(), which in turn is called below and by SettingsManager), loadProTips(), and pickLogo() to use it. A new random seed will be chosen, or loaded from a file, before the first maze gets generated.
		if( not loadSeedFromFile( fileToLoad ) ) {
			if( settingsManager.isServer ) {
				setRandomSeed( time( nullptr ) );
			} else {
				network.processPackets();
			}
			firstMaze = false;
		} else {
			firstMaze = true;
		}
		
		if( settingsManager.getPlayMusic() ) {
			musicSettingChanged(); //Should be after the call to setRandomSeed() 
		}
		
		setMyPlayer( UINT8_MAX ); //Must call this before setControls() so that controls which affect player number "mine" will work. setMyPlayer() will be called again later to set the correct player number; the number used here doesn't matter.
		setupDevice(); //Must call this before setControls() because setControls() calls device->activateJoysticks()
		
		if( not isScreenSaver ) {
			setControls();
		} else {
			settingsManager.setPlayMusic( false );
			
			if( settingsManager.getNumBots() == 0 ) {
				settingsManager.setNumBots( 1 );
			}
			
			settingsManager.setNumPlayers( settingsManager.getNumBots() );
		}
		
		if( runAsScreenSaver ) {
			//The screen saver preview window might be really tiny
			screenSize = device->getVideoDriver()->getScreenSize();
			settingsManager.setWindowSize( screenSize );
		}
		
		viewportSize.set( screenSize.Width - ( screenSize.Width / sideDisplaySizeDenominator ), screenSize.Height - 1 );
		
		setupDriver();
		
		pickLogo();
		driver->beginScene( false, false ); //These falses specify whether the back buffer and z buffer should be cleared. Since this is the first time drawing anything, there's no need to clear anything beforehand.
		drawLogo(); //Why the fuck isn't this working consistently? Sometimes it draws, sometimes it only thinks it draws. Had to hack the drawLoadingScreen() function (which gets called several times, therefore is likely to work at least once).
		driver->endScene();

		gui = device->getGUIEnvironment();
		if( isNull( gui ) ) {
			throw( CustomException( std::wstring( L"Cannot get GUI environment" ) ) );
		} else {
			if ( settingsManager.debug ) {
				std::wcout << L"Got the gui environment" << std::endl;
			}
			for( uint_fast8_t i = 0; i < ( decltype( i ) )irr::gui::EGDC_COUNT ; ++i ) {
				irr::video::SColor guiSkinColor = gui->getSkin()->getColor( static_cast< irr::gui::EGUI_DEFAULT_COLOR >( i ) );
				guiSkinColor.setAlpha( 255 );
				gui->getSkin()->setColor( static_cast< irr::gui::EGUI_DEFAULT_COLOR >( i ), guiSkinColor );
			}
		}
		
		loadProTips();
		
		device->setWindowCaption( stringConverter.toStdWString( PACKAGE_STRING ).c_str() ); //stringConverter.toWCharArray( PACKAGE_STRING ) );
		
		if( settingsManager.debug ) {
			device->getLogger()->setLogLevel( irr::ELL_INFORMATION );
		} else {
			device->getLogger()->setLogLevel( irr::ELL_ERROR );
		}
		
		backgroundSceneManager = device->getSceneManager();
		if( isNull( backgroundSceneManager ) ) {
			throw( CustomException( std::wstring( L"Cannot get scene manager" ) ) );
		} else if ( settingsManager.debug ) {
			std::wcout << L"Got the scene manager" << std::endl;
		}
		
		if( not isScreenSaver ) {
			settingsScreen.setPointers( this, device, nullptr, nullptr, &settingsManager );
			settingsScreen.setupIconsAndStuff(); //Icon size might depend on screen/window size; that's why we call this after readPrefs()
		}
		
		loadFonts();
		
		if( not isScreenSaver ) {
			menuManager.setMainGame( this );
			menuManager.setPositions( screenSize.Height );
			menuManager.loadIcons( device );
		}

		if ( settingsManager.debug ) {
			std::wcout << L"Resizing player and playerStart vectors to " << settingsManager.getNumPlayers() << std::endl;
		}

		player.resize( settingsManager.getNumPlayers() );
		playerStart.resize( settingsManager.getNumPlayers() );
		playerAssigned.resize( settingsManager.getNumPlayers() );
		
		loadTextures();
		
		if( settingsManager.isServer ) {
			setMyPlayer( 0 );
		} else {
			settingsManager.setNumBots( 0 ); //Only the server can control the bots. Clients should see them as other players.
		}
		
		if( settingsManager.getNumBots() > settingsManager.getNumPlayers() ) {
			settingsManager.setNumBots( settingsManager.getNumPlayers() );
		}
		
		if( settingsManager.getNumBots() > 0 ) {
			if ( settingsManager.debug ) {
				std::wcout << L"Resizing bot vector to " << settingsManager.getNumBots() << std::endl;
			}
			
			setNumBots( settingsManager.getNumBots() );
			
			for( decltype( settingsManager.getNumBots() ) i = 0; i < settingsManager.getNumBots(); ++i ) {
				decltype( bot.at( i ).getPlayer() ) p = settingsManager.getNumPlayers() - ( i + 1 );
				bot.at( i ).setPlayer( p ) ;
				playerAssigned.at( p ) = true;
				player.at( bot.at( i ).getPlayer() ).isHuman = false;
				//bot.at( i ).setup( mazeManager.maze, mazeManager.cols, mazeManager.rows, this, botsKnowSolution, botAlgorithm, botMovementDelay );
				bot.at( i ).setup( this, settingsManager.botsKnowSolution, settingsManager.getBotAlgorithm(), settingsManager.botMovementDelay );
			}
		}
		
		timer = device->getTimer();
		
		loadExitConfirmations();
		
		if( settingsManager.debug ) {
			std::wcout << L"end of MainGame constructor" << std::endl;
		}
	} catch( std::exception &e ) {
		std::wcerr << L"Error in MainGame::MainGame(): " << e.what() << std::endl;
	}
}


/**
 * Finds all playable music files in the ./Music folder and compiles them into a list. If ./Music does not exist or is not a folder, it uses the parent path instead.
 */
void MainGame::makeMusicList() {
	try {
		if( settingsManager.debug ) {
			std::wcout << L"makeMusicList() called" << std::endl;
		}
		
		musicList.clear(); //The music list should be empty anyway, since makeMusicList() only gets called once, but just in case...
		
		std::vector< std::wstring > allowedFileExtensions;
		
		//if( settingsManager.debug ) {
			decltype( Mix_GetNumMusicDecoders() ) numMusicDecoders = Mix_GetNumMusicDecoders();
			
			if( settingsManager.debug ) {
				std::wcout << L"There are " << numMusicDecoders << L" music decoders available. They are:" << std::endl;
			}
			
			for( decltype( numMusicDecoders ) decoder = 0; decoder < numMusicDecoders; ++decoder ) {
				std::wstring decoderName = stringConverter.toStdWString( Mix_GetMusicDecoder( decoder ) );
				
				if( settingsManager.debug ) {
					std::wcout << decoder << L": \"" << decoderName << L"\"" << std::endl;
				}
				
				{
					//These are all the decoders I know of as of 2017-01-02:
					static std::map< std::wstring, uint_fast8_t > names = {
						{ L"WAVE", 0 },
						{ L"MODPLUG", 1 },
						{ L"OGG", 2 },
						{ L"FLAC", 3 },
						{ L"MP3", 4 },
						{ L"TIMIDITY", 5 }
					};
					
					try {
						//These are all the file extensions I could think of for each decoder. Feel free to add more.
						switch( names.at( decoderName ) ) {
							case 0: {
								allowedFileExtensions.push_back( L".wav" );
								allowedFileExtensions.push_back( L".wave" );
								break;
							}
							case 1: {
								allowedFileExtensions.push_back( L".mod" );
								break;
							}
							case 2: {
								allowedFileExtensions.push_back( L".ogg" );
								allowedFileExtensions.push_back( L".oga" );
								allowedFileExtensions.push_back( L".ogm" );
								break;
							}
							case 3: {
								allowedFileExtensions.push_back( L".flac" );
								allowedFileExtensions.push_back( L".fla" );
								break;
							}
							case 4: {
								allowedFileExtensions.push_back( L".mp3" );
								break;
							}
							case 5: {
								allowedFileExtensions.push_back( L".mid" );
								break;
							}
						}
					} catch( const std::out_of_range& err ) {
						//This allows for decoders not yet known to me: the decoder name might be used as an extension
						boost::algorithm::to_lower( decoderName );
						allowedFileExtensions.push_back( L"." + decoderName );
						std::wcerr << L"Unknown music decoder: \"" << decoderName << L"\"" << std::endl;
					}
				}
			}
		//}
		
		if( settingsManager.debug ) {
			std::wcout << L"Allowed file extensions for music: ";
			for( auto it = allowedFileExtensions.begin(); it != allowedFileExtensions.end(); ++it ) {
				std::wcout << *it << L", ";
			}
			std::wcout << std::endl;
		}
		
		auto folderList = system.getMusicFolders();
		
		//If musicPath (defined inside the for loop inside the following while loop) has a Music subfolder, we want to find music there first.
		bool useMusicSubfolder = true;
		for( uint_fast8_t repeat = 0; repeat < 2; ++repeat ) {
			
			for( auto folderListIterator = folderList.begin(); folderListIterator != folderList.end(); ++folderListIterator ) {
				auto musicPath = *folderListIterator;
				
				if( useMusicSubfolder ) {
					musicPath = musicPath/L"Music";
				}
				
				//Which is better: system_complete() or absolute()? On my computer they seem to do the same thing. Both are part of Boost Filesystem.
				musicPath = system_complete( musicPath );
				//musicPath = absolute( musicPath );
				
				if( settingsManager.debug ) {
					std::wcout << L"music path is absolute? " << musicPath.is_absolute() << std::endl;
				}
				
				while( ( not exists( musicPath ) or not is_directory( musicPath ) ) and musicPath.has_parent_path() ) {
					if( settingsManager.debug ) {
						std::wcout << L"Path " << musicPath.wstring() << L" does not exist or is not a directory. Checking parent path " << musicPath.parent_path().wstring() << std::endl;
					}
					
					musicPath = musicPath.parent_path();
				}
				
				if( exists( musicPath ) ) {
					boost::filesystem::recursive_directory_iterator end;
					
					for( boost::filesystem::recursive_directory_iterator i( musicPath ); i not_eq end; ++i ) {
						if( not is_directory( i->path() ) ) { //We've found a file
							/* Attempts to load a file as music. If successful, unload the file and add it to musicList.
							 * This way the game is certain to accept any file formats the music library can use.
							 * The problem is, it can accept non-music files too. Such as the Cybrinth executable.
							 * For now, we work around this by only allowing certain extensions based on the music decoders already enumerated.
							 * This is non-ideal because file extensions don't necessarily reflect their actual type, and certain file types (e.g. OGG containers) may have multiple extensions which we may not have thought to allow (e.g. the OGG container can use .ogg, .oga, .ogm, or potentally something else)
							 * TODO: Find a better way to detect music file formats.
							 * */
							
							auto extension = i->path().extension().wstring();
							
							if( settingsManager.debug ) {
								std::wcout << L"Music file extension: \"" << extension << L"\"" << std::endl;
							}
							
							if( std::find( allowedFileExtensions.begin(), allowedFileExtensions.end(), extension ) != allowedFileExtensions.end() ) {
								Mix_Music* temp = Mix_LoadMUS( i->path().c_str() );
								
								if( not isNull( temp ) ) {
									musicList.push_back( i->path() );
									
									if( settingsManager.debug ) {
										std::wcout << L"Appending file to music list: " << i->path() << std::endl;
									}
									
									Mix_FreeMusic( temp );
								}
							} else if( settingsManager.debug ) {
								std::wcout << L"File extension disallowed for music: \"" << extension << L"\"" << std::endl; 
							}
						}
					}
				}
				
				if( not musicList.empty() ) {
					break; //We found music; no need to keep searching more folders.
				}
			}
			
			useMusicSubfolder = not musicList.empty();
			if( useMusicSubfolder ) {
				break; //we found music; no need to try again without the music folder
			}
		}
		
		if( not musicList.empty() ) {
			//The random number generator must be seeded before this point
			std::shuffle( musicList.begin(), musicList.end(), randomNumberGenerator );
			
			currentMusic = musicList.back();
		} else {
			std::wcerr << L"Could not find any music to play. Turning off playback." << std::endl;
			settingsManager.setPlayMusic( false );
		}
		
		if( settingsManager.debug ) {
			std::wcout << L"end of makeMusicList()" << std::endl;
		}
	} catch( std::exception &e ) {
		std::wcerr << L"Error in MainGame::makeMusicList(): " << e.what() << std::endl;
	}
}

/**
 * Anything that should be done when a player moves, whichever axis they move on. Should only be called by movePlayerOnX() or movePlayerOnY().
 * Arguments:
 * --- p: The player to move
 */
 void MainGame::movePlayerCommon( uint_fast8_t p ) {
	mazeManager.maze[ player.at( p ).getX() ][ player.at( p ).getY() ].visited = true;
	if( player.at( p ).stepsTakenThisMaze % 2 == 0 ) {
		mazeManager.maze[ player.at( p ).getX() ][ player.at( p ).getY() ].setVisitorColor( player.at( p ).getColorTwo() );
	} else {
		mazeManager.maze[ player.at( p ).getX() ][ player.at( p ).getY() ].setVisitorColor( player.at( p ).getColorOne() );
	}
	mazeManager.makeCellsVisible( player.at( p ).getX(), player.at( p ).getY() );
}

/**
 * If the maze allows it, moves a player one unit in the indicated direction along the X axis.
 * Arguments:
 * --- p: the player to move
 * --- direction: a signed integer representing the direction to move.
 * --- fromServer: Indicates whether this was triggered as a result of the server sending a position update
 */
void MainGame::movePlayerOnX( uint_fast8_t p, int_fast8_t direction, bool fromServer ) {
	try {
		
		if( not isScreenSaver and not fromServer and( settingsManager.isServer or p == myPlayer ) ) {
			network.sendPlayerPosXMove( p, direction );
		}
		
		if( settingsManager.getNumPlayers() > p and mazeManager.cols > 0 ) {
			if( direction < 0 ) {
				if( player.at( p ).hasItem() and player.at( p ).getItemType() == Collectable::ACID and player.at( p ).getX() > 0 and mazeManager.maze[ player.at( p ).getX() ][ player.at( p ).getY() ].getLeft() not_eq MazeCell::ACIDPROOF and mazeManager.maze[ player.at( p ).getX() ][ player.at( p ).getY() ].getLeft() not_eq MazeCell::LOCK  and mazeManager.maze[ player.at( p ).getX() ][ player.at( p ).getY() ].getLeft() not_eq MazeCell::NONE ) {
					player.at( p ).removeItem();
					mazeManager.maze[ player.at( p ).getX() ][ player.at( p ).getY() ].setLeft( MazeCell::NONE );
				}

				if( player.at( p ).getX() > 0 and mazeManager.maze[ player.at( p ).getX() ][ player.at( p ).getY() ].getLeft() == MazeCell::NONE ) {
					player.at( p ).moveX( -1 );
				}
			} else {
				if( player.at( p ).hasItem() and player.at( p ).getItemType() == Collectable::ACID and player.at( p ).getX() < ( mazeManager.cols - 1 ) and mazeManager.maze[ player.at( p ).getX() + 1 ][ player.at( p ).getY() ].getLeft() not_eq MazeCell::ACIDPROOF and mazeManager.maze[ player.at( p ).getX() + 1 ][ player.at( p ).getY() ].getLeft() not_eq MazeCell::LOCK and mazeManager.maze[ player.at( p ).getX() + 1 ][ player.at( p ).getY() ].getLeft() not_eq MazeCell::NONE ) {
					player.at( p ).removeItem();
					mazeManager.maze[ player.at( p ).getX() + 1 ][ player.at( p ).getY() ].setLeft( MazeCell::NONE );
				}

				if( player.at( p ).getX() < ( mazeManager.cols - 1 ) and mazeManager.maze[ player.at( p ).getX() + 1 ][ player.at( p ).getY() ].getLeft() == MazeCell::NONE ) {
					player.at( p ).moveX( 1 );
				}
			}

			movePlayerCommon( p );
			if( player.at( p ).getX() >= mazeManager.cols ) {
				throw CustomException( L"Player "+ stringConverter.toStdWString( p ) + L"'s X (" + stringConverter.toStdWString( player.at( p ).getX() ) + L") is outside mazeManager's cols (" + stringConverter.toStdWString( mazeManager.cols ) + L")." );
			} else if( player.at( p ).getY() >= mazeManager.rows ) {
				throw CustomException( L"Player "+ stringConverter.toStdWString( p ) + L"'s Y (" + stringConverter.toStdWString( player.at( p ).getY() ) + L") is outside mazeManager's rows (" + stringConverter.toStdWString( mazeManager.rows ) + L")." );
			}
		} else {
			std::wstring e = L"Player " + stringConverter.toStdWString( p ) + L" is greater than settingsManager.getNumPlayers() (" + stringConverter.toStdWString( settingsManager.getNumPlayers() ) + L")";
			throw CustomException( e );
		}
	} catch( CustomException &e ) {
		std::wcerr << L"Error in MainGame::movePlayerOnX(): " << e.what() << std::endl;
	} catch( std::exception &e ) {
		std::wcerr << L"Error in MainGame::movePlayerOnX(): " << e.what() << std::endl;
	}
}

/**
 * If the maze allows it, moves a player one unit in the indicated direction along the Y axis.
 * Arguments:
 * --- p: the player to move
 * --- direction: a signed integer representing the direction to move.
 * --- fromServer: Indicates whether this was triggered as a result of the server sending a position update
 */
void MainGame::movePlayerOnY( uint_fast8_t p, int_fast8_t direction, bool fromServer ) {
	try {
		
		if( not isScreenSaver and not fromServer and ( settingsManager.isServer or p == myPlayer ) ) {
			network.sendPlayerPosYMove( p, direction );
		}
		
		if( settingsManager.getNumPlayers() > p and mazeManager.rows > 0 ) {
			if( direction < 0 ) {
				if( player.at( p ).hasItem() and player.at( p ).getItemType() == Collectable::ACID and player.at( p ).getY() > 0 and mazeManager.maze[ player.at( p ).getX() ][ player.at( p ).getY() ].getTop() not_eq MazeCell::ACIDPROOF and mazeManager.maze[ player.at( p ).getX() ][ player.at( p ).getY() ].getTop() not_eq MazeCell::LOCK and mazeManager.maze[ player.at( p ).getX() ][ player.at( p ).getY() ].getTop() not_eq MazeCell::NONE ) {
					player.at( p ).removeItem();
					mazeManager.maze[ player.at( p ).getX() ][ player.at( p ).getY() ].setTop( MazeCell::NONE );
				}

				if( player.at( p ).getY() > 0 and mazeManager.maze[ player.at( p ).getX() ][ player.at( p ).getY() ].getTop() == MazeCell::NONE ) {
					player.at( p ).moveY( -1 );
				}
			} else {
				if( player.at( p ).hasItem() and player.at( p ).getItemType() == Collectable::ACID and player.at( p ).getY() < ( mazeManager.rows - 1 ) and mazeManager.maze[ player.at( p ).getX() ][ player.at( p ).getY() + 1 ].getTop() not_eq MazeCell::ACIDPROOF and mazeManager.maze[ player.at( p ).getX() ][ player.at( p ).getY() + 1 ].getTop() not_eq MazeCell::LOCK and mazeManager.maze[ player.at( p ).getX() ][ player.at( p ).getY() + 1 ].getTop() not_eq MazeCell::NONE ) {
					player.at( p ).removeItem();
					mazeManager.maze[ player.at( p ).getX() ][ player.at( p ).getY() + 1 ].setTop( MazeCell::NONE );
				}

				if( player.at( p ).getY() < ( mazeManager.rows - 1 ) and mazeManager.maze[ player.at( p ).getX() ][ player.at( p ).getY() + 1 ].getTop() == MazeCell::NONE ) {
					player.at( p ).moveY( 1 );
				}
			}

			if( player.at( p ).getX() >= mazeManager.cols ) {
				throw CustomException( L"Player "+ stringConverter.toStdWString( p ) + L"'s X (" + stringConverter.toStdWString( player.at( p ).getX() ) + L") is outside mazeManager's cols (" + stringConverter.toStdWString( mazeManager.cols ) + L")." );
			} else if( player.at( p ).getY() >= mazeManager.rows ) {
				throw CustomException( L"Player "+ stringConverter.toStdWString( p ) + L"'s Y (" + stringConverter.toStdWString( player.at( p ).getY() ) + L") is outside mazeManager's rows (" + stringConverter.toStdWString( mazeManager.rows ) + L")." );
			}

			movePlayerCommon( p );
		} else {
			std::wstring e = L"Player " + stringConverter.toStdWString( p ) + L" is greater than settingsManager.getNumPlayers() (" + stringConverter.toStdWString( settingsManager.getNumPlayers() ) + L")";
			throw CustomException( e );
		}
	} catch( CustomException &e ) {
		std::wcerr << L"Error in MainGame::movePlayerOnY(): " << e.what() << std::endl;
	} catch( std::exception &e ) {
		std::wcerr << L"Error in MainGame::movePlayerOnY(): " << e.what() << std::endl;
	}
}

void MainGame::musicSettingChanged() {
	if( settingsManager.getPlayMusic() ) {
		setupMusicStuff();
		loadMusicFont();
	} else {
		Mix_HaltMusic();
		
		if( !isNull( music ) ) {
			Mix_FreeMusic( music );
		}
		
		Mix_CloseAudio();
		
		while( Mix_Init( 0 ) ) {
			Mix_Quit();
		}
		
		SDL_Quit();
	}
}

void MainGame::musicVolumeChanged() {
	Mix_VolumeMusic( settingsManager.getMusicVolume() * MIX_MAX_VOLUME / 100 );
}

/**
 * The network manager calls this as a way of requesting data to send to a new client.
 */
void MainGame::networkHasNewConnection() {
	network.sendMaze( randomSeed );
	
	for( decltype( myPlayer ) p = 0; p < settingsManager.getNumPlayers(); ++p ) {
		std::wcout << p << " ";
		if( !playerAssigned.at( p ) ) {
			std::wcout << "false";
		} else {
			std::wcout << "true";
		}
		std::wcout << std::endl;
	}
	
	for( decltype( myPlayer ) p = 0; p < settingsManager.getNumPlayers(); ++p ) {
		if( !playerAssigned.at( p ) ) {
			network.tellNewClientItsPlayer( p );
			playerAssigned.at( p ) = true;
			break;
		}
	}
	
	for( uint_fast8_t p = 0; p < settingsManager.getNumPlayers(); ++p ) {
		network.sendPlayerPos( p );
	}
}

/**
 * Calls the other newMaze()
 */
void MainGame::newMaze() {
	try {
		if( settingsManager.debug ) {
			std::wcout << L"newMaze() called with no arguments" << std::endl;
		}
		
		if( firstMaze ) {
			newMaze( randomSeed );
		} else {
			newMaze( getRandomNumber() );
		}
		
		if( settingsManager.debug ) {
			std::wcout << L"end of newMaze() with no arguments" << std::endl;
		}
	} catch( std::exception &e ) {
		std::wcerr << L"Error in MainGame::newMaze(): " << e.what() << std::endl;
	}
}

/**
 * Calls resetThings(), makes the maze manager load a maze from a file, then adjusts cellWidth and cellHeight.
 * Arguments:
 * --- boost::filesystem::path src: the file from which to load the maze.
 */
void MainGame::newMaze( boost::filesystem::path src ) {
	if( not loadSeedFromFile( src ) ) {
		//If we get this far, it's an error. Probably a file not found. Fail gracefully by starting a new maze anyway.
		gui->addMessageBox( L"Could not use file", L"Unable to load maze from file. Generating a new maze." );
		newMaze( getRandomNumber() );
	} else {
		newMaze( randomSeed );
	}
}

/**
 * Calls resetThings(), makes the maze manager load a maze from a file, then adjusts cellWidth and cellHeight.
 * Arguments:
 * --- std::minstd_rand::result_type newRandomSeed: The random seed to use.
 */
void MainGame::newMaze( std::minstd_rand::result_type newRandomSeed ) {
	try {
		if( settingsManager.debug ) {
			std::wcout << L"newMaze() called with an argument" << std::endl;
		}
		
		allPlayersReady( false );
		
		if( settingsManager.isServer and not isScreenSaver ) {
			network.sendMaze( newRandomSeed );
		}
		
		resetThings();
		setRandomSeed( newRandomSeed );
		
		mazeManager.makeRandomLevel();
		
		cellWidth = ( viewportSize.Width ) / mazeManager.cols;
		cellHeight = ( viewportSize.Height ) / mazeManager.rows;
		for( decltype( settingsManager.getNumBots() ) b = 0; b < settingsManager.getNumBots(); ++b ) {
			bot.at( b ).setup( this, settingsManager.botsKnowSolution, settingsManager.getBotAlgorithm(), settingsManager.botMovementDelay );
		}
		
		if( numLocks == 0 ) {
			for( decltype( settingsManager.getNumBots() ) b = 0; b < settingsManager.getNumBots(); ++b ) {
				bot.at( b ).allKeysFound(); //Lets all the bots know they don't need to search for keys
			}
		}
		
		setLoadingPercentage( 100 );
		
		if( not isScreenSaver ) {
			network.ImReadyToPlay();
		}
		
		if( settingsManager.debug ) {
			std::wcout << L"end of newMaze() with an argument" << std::endl;
		}
	} catch( std::exception &e ) {
		std::wcerr << L"Error in MainGame::newMaze(): " << e.what() << std::endl;
	}
}

/**
 * Called by Irrlicht every time an event (keyboard, mouse, controller, etc.) occurs.
 * Arguments:
 * --- const SEvent& event: an object representing the event that happened.
 * Returns: True if the event was handled by this function and does not involve any game controls (since the relationship of controls:actions can be many:many), false otherwise.
 */
//cppcheck-suppress unusedFunction
bool MainGame::OnEvent( const irr::SEvent& event ) {
	try {
		switch( currentScreen ) {
			case SETTINGSSCREEN: {
				return settingsScreen.OnEvent( event );
			}
			case MAINSCREEN:
			case MENUSCREEN:
			case LOADINGSCREEN:
			case WAITINGFORPLAYERSSCREEN: {
				switch( event.EventType ) {
					case irr::EET_KEY_INPUT_EVENT: {
						for( decltype( controls.size() ) k = 0; k < controls.size(); ++k ) {
							if( controls.at( k ).getKey() == event.KeyInput.Key ) {
								controls.at( k ).activated = event.KeyInput.PressedDown;
							}
						}
					}
					break;

					case irr::EET_MOUSE_INPUT_EVENT: {
						if( currentScreen == MENUSCREEN ) {
							switch( event.MouseInput.Event ) {
								case irr::EMIE_MOUSE_MOVED: {
									menuManager.findHighlights( event.MouseInput.X, event.MouseInput.Y );
									break;
								}
								case irr::EMIE_LMOUSE_PRESSED_DOWN: {
									if( ( gui->getRootGUIElement()->getChildren().getSize() == 0 ) ) {
										menuManager.processSelection();
									}
									break;
								}
								default: {
									break;
								}
							}
						}

						for( decltype( controls.size() ) k = 0; k < controls.size(); ++k ) {
							if( event.MouseInput.Event == controls.at( k ).getMouseEvent() ) {
								switch( controls.at( k ).getMouseEvent() ) {
									case irr::EMIE_MOUSE_WHEEL: {
										controls.at( k ).activated = ( ( event.MouseInput.Wheel > 0 and controls.at( k ).getMouseWheelUp() ) or ( event.MouseInput.Wheel < 0 and not controls.at( k ).getMouseWheelUp() ) );
										break;
									}
									case irr::EMIE_MOUSE_MOVED: {
										switch( controls.at( k ).getMouseDirection() ) {
											case ControlMapping::MOUSE_UP: {
												controls.at( k ).activated = ( event.MouseInput.Y < mouseY );
												break;
											}
											case ControlMapping::MOUSE_DOWN: {
												controls.at( k ).activated = ( event.MouseInput.Y > mouseY );
												break;
											}
											case ControlMapping::MOUSE_LEFT: {
												controls.at( k ).activated = ( event.MouseInput.X < mouseX );
												break;
											}
											case ControlMapping::MOUSE_RIGHT: {
												controls.at( k ).activated = ( event.MouseInput.X > mouseX );
												break;
											}
											default: { //ControlMapping::MOUSE_DO_NOT_USE
												break;
											}
										}
										break;
									}
									default: {
										//TODO: Add mouse handling stuff here
										break;
									}
								}
							}
						}
						
						switch( event.MouseInput.Event ) { //Anything that should be updated regardless of whether it's required by any controls
							case irr::EMIE_MOUSE_MOVED: { //So far this is the only thing that should always be updated
								mouseX = event.MouseInput.X;
								mouseY = event.MouseInput.Y;
								break;
							}
							default: {
								break;
							}
						}
					}
					break;

					case irr::EET_USER_EVENT: {
						switch( event.UserEvent.UserData1 ) {
							case USER_EVENT_WINDOW_RESIZE: {
								screenSize.set( driver->getScreenSize().Width, driver->getScreenSize().Height );

								//TODO: Find a way to resize the window or set a minimum size
								/*if (!allowSmallSize) {
									bool sizeChanged = false;

									if (screenSize.Height < minHeight) {
										screenSize.Height = minHeight;
										sizeChanged = true;
									}

									if (screenSize.Width < minWidth) {
										screenSize.Width = minWidth;
										sizeChanged = true;
									}

									if (sizeChanged) {
										driver->OnResize( screenSize );
									}
								}*/

								viewportSize.set( screenSize.Width - ( screenSize.Width / sideDisplaySizeDenominator ), screenSize.Height - 1 );
								cellWidth = ( viewportSize.Width ) / mazeManager.cols;
								cellHeight = ( viewportSize.Height ) / mazeManager.rows;
								loadFonts();
								
								menuManager.setPositions( screenSize.Height );
								settingsScreen.setupIconsAndStuff();
								
								if( settingsManager.showBackgrounds ) {
									
									irr::scene::ICameraSceneNode* camera = backgroundSceneManager->getActiveCamera();
									if( not isNull( camera ) ) {
										camera->setAspectRatio( static_cast< decltype( camera->getAspectRatio() ) >( screenSize.Width ) / screenSize.Height );
									}
									
									if( backgroundChosen == IMAGES and not isNull( backgroundTexture ) and backgroundTexture->getSize() not_eq screenSize ) {
										if( backgroundFilePath.size() > 0 ) {// not backgroundFilePath.empty() ) { Irrlicht 1.8+ has .empty() but Raspbian only has 1.7 in its repositories
											backgroundTexture = driver->getTexture( backgroundFilePath );
										}
										if( not isNull( backgroundTexture ) and backgroundTexture->getSize() not_eq screenSize ) {
											backgroundTexture = resizer.resize( backgroundTexture, screenSize.Width, screenSize.Height, driver );
										}
									} else if( backgroundChosen == STAR_TRAILS and backgroundTexture->getSize() not_eq screenSize ) {
										driver->removeTexture( backgroundTexture );
										backgroundTexture = driver->addRenderTargetTexture( screenSize );
									}
								}
								return true;
							}
							break;
							default:
								break;
						}
					}
					break;

					case irr::EET_JOYSTICK_INPUT_EVENT: {
						if( enableController ) {
							for( decltype( controls.size() ) k = 0; k < controls.size(); ++k ) {
								if( event.JoystickEvent.Joystick == controls.at( k ).getControllerNumber() ) {
									{ //Handle joystick axes
										int_fast16_t joystickDeadZone = controls.at( k ).getJoystickDeadZone(); //TODO: Make the dead zone user adjustable.
										
										if( controls.at( k ).getJoystickDirection() == ControlMapping::JOYSTICK_INCREASE ) {
											if( controls.at( k ).getJoystickAxis() == ( decltype( controls.at( k ).getJoystickAxis() ) ) irr::SEvent::SJoystickEvent::AXIS_X ) {
												controls.at( k ).activated = ( event.JoystickEvent.Axis[ irr::SEvent::SJoystickEvent::AXIS_X ] > joystickDeadZone );
												
											} else if( controls.at( k ).getJoystickAxis() == ( decltype( controls.at( k ).getJoystickAxis() ) ) irr::SEvent::SJoystickEvent::AXIS_Y ) {
												controls.at( k ).activated = ( event.JoystickEvent.Axis[ irr::SEvent::SJoystickEvent::AXIS_Y ] > joystickDeadZone );
												
											} else if( controls.at( k ).getJoystickAxis() == ( decltype( controls.at( k ).getJoystickAxis() ) ) irr::SEvent::SJoystickEvent::AXIS_Z ) {
												controls.at( k ).activated = ( event.JoystickEvent.Axis[ irr::SEvent::SJoystickEvent::AXIS_Z ] > joystickDeadZone );
												
											} else if( controls.at( k ).getJoystickAxis() == ( decltype( controls.at( k ).getJoystickAxis() ) ) irr::SEvent::SJoystickEvent::AXIS_R ) {
												controls.at( k ).activated = ( event.JoystickEvent.Axis[ irr::SEvent::SJoystickEvent::AXIS_R ] > joystickDeadZone );
												
											} else if( controls.at( k ).getJoystickAxis() == ( decltype( controls.at( k ).getJoystickAxis() ) ) irr::SEvent::SJoystickEvent::AXIS_U ) {
												controls.at( k ).activated = ( event.JoystickEvent.Axis[ irr::SEvent::SJoystickEvent::AXIS_U ] > joystickDeadZone );
												
											} else if( controls.at( k ).getJoystickAxis() == ( decltype( controls.at( k ).getJoystickAxis() ) ) irr::SEvent::SJoystickEvent::AXIS_V ) {
												controls.at( k ).activated = ( event.JoystickEvent.Axis[ irr::SEvent::SJoystickEvent::AXIS_V ] > joystickDeadZone );
												
											}
										} else if( controls.at( k ).getJoystickDirection() == ControlMapping::JOYSTICK_DECREASE ) {
											if( controls.at( k ).getJoystickAxis() == ( decltype( controls.at( k ).getJoystickAxis() ) ) irr::SEvent::SJoystickEvent::AXIS_X ) {
												controls.at( k ).activated = ( event.JoystickEvent.Axis[ irr::SEvent::SJoystickEvent::AXIS_X ] < -joystickDeadZone );
												
											} else if( controls.at( k ).getJoystickAxis() == ( decltype( controls.at( k ).getJoystickAxis() ) ) irr::SEvent::SJoystickEvent::AXIS_Y ) {
												controls.at( k ).activated = ( event.JoystickEvent.Axis[ irr::SEvent::SJoystickEvent::AXIS_Y ] < -joystickDeadZone );
												
											} else if( controls.at( k ).getJoystickAxis() == ( decltype( controls.at( k ).getJoystickAxis() ) ) irr::SEvent::SJoystickEvent::AXIS_Z ) {
												controls.at( k ).activated = ( event.JoystickEvent.Axis[ irr::SEvent::SJoystickEvent::AXIS_Z ] < -joystickDeadZone );
												
											} else if( controls.at( k ).getJoystickAxis() == ( decltype( controls.at( k ).getJoystickAxis() ) ) irr::SEvent::SJoystickEvent::AXIS_R ) {
												controls.at( k ).activated = ( event.JoystickEvent.Axis[ irr::SEvent::SJoystickEvent::AXIS_R ] < -joystickDeadZone );
												
											} else if( controls.at( k ).getJoystickAxis() == ( decltype( controls.at( k ).getJoystickAxis() ) ) irr::SEvent::SJoystickEvent::AXIS_U ) {
												controls.at( k ).activated = ( event.JoystickEvent.Axis[ irr::SEvent::SJoystickEvent::AXIS_U ] < -joystickDeadZone );
												
											} else if( controls.at( k ).getJoystickAxis() == ( decltype( controls.at( k ).getJoystickAxis() ) ) irr::SEvent::SJoystickEvent::AXIS_V ) {
												controls.at( k ).activated = ( event.JoystickEvent.Axis[ irr::SEvent::SJoystickEvent::AXIS_V ] < -joystickDeadZone );
												
											}
										}
									}
									
									//Handle controller buttons
									if( controls.at( k ).getControllerButton() not_eq UINT_FAST8_MAX ) {
										controls.at( k ).activated = event.JoystickEvent.IsButtonPressed( controls.at( k ).getControllerButton() );
									}
								}
							}
						}
					}
					break;

					case irr::EET_GUI_EVENT: {
						switch( event.GUIEvent.EventType ) {
							case irr::gui::EGET_FILE_SELECTED: {
								if( not isNull( loadMazeDialog ) and event.GUIEvent.Caller->getID() == loadMazeDialog->getID() ) {
									if( settingsManager.debug ) {
										std::wcout << L"File selected for loading. Folder: " << stringConverter.toStdWString( loadMazeDialog->getDirectoryName() ) << L"\tFile: " << loadMazeDialog->getFileName() << std::endl;
									}
									boost::filesystem::current_path( currentDirectory ); //Resets the actual current directory to currentDirectory, since the file chooser can change the current directory.

									newMaze( loadMazeDialog->getFileName() );
									loadMazeDialog = nullptr;
									return true;
								} else if( not isNull( saveMazeDialog ) and event.GUIEvent.Caller->getID() == saveMazeDialog->getID() ) {
									if( settingsManager.debug ) {
										std::wcout << L"File selected for saving. Folder: " << stringConverter.toStdWString( loadMazeDialog->getDirectoryName() ) << L"\tFile: " << loadMazeDialog->getFileName() << std::endl;
									}
									boost::filesystem::current_path( currentDirectory ); //Resets the actual current directory to currentDirectory, since the file chooser can change the current directory.

									mazeManager.saveToFile( saveMazeDialog->getFileName() );
									saveMazeDialog = nullptr;
									return true;
								}
								break;
							}
							case irr::gui::EGET_DIRECTORY_SELECTED: {
								if( not isNull( loadMazeDialog ) and event.GUIEvent.Caller->getID() == loadMazeDialog->getID() ) {
									if( settingsManager.debug ) {
										std::wcout << L"Folder selected." << std::endl;
									}
									return true;
								} else if( not isNull( saveMazeDialog ) and event.GUIEvent.Caller->getID() == saveMazeDialog->getID() ) {
									if( settingsManager.debug ) {
										std::wcout << L"Folder selected." << std::endl;
									}
									return true;
								}

								break;
							}
							case irr::gui::EGET_MESSAGEBOX_YES: {
								if( event.GUIEvent.Caller->getID() == exitConfirmation->getID() ) {
									device->closeDevice();
									donePlaying = true;
									exitConfirmation = nullptr;
									return true;
								}
								break;
							}
							case irr::gui::EGET_MESSAGEBOX_NO: {
								if( event.GUIEvent.Caller->getID() == exitConfirmation->getID() ) {
									exitConfirmation = nullptr;
									return true;
								}
								break;
							}
							case irr::gui::EGET_FILE_CHOOSE_DIALOG_CANCELLED: //Deliberate fall-through
							case irr::gui::EGET_ELEMENT_CLOSED: {
								auto caller = event.GUIEvent.Caller;
								if( caller == exitConfirmation ) {
									exitConfirmation = nullptr;
								} else if( caller == loadMazeDialog ) {
									loadMazeDialog = nullptr;
								} else if( caller == saveMazeDialog ) {
									saveMazeDialog = nullptr;
								} else {
									std::wcerr << L"An unrecognized element was just closed." << std::endl;
								}
								break;
							}
							default: {
								break;
							}
						}
					}
					default: {
						break;
					}
					break;
				}
				
				break;
			}
			default: {
				std::wcerr << L"Error in MainGame::OnEvent(): currentScreen " << ( unsigned int ) currentScreen << L" not handled in switch statement." << std::endl;
				break;
			}
		}

	} catch( std::exception &e ) {
		std::wcerr << L"Error in MainGame::OnEvent(): " << e.what() << std::endl;
	}

	return false;
}

/**
 * Randomly selects a logo. Should NOT be called by drawAll().
 */
 void MainGame::pickLogo() {
	try {
		if( settingsManager.debug ) {
			std::wcout << L"pickLogo() called" << std::endl;
		}
		
		boost::filesystem::path logoPath( boost::filesystem::current_path()/L"Images/logos" );
		
		std::vector< boost::filesystem::path > logoList = getLoadableTexturesList( logoPath );
		
		auto folders = system.getImageFolders();
		for( auto iter = folders.begin(); logoList.empty() and iter != folders.end(); ++iter ) {
			auto tempLogoList = getLoadableTexturesList( *iter / L"logos" );
			for( auto innerIter = tempLogoList.begin(); innerIter != tempLogoList.end(); ++innerIter ) {
				logoList.push_back( *innerIter );
			}
		}
		
		if( logoList.size() > 0 ) {
			std::vector< boost::filesystem::path >::iterator newEnd = std::unique( logoList.begin(), logoList.end() ); //unique "removes all but the first element from every consecutive group of equivalent elements in the range [first,last)." (source: http://www.cplusplus.com/reference/algorithm/unique/ )
			logoList.resize( std::distance( logoList.begin(), newEnd ) );
			
			//Pick a random logo and load it
			auto logoChosen = getRandomNumber() % logoList.size();
			if( settingsManager.debug ) {
				std::wcout << L"Logo chosen: #" << logoChosen << L"/" << logoList.size();
				std::wcout << L" " << logoList.at( logoChosen ).wstring() << std::endl;
			}
			irr::io::path logoFilePath = stringConverter.toIrrlichtStringW( logoList.at( logoChosen ).wstring() );
			logoTexture = driver->getTexture( logoFilePath );
			
			if( not isNull( logoTexture ) ) {
				ImageModifier resizer;
				
				if( logoTexture->getSize() not_eq screenSize ) {
					auto newTexture = resizer.resize( logoTexture, screenSize.Width, screenSize.Height, driver );
					driver->removeTexture( logoTexture );
					logoTexture = newTexture;
				}
				
				irr::video::IImage* image = resizer.textureToImage( driver, logoTexture );
				irr::core::stringw textureName = logoTexture->getName().getInternalName(); //Needed when converting the image back to a texture
				driver->removeTexture( logoTexture );
				logoTexture = nullptr;
				
				adjustImageColors( image );
				
				textureName += L"-recolored";
				logoTexture = resizer.imageToTexture( driver, image, textureName );
			}
			
			if( isNull( logoTexture ) ) {
				std::wstring error = L"Cannot load logo texture, even though Irrlicht said it was loadable?!?";
				throw CustomException( error );
			}
			
			drawLogo();
		} else {
			std::wcerr << L"Could not find any logo images." << std::endl;
		}
		
		if( settingsManager.debug ) {
			std::wcout << L"end of pickLogo()" << std::endl;
		}
	} catch ( std::exception &error ) {
		std::wcerr << L"Error in drawLogo(): " << error.what() << std::endl;
	}
 }

/**
* Should be called only by run().
* Arguments:
* None.
*/
void MainGame::processControls() {
	try {
		for( decltype( controls.size() ) k = 0; k < controls.size(); ++k ) {
			if( controls.at( k ).activated ) {
				if ( controls.at( k ).getMouseEvent() == irr::EMOUSE_INPUT_EVENT::EMIE_MOUSE_WHEEL ) { //There's no event for when the wheel stops moving, so we're manually deactivating those controls
					controls.at( k ).activated = false;
				}
				
				switch( controls.at( k ).getAction() ) {
					case ControlMapping::ACTION_MENU_ACTIVATE: {
						switch( currentScreen ) {
							case MENUSCREEN: {
								menuManager.processSelection();
								break;
							}
							case SETTINGSSCREEN: {
								//TODO:Settings stuff
								std::wcout << L"ACTION_MENU_ACTIVATE control used on settings screen." << std::endl;
								break;
							}
							case MAINSCREEN: {
								currentScreen = MENUSCREEN;
								break;
							}
							default: {
								throw CustomException( L"ACTION_MENU_ACTIVATE control used on a screen which is not in the switch statement." );
							}
						}
						break;
					}
					case ControlMapping::ACTION_MENU_UP: {
						switch( currentScreen ) {
							case MENUSCREEN: {
								menuManager.scrollSelection( true );
								break;
							}
							default: {
								//ignore
							}
						}
						break;
					}
					case ControlMapping::ACTION_MENU_DOWN: {
						switch( currentScreen ) {
							case MENUSCREEN: {
								menuManager.scrollSelection( false );
								break;
							}
							default: {
								//ignore
							}
						}
						break;
					}
					case ControlMapping::ACTION_SCREENSHOT: {
						takeScreenShot();
						break;
					}
					case ControlMapping::ACTION_VOLUME_UP: {
						settingsManager.setMusicVolume( settingsManager.getMusicVolume() + 5 );
						break;
					}
					case ControlMapping::ACTION_VOLUME_DOWN: {
						if( settingsManager.getMusicVolume() >= 5 ) {
							settingsManager.setMusicVolume( settingsManager.getMusicVolume() - 5 );
						} else {
							settingsManager.setMusicVolume( 0 );
						}
						
						musicVolumeChanged();//Mix_VolumeMusic( settingsManager.musicVolume * MIX_MAX_VOLUME / 100 );
						break;
					}
					default: { //Handle player controls
						bool ignoreKey = false;
						
						if( currentScreen not_eq MAINSCREEN ) { //Don't move the players if the game is paused.
							ignoreKey = true;
						}
						
						for( decltype( settingsManager.getNumBots() ) b = 0; not settingsManager.isServer and not ignoreKey and b < settingsManager.getNumBots(); ++b ) { //Ignore controls that affect bots
							if( controls.at( k ).getPlayer() == bot.at( b ).getPlayer() ) {
								ignoreKey = true;
							}
						}
						
						if( not settingsManager.isServer and controls.at( k ).getPlayer() not_eq myPlayer ) {
							ignoreKey = true;
						}
						
						if( not ignoreKey ) {
							switch( controls.at( k ).getAction() ) {
								case ControlMapping::ACTION_PLAYER_UP: {
									movePlayerOnY( controls.at( k ).getPlayer(), -1, false );
									break;
								}
								case ControlMapping::ACTION_PLAYER_DOWN: {
									movePlayerOnY( controls.at( k ).getPlayer(), 1, false );
									break;
								}
								case ControlMapping::ACTION_PLAYER_RIGHT: {
									movePlayerOnX( controls.at( k ).getPlayer(), 1, false );
									break;
								}
								case ControlMapping::ACTION_PLAYER_LEFT: {
									movePlayerOnX( controls.at( k ).getPlayer(), -1, false );
									break;
								}
								default: {
									break;
								}
							}
						}
						break;
					}
				}
			}
		}
	} catch( std::exception &e ) {
		std::wcerr << L"Error in MainGame::processControls(): " << e.what() << std::endl;
	}
}


/**
 * @brief Prompts the user for the IP and port of the server to which to connect
 */
void MainGame::promptForServerIP() {
	
	//This function currently only gets called by MenuManager, thus it's impossible for currentScreen to be anything other than MENUSCREEN. I have learned to prepare for the impossible.
	if( currentScreen != MENUSCREEN ) {
		//We don't want to interrupt gameplay by popping up a dialog over the play screen; thus we must ensure the game is paused by changing to the menu screen.
		currentScreen = MENUSCREEN;
	}
	
	settingsManager.isServer = false;
	
	std::wstring newIP = L"";
	std::wcout << L"Server IP: ";
	std::wcin >> newIP;
	network.setIP( newIP );
	
	uint_fast16_t newPort;
	std::wcout << L"Port: ";
	std::wcin >> newPort;
	network.setPort( newPort );
	
	network.setup( this, settingsManager.isServer );
}

/**
 * Resets miscellaneous stuff between mazes.
 */
void MainGame::resetThings() {
	try {
		if( settingsManager.debug ) {
			std::wcout << L"resetThings() called" << std::endl;
		}
		
		setLoadingPercentage( 0 );
		
		setRandomSeed( time( nullptr ) );
		
		//The delay exists so that people can admire the logo artwork or read the pro tips on the loading screen. Actual loading happens in the blink of an eye on my computer.
		if( not haveShownLogo ) {
			loadingDelay = 6000;
		} else {
			loadingDelay = 1000 + ( getRandomNumber() % 5000 ); //Adds some randomness just to make it seem less artificial.
		}

		winnersLoadingScreen = winners;
		winners.clear();
		stuff.clear();
		numKeysFound = 0;
		numLocks = 0;
		donePlaying = false;

		for( decltype( settingsManager.getNumPlayers() ) p = 0; p < settingsManager.getNumPlayers(); ++p ) {
			playerStart.at( p ).reset();
			player.at( p ).reset();
		}

		//Calculate players' total scores
		for( decltype( settingsManager.getNumPlayers() ) w = 0; w < winnersLoadingScreen.size(); ++w ) { //changed decltype( winnersLoadingScreen.size() ) to decltype( numPlayers ) because winnersLoadingScreen.size() can never exceed numPlayers but can be stored in a needlessly slow integer type.
			decltype( player.at( winnersLoadingScreen.at( w ) ).getScoreTotal() ) score = 0;
			decltype( player.at( winnersLoadingScreen.at( w ) ).getScoreTotal() ) additiveMultiplier = 10; //So the scores don't get too negative, numbers that add to the score get multiplied by a magic number.
			decltype( player.at( winnersLoadingScreen.at( w ) ).getScoreTotal() ) subtractiveDivisor = 10; //Likewise, numbers that subtract from the score get divided by a magic number.
			
			if( settingsManager.debug ) {
				std::wcout << L"Setting player " << winnersLoadingScreen.at( w ) << L"'s score to ";
			}
			{
				decltype( score ) temp = ( winnersLoadingScreen.size() - w ) * additiveMultiplier;
				score += temp;
				if( settingsManager.debug ) {
					std::wcout << temp;
				}
			}
			{
				decltype( score ) temp = ( player.at( winnersLoadingScreen.at( w ) ).stepsTakenLastMaze ) / subtractiveDivisor;
				score -= temp;
				if( settingsManager.debug ) {
					std::wcout << L" - " << temp;
				}
			}
			{
				decltype( score ) temp = player.at( winnersLoadingScreen.at( w ) ).timeTakenLastMaze / 1000 / subtractiveDivisor; //The 1000 is for converting the time to seconds
				score -= temp;
				if( settingsManager.debug ) {
					std::wcout << L" - " << temp;
				}
			}
			{
				decltype( score ) temp = player.at( winnersLoadingScreen.at( w ) ).keysCollectedLastMaze * additiveMultiplier;
				score += temp;
				if( settingsManager.debug ) {
					std::wcout << L" + " << temp << L" for a total of " << score << std::endl;
				}
			}
			
			player.at( winnersLoadingScreen.at( w ) ).setScore( score );
		}

		for( decltype( settingsManager.getNumBots() ) b = 0; b < settingsManager.getNumBots(); ++b ) {
			bot.at( b ).reset();
		}

		for( decltype( stuff.size() ) i = 0; i < stuff.size(); ++i ) {
			stuff.at( i ).setColorMode( settingsManager.colorMode );
			stuff.at( i ).loadTexture( device );
		}

		for( decltype( mazeManager.cols ) x = 0; x < mazeManager.cols; ++x ) {
			for( decltype( mazeManager.rows ) y = 0; y < mazeManager.rows; ++y ) {
				mazeManager.maze[ x ][ y ].visited = false;
			}
		}

		won = false;
		backgroundSceneManager->clear();

		if( settingsManager.showBackgrounds ) {
			setupBackground();
		}
		timer->setTime( 0 );
		timer->start();
		
		if( not proTips.empty() ) {
			currentProTip = ( currentProTip + 1 ) % proTips.size(); //This generates a floating point error if proTips.size() is 0, hence the surrounding if block.
		} else {
			currentProTip = 0;
		}
		
		if( settingsManager.debug ) {
			std::wcout << L"Current pro tip #: " << currentProTip << std::endl;
		}
		
		loadTipFont();
		startLoadingScreen();
		lastTimeControlsProcessed = timer->getTime();
	} catch( std::exception &e ) {
		std::wcerr << L"Error in MainGame::resetThings(): " << e.what() << std::endl;
	}
	
	if( settingsManager.debug ) {
		std::wcout << L"end of resetThings()" << std::endl;
	}
}

/**
 * The game's main loop. Should only be called by main() in main.cpp
 * Returns: EXIT_SUCCESS if the game exits normally, EXIT_FAILURE if an exception is caught.
 */
uint_fast8_t MainGame::run() {
	try {
		if( settingsManager.debug ) {
			std::wcout << L"run() called" << std::endl;
		}
		
		while( device->run() and not donePlaying ) {
			if( settingsManager.isServer ) { //If we're not a server, and thus are connecting to one, then the server will send us a random seed. We will generate a new maze at that time.
				newMaze();
			} else {
				network.processPackets();
			}
			
			haveShownLogo = true; //This should only ever be false at the start of the program.
			
			while( device->run() and not won and not donePlaying ) {
				
				if( currentScreen == LOADINGSCREEN and ( timer->getRealTime() > timeStartedLoading + loadingDelay ) ) {
					currentScreen = MAINSCREEN;
				}
				
				if( driver->getFPS() > 60 ) {
					/*These next 3 lines are for limiting processor usage. I really
					 *doubt they're all needed, so comment one or another if you
					 *experience slowness. Two are provided by Irrlicht and the other by SDL.*/
					device->yield();
					//device->sleep( 17 ); //17 = 1/60 of a second, rounded up, in milliseconds. My monitor refreshes at 60 Hz.
					//SDL_Delay( 17 );
				}
				
				if( driver->getScreenSize() not_eq screenSize ) { //If the window has been resized. Only here until Irrlicht implements proper window resize events.
					irr::SEvent temp;
					temp.EventType = irr::EET_USER_EVENT;
					temp.UserEvent.UserData1 = USER_EVENT_WINDOW_RESIZE;
					device->postEventFromUser( temp );
				}

				if( settingsManager.getPlayMusic() and not Mix_PlayingMusic() ) { //If we've finished playing a song.
					loadNextSong();
				}
				
				if( not isScreenSaver ) {
					auto time = timer->getRealTime(); //getRealTime() works even if the timer is stopped, as it is when the game is paused.
					if( time >= lastTimeControlsProcessed + controlProcessDelay or time < lastTimeControlsProcessed ) {
						processControls();
						lastTimeControlsProcessed = time;
					}
				}
				
				if( ( currentScreen != LOADINGSCREEN and ( isScreenSaver or device->isWindowActive() ) ) or settingsManager.debug ) {
					//It's the bots' turn to move now.
					if( currentScreen == MAINSCREEN and settingsManager.getNumBots() > 0 ) {
						for( decltype( settingsManager.getNumBots() ) i = 0; i < settingsManager.getNumBots(); ++i ) {
							if( not bot.at( i ).atGoal() and ( allHumansAtGoal() or bot.at( i ).doneWaiting() ) ) {
								bot.at( i ).move();
							}
						}
					}
					
					device->getCursorControl()->setVisible( currentScreen not_eq MAINSCREEN or settingsManager.debug );
					drawAll();
					
					//Check if any of the players have landed on a collectable item
					for( decltype( settingsManager.getNumPlayers() ) p = 0; p < settingsManager.getNumPlayers(); ++p ) {
						for( decltype( stuff.size() ) s = 0; s < stuff.size(); ++s ) {
							if( not stuff.at( s ).owned and player.at( p ).getX() == stuff.at( s ).getX() and player.at( p ).getY() == stuff.at( s ).getY() ) {
								switch( stuff.at( s ).getType() ) {
									case Collectable::ACID: {
										bool anyPlayerHasItem = stuff.at( s ).owned;

										if( not anyPlayerHasItem ) {
											player.at( p ).giveItem( s, stuff.at( s ).getType() );
										}

										break;
									}
									case Collectable::KEY: {
										++numKeysFound;
										player.at( p ).keysCollectedThisMaze += 1;
										eraseCollectable( s );
										//stuff.erase( stuff.begin() + s );

										if( numKeysFound >= numLocks ) {
											for( decltype( mazeManager.cols ) c = 0; c < mazeManager.cols; ++c ) {
												for( decltype( mazeManager.rows ) r = 0; r < mazeManager.rows; ++r ) {
													mazeManager.maze[ c ][ r ].removeLocks();
												}
											}

											for( decltype( settingsManager.getNumBots() ) b = 0; b < settingsManager.getNumBots(); ++b ) {
												bot.at( b ).allKeysFound();
											}
										} else {
											for( decltype( settingsManager.getNumBots() ) b = 0; b < settingsManager.getNumBots(); ++b ) {
												bot.at( b ).keyFound( s );
											}
										}

									break;
									}
									default:
										break;
								}
							}
						}
					}


					for( decltype( settingsManager.getNumPlayers() ) p = 0; p < settingsManager.getNumPlayers(); ++p ) {
						if( ( player.at( p ).getX() == goal.getX() ) and player.at( p ).getY() == goal.getY() ) { //Make a list of who finished in what order
							bool alreadyFinished = false; //Indicates whether the player is already on the winners list

							for( decltype( settingsManager.getNumPlayers() ) i = 0; i < winners.size() and not alreadyFinished; ++i ) { //changed decltype( winners.size() ) to decltype( numPlayers ) because winners.size() can never exceed numPlayers but can be stored in a needlessly slow integer type.
								if( p == winners.at( i ) ) {
									alreadyFinished = true;
								}
							}

							if( not alreadyFinished ) {
								player.at( p ).timeTakenThisMaze = timer->getTime();
								winners.push_back( p );
							}
						}
					}

					won = ( winners.size() >= settingsManager.getNumPlayers() ); //If all the players are on the winners list, we've won.

				} else if( not device->isWindowActive() and not isScreenSaver ) { //if(( not showingLoadingScreen and device->isWindowActive() ) or debug )
					if( currentScreen != MENUSCREEN and currentScreen != SETTINGSSCREEN ) {
						currentScreen = MENUSCREEN;
					}
					device->yield();
				}
				
				if( currentScreen == MENUSCREEN and not timer->isStopped() ) {
					timer->stop();
				} else if( currentScreen not_eq MENUSCREEN and timer->isStopped() ) {
					timer->start();
				}
				
				//TODO: add networking stuff here
				if( ( settingsManager.isServer or network.getConnectionStatus() ) and not isScreenSaver ) {
					network.processPackets();
				}
				
				if( !settingsManager.isServer and ( isScreenSaver or not network.getConnectionStatus() ) ) {
					donePlaying = true;
				}
			}

			timer->stop();

			if( not donePlaying ) {
				if( settingsManager.debug ) {
					std::wcout << L"On to the next level! " << std::endl;
					std::wcout << L"Winners:";

					for( decltype( settingsManager.getNumPlayers() ) i = 0; i < winners.size(); ++i ) { //changed decltype( winners.size() ) to decltype( numPlayers ) because winners.size() can never exceed numPlayers but can be stored in a needlessly slow integer type.
						std::wcout << L" " << winners.at( i );
					}

					std::wcout << std::endl;
				}
				startLoadingScreen();
			}
		}
	} catch( std::exception &e ) {
		std::wcerr << L"Error in MainGame::run(): " << e.what() << std::endl;
		return EXIT_FAILURE;
	}
	
	if( settingsManager.debug ) {
		std::wcout << L"end of run()" << std::endl;
	}
	
	return EXIT_SUCCESS;
}

/**
 * Loads control settings from controls.cfg.
 */
void MainGame::setControls() {
	try {
		if( settingsManager.debug ) {
			std::wcout << L"setControls() called" << std::endl;
		}
		
		//Yeah, these are the only defaults so far.
		//TODO: Add default controls.
		enableController = false;
		joystickDeadZoneDefaultPercent = 50;
		std::map< uint_fast8_t, uint_fast8_t > joystickDeadZones;
		for( uint_fast8_t i = 0; i < UINT_FAST8_MAX; ++i ) {
			joystickDeadZones[ i ] = joystickDeadZoneDefaultPercent;
		}
		
		std::vector< boost::filesystem::path > configFolders = system.getConfigFolders(); // Flawfinder: ignore
		bool controlsFileFound = false;
		
		for( auto it = configFolders.begin(); it not_eq configFolders.end(); ++it ) {
			boost::filesystem::path controlsPath( *it/L"controls.cfg" );
			if( exists( controlsPath ) and not is_directory( controlsPath ) ) {
				//if( settingsManager.debug ) {
					std::wcout << L"Loading controls from file " << controlsPath.wstring() << std::endl;
				//}
				boost::filesystem::wifstream controlsFile;
				controlsFile.open( controlsPath );
				
				if( controlsFile.is_open() ) {
					controlsFileFound = true;
					uintmax_t lineNum = 0; //This used to be a uint_fast8_t, which should be enough, but when dealing with user input we don't want to make any assumptions.
					
					while( controlsFile.good() ) {
						++lineNum;
						std::wstring line;
						getline( controlsFile, line );
						line = line.substr( 0, line.find( L"//" ) ); //Filters out comments
						boost::algorithm::trim_all( line ); //Removes trailing and leading spaces, and spaces in the middle are reduced to one character
						boost::algorithm::to_lower( line ); //Converts to lower case
						if( settingsManager.debug ) {
							std::wcout << L"Line " << lineNum << L": \"" << line << "\"" << std::endl;
						}
						
						if( not line.empty() ) { //The line may be empty, either in the actual file or the result of removing comments and spaces
							try {
								std::wstring preference = boost::algorithm::trim_copy( line.substr( 0, line.find( L'\t' ) ) );
								std::wstring choiceStr = boost::algorithm::trim_copy( line.substr( line.find( L'\t' ) ) );
								if( settingsManager.debug ) {
									std::wcout << L"Control preference \"" << preference << L"\" choiceStr \"" << choiceStr << L"\""<< std::endl;
								}
								
								{
									ControlMapping temp;
									controls.push_back( temp );
								}
									
								std::vector< std::wstring > possibleChoiceStarts = { L"controller", L"key", L"mouse" };
								
								if( spellChecker.DamerauLevenshteinDistance( choiceStr.substr( 0, possibleChoiceStarts.at( 0 ).length() ), possibleChoiceStarts.at( 0 ) ) <= 1 ) {//L"controller"
									choiceStr = choiceStr.substr( possibleChoiceStarts.at( 0 ).length(), choiceStr.length() - possibleChoiceStarts.at( 0 ).length() ); //possibleChoiceStarts.at( 0 ).length() = length of the word "controller"
									boost::algorithm::trim( choiceStr ); //Remove trailing and leading spaces
									{
										std::wstring controllerNumStr = boost::algorithm::trim_copy( choiceStr.substr( 0, choiceStr.find( L' ' ) ) );
										
										if( settingsManager.debug ) {
											std::wcout << L"controller number (string) \"" << controllerNumStr << L"\"" << std::endl;
										}
										
										uint_fast8_t choice;
										
										choice = static_cast< uint_fast8_t >( boost::lexical_cast< unsigned short int >( controllerNumStr ) ); //Boost lexical cast can't convert directly to uint_fast8_t, at least on my computer
										
										controls.back().setControllerNumber( choice );
										
										if( settingsManager.debug ) {
											std::wcout << L" converts to integer " << choice << std::endl;
										}
									}
									
									{
										choiceStr = boost::algorithm::trim_copy( choiceStr.substr( choiceStr.find( L' ' ) ) );
										std::wstring buttonOrJoystick = choiceStr.substr( 0, choiceStr.find( L' ' ) );
										std::vector< std::wstring > possibleChoices = { L"button", L"joystick" };
										buttonOrJoystick = possibleChoices.at( spellChecker.indexOfClosestString( buttonOrJoystick, possibleChoices ) );
										
										if( buttonOrJoystick == possibleChoices.at( 0 ) ) { //L"button"
											std::wstring controllerButtonStr = boost::algorithm::trim_copy( choiceStr.substr( buttonOrJoystick.length() ) );
											uint_fast8_t choice;
											
											if( settingsManager.debug ) {
												std::wcout << L"controller button number (string) \"" << controllerButtonStr << L"\"" << std::endl;
											}
											
											choice = static_cast< uint_fast8_t >( boost::lexical_cast< unsigned short int >( controllerButtonStr ) ); //Boost lexical cast can't convert directly to uint_fast8_t, at least on my computer
											
											controls.back().setControllerButton( choice );
											
											if( settingsManager.debug ) {
												std::wcout << L" converts to integer " << choice << std::endl;
											}
										} else { //L"joystick"
											std::wstring joystickAxisStr = boost::algorithm::trim_copy( choiceStr.substr( buttonOrJoystick.length(), 2 ) );
											
											if( settingsManager.debug ) {
												std::wcout << L"joystick axis (string) \"" << joystickAxisStr << L"\"" << std::endl;
											}
											
											{
												std::vector< std::wstring > possibleChoices = { L"x", L"y", L"z", L"r", L"u", L"v" };
												std::wstring choice = possibleChoices.at( spellChecker.indexOfClosestString( joystickAxisStr, possibleChoices ) );
												
												if( settingsManager.debug ) {
													std::wcout << L" converts to string " << choice << std::endl;
												}
												
												if( choice == possibleChoices.at( 0 ) ) { //L"x"
													controls.back().setJoystickAxis( ( uint_fast8_t ) irr::SEvent::SJoystickEvent::AXIS_X );
												} else if( choice == possibleChoices.at( 1 ) ) { //L"y"
													controls.back().setJoystickAxis( ( uint_fast8_t ) irr::SEvent::SJoystickEvent::AXIS_Y );
												} else if( choice == possibleChoices.at( 2 ) ) { //L"z"
													controls.back().setJoystickAxis( ( uint_fast8_t ) irr::SEvent::SJoystickEvent::AXIS_Z );
												} else if( choice == possibleChoices.at( 3 ) ) { //L"r"
													controls.back().setJoystickAxis( ( uint_fast8_t ) irr::SEvent::SJoystickEvent::AXIS_R );
												} else if( choice == possibleChoices.at( 4 ) ) { //L"u"
													controls.back().setJoystickAxis( ( uint_fast8_t ) irr::SEvent::SJoystickEvent::AXIS_U );
												} else if( choice == possibleChoices.at( 5 ) ) { //L"v"
													controls.back().setJoystickAxis( ( uint_fast8_t ) irr::SEvent::SJoystickEvent::AXIS_V );
												} /*else if( choice == possibleChoices.at( 6 ) ) { // L"deadzone"
													//joystickDeadZones.at( controls.back().getControllerNumber() );
												}*/
											}
											
											{
												std::wstring increaseOrDecrease = boost::algorithm::trim_copy( choiceStr.substr( 2 + buttonOrJoystick.length() + joystickAxisStr.length() ) );
												std::vector< std::wstring > possibleChoices = { L"increase", L"decrease" };
												std::wstring choice = possibleChoices.at( spellChecker.indexOfClosestString( increaseOrDecrease, possibleChoices ) );
												if( choice == possibleChoices.at( 0 ) ) { //L"increase"
													controls.back().setJoystickDirection( ControlMapping::JOYSTICK_INCREASE );
												} else if( choice == possibleChoices.at( 1 ) ) { //L"decrease"
													controls.back().setJoystickDirection( ControlMapping::JOYSTICK_DECREASE );
												}
											}
										}
									}
									
								} else if( spellChecker.DamerauLevenshteinDistance( choiceStr.substr( 0, possibleChoiceStarts.at( 1 ).length() ), possibleChoiceStarts.at( 1 ) ) <= 1 ) { //L"key"
									choiceStr = boost::algorithm::trim_copy( choiceStr.substr( possibleChoiceStarts.at( 1 ).length(), choiceStr.length() - possibleChoiceStarts.at( 1 ).length() ) ); //possibleChoiceStarts.at( 1 ).length() = length of the word "key"
									irr::EKEY_CODE choice;
									
									choice = static_cast< irr::EKEY_CODE >( boost::lexical_cast< int >( choiceStr ) ); //Boost lexical cast can't convert directly to enumerated types
									
									controls.back().setKey( choice );
									
								} else if( spellChecker.DamerauLevenshteinDistance( choiceStr.substr( 0, possibleChoiceStarts.at( 2 ).length() ), possibleChoiceStarts.at( 2 ) ) <= 1 ) { //L"mouse"
									
									choiceStr = choiceStr.substr( possibleChoiceStarts.at( 2 ).length(), choiceStr.length() - possibleChoiceStarts.at( 2 ).length() ); //possibleChoiceStarts.at( 2 ).length() = length of the word "mouse"
									boost::algorithm::trim( choiceStr ); //Remove trailing and leading spaces
									if( settingsManager.debug ) {
										std::wcout << L"choiceStr: " << choiceStr;
									}
									
									{
										std::wstring moveOrButtonOrWheel = choiceStr.substr( 0, choiceStr.find( L' ' ) );
										if( settingsManager.debug ) {
											std::wcout << L"moveOrButtonOrWheel before spell checking: " << moveOrButtonOrWheel;
										}
										
										std::vector< std::wstring > possibleChoices = { L"wheel", L"leftbutton", L"middlebutton", L"rightbutton", L"move" };
										moveOrButtonOrWheel = possibleChoices.at( spellChecker.indexOfClosestString( moveOrButtonOrWheel, possibleChoices ) );
										
										if( settingsManager.debug ) {
											std::wcout << L"moveOrButtonOrWheel after spell checking: " << moveOrButtonOrWheel;
										}
										
										if( moveOrButtonOrWheel == possibleChoices.at( 0 ) ) { //L"wheel"
											controls.back().setMouseEvent( irr::EMIE_MOUSE_WHEEL );
											
											std::wstring wheelDirection = boost::algorithm::trim_copy( choiceStr.substr( moveOrButtonOrWheel.length() ) );
											if( settingsManager.debug ) {
												std::wcout << L"wheelDirection before spell checking: " << wheelDirection << std::endl;
											}
											std::vector< std::wstring > possibleDirections = { L"up", L"u", L"down", L"d" };
											wheelDirection = possibleDirections.at( spellChecker.indexOfClosestString( wheelDirection, possibleDirections ) );
											if( settingsManager.debug ) {
												std::wcout << L"wheelDirection after spell checking: " << wheelDirection << std::endl;
											}
											
											if( wheelDirection == possibleDirections.at( 0 ) or wheelDirection == possibleDirections.at( 1 ) ) { //up
												controls.back().setMouseWheelUp( true );
											} else {
												controls.back().setMouseWheelUp( false );
											}
										} else if( moveOrButtonOrWheel == possibleChoices.at( 1 ) ) { //L"leftbutton"
											std::wstring upOrDown = boost::algorithm::trim_copy( choiceStr.substr( moveOrButtonOrWheel.length() ) );
											if( settingsManager.debug ) {
												std::wcout << L"upOrDown before spell checking: " << upOrDown << std::endl;
											}
											std::vector< std::wstring > possibleStates = { L"up", L"u", L"down", L"d" };
											upOrDown = possibleStates.at( spellChecker.indexOfClosestString( upOrDown, possibleStates ) );
											if( settingsManager.debug ) {
												std::wcout << L"upOrDown after spell checking: " << upOrDown << std::endl;
											}
											
											if( upOrDown == possibleStates.at( 0 ) or upOrDown == possibleStates.at( 1 ) ) { //L"up"
												controls.back().setMouseEvent( irr::EMIE_LMOUSE_LEFT_UP );
											} else {
												controls.back().setMouseEvent( irr::EMIE_LMOUSE_PRESSED_DOWN );
											}
										} else if( moveOrButtonOrWheel == possibleChoices.at( 2 ) ) { //L"middlebutton"
											std::wstring upOrDown = boost::algorithm::trim_copy( choiceStr.substr( moveOrButtonOrWheel.length() ) );
											if( settingsManager.debug ) {
												std::wcout << L"upOrDown before spell checking: " << upOrDown << std::endl;
											}
											std::vector< std::wstring > possibleStates = { L"up", L"u", L"down", L"d" };
											upOrDown = possibleStates.at( spellChecker.indexOfClosestString( upOrDown, possibleStates ) );
											if( settingsManager.debug ) {
												std::wcout << L"upOrDown after spell checking: " << upOrDown << std::endl;
											}
											
											if( upOrDown == possibleStates.at( 0 ) or upOrDown == possibleStates.at( 1 ) ) { //L"up"
												controls.back().setMouseEvent( irr::EMIE_MMOUSE_LEFT_UP );
											} else {
												controls.back().setMouseEvent( irr::EMIE_MMOUSE_PRESSED_DOWN );
											}
										} else if( moveOrButtonOrWheel == possibleChoices.at( 3 ) ) { //L"rightbutton"
											std::wstring upOrDown = boost::algorithm::trim_copy( choiceStr.substr( moveOrButtonOrWheel.length() ) );
											if( settingsManager.debug ) {
												std::wcout << L"upOrDown before spell checking: " << upOrDown << std::endl;
											}
											std::vector< std::wstring > possibleStates = { L"up", L"u", L"down", L"d" };
											upOrDown = possibleStates.at( spellChecker.indexOfClosestString( upOrDown, possibleStates ) );
											if( settingsManager.debug ) {
												std::wcout << L"upOrDown after spell checking: " << upOrDown << std::endl;
											}
											
											if( upOrDown == possibleStates.at( 0 ) or upOrDown == possibleStates.at( 1 ) ) { //L"up"
												controls.back().setMouseEvent( irr::EMIE_RMOUSE_LEFT_UP );
											} else {
												controls.back().setMouseEvent( irr::EMIE_RMOUSE_PRESSED_DOWN );
											}
										} else if( moveOrButtonOrWheel == possibleChoices.at( 4 ) ) { //L"move"
											controls.back().setMouseEvent( irr::EMIE_MOUSE_MOVED );
											
											std::wstring direction = boost::algorithm::trim_copy( choiceStr.substr( moveOrButtonOrWheel.length() ) );
											
											if( settingsManager.debug ) {
												std::wcout << L"direction before spell checking: " << direction << std::endl;
											}
											std::vector< std::wstring > possibleDirections = { L"up", L"u", L"down", L"d", L"left", L"l", L"right", L"r" };
											direction = possibleDirections.at( spellChecker.indexOfClosestString( direction, possibleDirections ) );
											if( settingsManager.debug ) {
												std::wcout << L"direction after spell checking: " << direction << std::endl;
											}
											
											if( direction == possibleDirections.at( 0 ) or direction == possibleDirections.at( 1 ) ) { //L"up"
												controls.back().setMouseDirection( ControlMapping::MOUSE_UP );
											} else if( direction == possibleDirections.at( 2 ) or direction == possibleDirections.at( 3 ) ) { //L"down"
												controls.back().setMouseDirection( ControlMapping::MOUSE_DOWN );
											} else if( direction == possibleDirections.at( 4 ) or direction == possibleDirections.at( 5 ) ) { //L"left"
												controls.back().setMouseDirection( ControlMapping::MOUSE_LEFT );
											} else { //L"right"
												controls.back().setMouseDirection( ControlMapping::MOUSE_RIGHT );
											}
										}
										
										//TODO: Enable double- and triple-clicks.
										/*if( choiceStr == possibleChoices.at( 9 ) ) { //L"leftdoubleclick"
											controls.back().setMouseEvent( EMIE_LMOUSE_DOUBLE_CLICK );
										} else if( choiceStr == possibleChoices.at( 10 ) ) { //L"rightdoubleclick"
											controls.back().setMouseEvent( EMIE_RMOUSE_DOUBLE_CLICK );
										} else if( choiceStr == possibleChoices.at( 11 ) ) { //L"middledoubleclick"
											controls.back().setMouseEvent( EMIE_MMOUSE_DOUBLE_CLICK );
										} else if( choiceStr == possibleChoices.at( 12 ) ) { //L"lefttripleclick"
											controls.back().setMouseEvent( EMIE_LMOUSE_TRIPLE_CLICK );
										} else if( choiceStr == possibleChoices.at( 13 ) ) { //L"righttripleclick"
											controls.back().setMouseEvent( EMIE_RMOUSE_TRIPLE_CLICK );
										} else if( choiceStr == possibleChoices.at( 14 ) ) { //L"middletripleclick"
											controls.back().setMouseEvent( EMIE_MMOUSE_TRIPLE_CLICK );
										}*/
									}
								}
								
								if( preference.substr( 0, 6 ) == L"player" ) {
									try {
										preference = preference.substr( 7 );
										std::wstring playerNumStr = boost::algorithm::trim_copy( preference.substr( 0, preference.find( L' ' ) ) );
										std::wstring actionStr = boost::algorithm::trim_copy( preference.substr( preference.find( L' ' ) ) );
										
										decltype( settingsManager.getNumPlayers() ) playerNum;
										if( spellChecker.DamerauLevenshteinDistance( playerNumStr, L"mine" ) <= 1 ) {
											playerNum = myPlayer;
										} else {
											playerNum = boost::lexical_cast< unsigned short int >( playerNumStr ); //Boost doesn't like casting to uint_fast8_t
										}
										
										if( playerNum < settingsManager.getNumPlayers() or playerNum == myPlayer ) {
											std::vector< std::wstring > possibleActions = { L"up", L"u", L"down", L"d", L"left", L"l", L"right", L"r" };
											actionStr = possibleActions.at( spellChecker.indexOfClosestString( actionStr, possibleActions) );
											controls.back().setPlayer( playerNum );
											
											if( actionStr == possibleActions.at( 0 )  or actionStr == possibleActions.at( 1 ) ) { //L"up"
												controls.back().setAction( ControlMapping::ACTION_PLAYER_UP );
											} else if( actionStr == possibleActions.at( 2 ) or actionStr == possibleActions.at( 3 ) ) { //L"down"
												controls.back().setAction( ControlMapping::ACTION_PLAYER_DOWN );
											} else if( actionStr == possibleActions.at( 4 ) or actionStr == possibleActions.at( 5 ) ) { //L"left"
												controls.back().setAction( ControlMapping::ACTION_PLAYER_LEFT );
											} else { //L"right"
												controls.back().setAction( ControlMapping::ACTION_PLAYER_RIGHT );
											}
											
										} else { //We ignore that player number because we don't have that many players
											controls.pop_back();
										}
									} catch ( boost::bad_lexical_cast &e ) {
										std::wcerr << L"Error reading player number (is it not a number?) on line " << lineNum << L": " << e.what() << std::endl;
									}
								} else if( preference.substr( 0, 4 ) == L"menu" ) {
									std::wstring menuWhat = boost::algorithm::trim_copy( preference.substr( 4 ) );
									std::vector< std::wstring > possibleMenuStuff = { L"up", L"u", L"down", L"d", L"activate" };
									menuWhat = possibleMenuStuff.at( spellChecker.indexOfClosestString( menuWhat, possibleMenuStuff ) );
									if( settingsManager.debug ) {
										std::wcout << L"Menu action: " << menuWhat << std::endl;
									}
									if( menuWhat == possibleMenuStuff.at( 0 ) or menuWhat == possibleMenuStuff.at( 1 ) ) { //L"up"
										if( settingsManager.debug ) {
											std::wcout << L"Setting control to menu up" << std::endl;
										}
										controls.back().setAction( ControlMapping::ACTION_MENU_UP );
									} else if( menuWhat == possibleMenuStuff.at( 2 ) or menuWhat == possibleMenuStuff.at( 3 ) ) { //L"down"
										if( settingsManager.debug ) {
											std::wcout << L"Setting control to menu down" << std::endl;
										}
										controls.back().setAction( ControlMapping::ACTION_MENU_DOWN );
									} else { //L"activate"
										if( settingsManager.debug ) {
											std::wcout << L"Setting control to menu activate" << std::endl;
										}
										controls.back().setAction( ControlMapping::ACTION_MENU_ACTIVATE );
									}
								} else if( preference.substr( 0, 6 ) == L"volume" ) {
									std::wstring volumeWhat = boost::algorithm::trim_copy( preference.substr( 6 ) );
									std::vector< std::wstring > possibleVolumes = { L"up", L"u", L"down", L"d" };
									volumeWhat = possibleVolumes.at( spellChecker.indexOfClosestString( volumeWhat, possibleVolumes ) );
									if( volumeWhat == possibleVolumes.at( 0 ) or volumeWhat == possibleVolumes.at( 1 ) ) { //L"up"
										controls.back().setAction( ControlMapping::ACTION_VOLUME_UP );
									} else { //L"down"
										controls.back().setAction( ControlMapping::ACTION_VOLUME_DOWN );
									}
								} /*else if( preference.substr( 0, 1 ) == L"j" ) {
									std::wcout << preference.length() << std::endl;
									try {
										if( preference.substr( 0, 18 ) == L"joystick dead zone" ) {
											std::wstring deadZoneString = boost::algorithm::trim_copy( preference.substr( 18 ) );
											std::wcout << L"deadZoneString: \"" << deadZoneString << L"\"" << std::endl;
										}
									} catch( std::exception &e ) {
										//do nothing;
									}
									
								}*/ else {
									if( settingsManager.debug ) {
										std::wcout << L"preference before spell checking: " << preference;
									}
									
									std::vector< std::wstring > possiblePrefs = { L"screenshot", L"enable controller" };
									preference = possiblePrefs.at( spellChecker.indexOfClosestString( preference, possiblePrefs ) );
									
									if( settingsManager.debug ) {
										std::wcout  << "\tand after: " << preference << std::endl;
									}
									
									if( preference == possiblePrefs.at( 0 ) ) {
										controls.back().setAction( ControlMapping::ACTION_SCREENSHOT );
									} else if( preference == possiblePrefs.at( 1 ) ) { //L"enable controller"
										std::vector< std::wstring > possibleChoices = { L"true", L"false" };
										choiceStr = possibleChoices.at( spellChecker.indexOfClosestString( choiceStr, possibleChoices ) );
										
										if( choiceStr == possibleChoices.at( 0 ) ) {
											if( settingsManager.debug ) {
												std::wcout << L"controller is ENABLED" << std::endl;
											}
											enableController = true;
										} else {
											if( settingsManager.debug ) {
												std::wcout << L"controller is DISABLED" << std::endl;
											}
											enableController = false;
										}
									}
								}
								
							} catch( std::exception &e ) {
								std::wcerr << L"Error in MainGame::setControls(): " << e.what() << std::endl;
							}
						}
					}
					
					controlsFile.close();
				}
			}
		}
		
		if( not controlsFileFound ) {
			throw( CustomException( std::wstring( L"controls.cfg does not exist or is not readable in any of the folders that were searched." ) ) );
		}
	} catch( CustomException &e ) {
		std::wcerr << L"Error in MainGame::setControls(): " << e.what() << std::endl;
	} catch( std::exception &e ) {
		std::wcerr << L"Error in MainGame::setControls(): " << e.what() << std::endl;
	}
	
	if( settingsManager.debug ) {
		std::wcout << L"end of setControls()" << std::endl;
	}
	
	//Eliminate duplicate controls
	for( decltype( controls.size() ) a = 0; a < controls.size(); ++a ) {
		for( decltype( controls.size() ) b = a + 1; b < controls.size(); ++b ) {
			if( controls.at( a ) == controls.at( b ) ) {
				controls.erase( controls.begin() + b );
			}
		}
	}
	
	
	if( enableController and device->activateJoysticks( controllerInfo ) and settingsManager.debug ) { //activateJoysticks fills controllerInfo with info about each controller
		std::wcout << L"controller support is enabled and " << controllerInfo.size() << L" controller(s) are present." << std::endl;
		
		for( decltype( controllerInfo.size() ) controller = 0; controller < controllerInfo.size(); ++controller ) {
			std::wcout << L"controller " << controller << L":" << std::endl;
			std::wcout << L"\tName: '" << stringConverter.toStdWString( controllerInfo[ controller ].Name ).c_str() << L"'" << std::endl; //stringConverter.toWCharArray( controllerInfo[ controller ].Name ) << L"'" << std::endl;
			std::wcout << L"\tAxes: " << controllerInfo[ controller ].Axes << std::endl;
			std::wcout << L"\tButtons: " << controllerInfo[ controller ].Buttons << std::endl;
			
			std::wcout << L"\tHat is: ";
			
			switch( controllerInfo[controller ].PovHat ) {
				case irr::SJoystickInfo::POV_HAT_PRESENT:
					std::wcout << L"present" << std::endl;
					break;
					
				case irr::SJoystickInfo::POV_HAT_ABSENT:
					std::wcout << L"absent" << std::endl;
					break;
					
				case irr::SJoystickInfo::POV_HAT_UNKNOWN:
				default:
					std::wcout << L"unknown" << std::endl;
					break;
			}
		}
	} else if( settingsManager.debug ) {
		std::wcout << L"controller support is not enabled." << std::endl;
	}
}

/**
 * Randomly picks a background animation and does whatever it needs to do to set it up.
 */
void MainGame::setupBackground() {
	try {
		if( settingsManager.debug ) {
			std::wcout << L"setupBackground() called" << std::endl;
		}
		
		if( settingsManager.debug ) {
			//backgroundChosen = IMAGES;
			//backgroundChosen = NUMBER_OF_BACKGROUNDS - 1; //If we're debugging, we may be testing the last background added.
			if( settingsManager.backgroundAnimations ) {
				backgroundChosen = getRandomNumber() % NUMBER_OF_BACKGROUNDS;
			} else {
				switch( getRandomNumber() % 2 ) {
					case 0: {
						backgroundChosen = IMAGES;
						break;
					}
					case 1: {
						backgroundChosen = PLAIN_COLOR;
						break;
					}
				}
			}
		} else {
			if( settingsManager.backgroundAnimations ) {
				backgroundChosen = getRandomNumber() % NUMBER_OF_BACKGROUNDS;
			} else {
				switch( getRandomNumber() % 2 ) {
					case 0: {
						backgroundChosen = IMAGES;
						break;
					}
					case 1: {
						backgroundChosen = PLAIN_COLOR;
						break;
					}
				}
			}
		}
		
		backgroundTexture = nullptr;
		fillBackgroundTextureAfterLoading = false; //Most backgrounds don't need this to be true
		
		switch( backgroundChosen ) {
			case ORIGINAL_STARFIELD: { //Original starfield: just flies straight forward.
				backgroundColor = BLACK;
				
				// create a particle system
				irr::scene::ICameraSceneNode* camera = backgroundSceneManager->addCameraSceneNode();
				camera->setPosition( irr::core::vector3df( 0, 0, -150 ) );
				irr::scene::IParticleSystemSceneNode* ps = backgroundSceneManager->addParticleSystemSceneNode( false );

				irr::video::SColor darkStarColor;
				irr::video::SColor lightStarColor;

				switch( getRandomNumber() % 8 ) { //Not a magic number: count the cases
					case 0: {
						switch( settingsManager.colorMode ) {
							case SettingsManager::COLOR_MODE_DO_NOT_USE:
							case SettingsManager::FULLCOLOR: {
								darkStarColor = BLACK;
								lightStarColor = WHITE;
								break;
							}
							case SettingsManager::GRAYSCALE: {
								darkStarColor = BLACK_GRAYSCALE;
								lightStarColor = WHITE_GRAYSCALE;
								break;
							}
							case SettingsManager::GREENSCALE: {
								darkStarColor = BLACK_GREENSCALE;
								lightStarColor = WHITE_GREENSCALE;
								break;
							}
							case SettingsManager::AMBERSCALE: {
								darkStarColor = BLACK_AMBERSCALE;
								lightStarColor = WHITE_AMBERSCALE;
								break;
							}
						}
						
						break;
					}
					case 1: {
						switch( settingsManager.colorMode ) {
							case SettingsManager::COLOR_MODE_DO_NOT_USE:
							case SettingsManager::FULLCOLOR: {
								darkStarColor = BLUE;
								lightStarColor = LIGHTBLUE;
								break;
							}
							case SettingsManager::GRAYSCALE: {
								darkStarColor = BLUE_GRAYSCALE;
								lightStarColor = LIGHTBLUE_GRAYSCALE;
								break;
							}
							case SettingsManager::GREENSCALE: {
								darkStarColor = BLUE_GREENSCALE;
								lightStarColor = LIGHTBLUE_GREENSCALE;
								break;
							}
							case SettingsManager::AMBERSCALE: {
								darkStarColor = BLUE_AMBERSCALE;
								lightStarColor = LIGHTBLUE_AMBERSCALE;
								break;
							}
						}
						break;
					}
					case 2: {
						switch( settingsManager.colorMode ) {
							case SettingsManager::COLOR_MODE_DO_NOT_USE:
							case SettingsManager::FULLCOLOR: {
								darkStarColor = GREEN;
								lightStarColor = LIGHTGREEN;
								break;
							}
							case SettingsManager::GRAYSCALE: {
								darkStarColor = GREEN_GRAYSCALE;
								lightStarColor = LIGHTGREEN_GRAYSCALE;
								break;
							}
							case SettingsManager::GREENSCALE: {
								darkStarColor = GREEN_GREENSCALE;
								lightStarColor = LIGHTGREEN_GREENSCALE;
								break;
							}
							case SettingsManager::AMBERSCALE: {
								darkStarColor = GREEN_AMBERSCALE;
								lightStarColor = LIGHTGREEN_AMBERSCALE;
								break;
							}
						}
						break;
					}
					case 3: {
						switch( settingsManager.colorMode ) {
							case SettingsManager::COLOR_MODE_DO_NOT_USE:
							case SettingsManager::FULLCOLOR: {
								darkStarColor = CYAN;
								lightStarColor = LIGHTCYAN;
								break;
							}
							case SettingsManager::GRAYSCALE: {
								darkStarColor = CYAN_GRAYSCALE;
								lightStarColor = LIGHTCYAN_GRAYSCALE;
								break;
							}
							case SettingsManager::GREENSCALE: {
								darkStarColor = CYAN_GREENSCALE;
								lightStarColor = LIGHTCYAN_GREENSCALE;
								break;
							}
							case SettingsManager::AMBERSCALE: {
								darkStarColor = CYAN_AMBERSCALE;
								lightStarColor = LIGHTCYAN_AMBERSCALE;
								break;
							}
						}
						break;
					}
					case 4: {
						switch( settingsManager.colorMode ) {
							case SettingsManager::COLOR_MODE_DO_NOT_USE:
							case SettingsManager::FULLCOLOR: {
								darkStarColor = RED;
								lightStarColor = LIGHTRED;
								break;
							}
							case SettingsManager::GRAYSCALE: {
								darkStarColor = RED_GRAYSCALE;
								lightStarColor = LIGHTRED_GRAYSCALE;
								break;
							}
							case SettingsManager::GREENSCALE: {
								darkStarColor = RED_GREENSCALE;
								lightStarColor = LIGHTRED_GREENSCALE;
								break;
							}
							case SettingsManager::AMBERSCALE: {
								darkStarColor = RED_AMBERSCALE;
								lightStarColor = LIGHTRED_AMBERSCALE;
								break;
							}
						}
						break;
					}
					case 5: {
						switch( settingsManager.colorMode ) {
							case SettingsManager::COLOR_MODE_DO_NOT_USE:
							case SettingsManager::FULLCOLOR: {
								darkStarColor = MAGENTA;
								lightStarColor = LIGHTMAGENTA;
								break;
							}
							case SettingsManager::GRAYSCALE: {
								darkStarColor = MAGENTA_GRAYSCALE;
								lightStarColor = LIGHTMAGENTA_GRAYSCALE;
								break;
							}
							case SettingsManager::GREENSCALE: {
								darkStarColor = MAGENTA_GREENSCALE;
								lightStarColor = LIGHTMAGENTA_GREENSCALE;
								break;
							}
							case SettingsManager::AMBERSCALE: {
								darkStarColor = MAGENTA_AMBERSCALE;
								lightStarColor = LIGHTMAGENTA_AMBERSCALE;
								break;
							}
						}
						break;
					}
					case 6: {
						switch( settingsManager.colorMode ) {
							case SettingsManager::COLOR_MODE_DO_NOT_USE:
							case SettingsManager::FULLCOLOR: {
								darkStarColor = GRAY;
								lightStarColor = LIGHTGRAY;
								break;
							}
							case SettingsManager::GRAYSCALE: {
								darkStarColor = GRAY_GRAYSCALE;
								lightStarColor = LIGHTGRAY_GRAYSCALE;
								break;
							}
							case SettingsManager::GREENSCALE: {
								darkStarColor = GRAY_GREENSCALE;
								lightStarColor = LIGHTGRAY_GREENSCALE;
								break;
							}
							case SettingsManager::AMBERSCALE: {
								darkStarColor = GRAY_AMBERSCALE;
								lightStarColor = LIGHTGRAY_AMBERSCALE;
								break;
							}
						}
						break;
					}
					case 7: {
						switch( settingsManager.colorMode ) {
							case SettingsManager::COLOR_MODE_DO_NOT_USE:
							case SettingsManager::FULLCOLOR: {
								darkStarColor = YELLOW;
								lightStarColor = BROWN;
								break;
							}
							case SettingsManager::GRAYSCALE: {
								darkStarColor = YELLOW_GRAYSCALE;
								lightStarColor = BROWN_GRAYSCALE;
								break;
							}
							case SettingsManager::GREENSCALE: {
								darkStarColor = YELLOW_GREENSCALE;
								lightStarColor = BROWN_GREENSCALE;
								break;
							}
							case SettingsManager::AMBERSCALE: {
								darkStarColor = YELLOW_AMBERSCALE;
								lightStarColor = BROWN_AMBERSCALE;
								break;
							}
						}
						break;
					}
				}

				irr::scene::IParticleEmitter* em = ps->createBoxEmitter(
												  camera->getViewFrustum()->getBoundingBox(), //core::aabbox3d< float >(-7,-7,-7,7,7,7), // emitter size
												  irr::core::vector3df( 0.0f, 0.0f, -0.1f ), // initial direction
												  100, 500,							// Min & max emit rate
												  darkStarColor,	   // darkest color
												  lightStarColor,	   // brightest color
												  2000, 20000, 0,					   // min and max age, angle
												  irr::core::dimension2df( 1.f, 1.f ),	  // min size
												  irr::core::dimension2df( 20.f, 20.f ) );	// max size

				ps->setEmitter( em ); // this grabs the emitter
				em->drop(); // so we can drop it here without deleting it
				
				ps->setPosition( irr::core::vector3df( 0, 0, 40 ) );
				ps->setScale( irr::core::vector3df( 1, 1, 1 ) );
				ps->setMaterialFlag( irr::video::EMF_LIGHTING, false );
				//ps->setMaterialFlag( irr::video::EMF_ZWRITE_ENABLE, false ); //Don't remember why I put this here; it doesn't seem to make any difference
				//ps->setMaterialTexture( 0, driver->getTexture( "star.png" ) );
				ps->setMaterialType( irr::video::EMT_TRANSPARENT_ALPHA_CHANNEL );
				
				irr::video::IImage* pixelImage = driver->createImage( irr::video::ECF_A1R5G5B5, irr::core::dimension2d< irr::u32 >( 1, 1 ) );
				
				{
					irr::video::SColor color;
					switch( settingsManager.colorMode ) {
						case SettingsManager::COLOR_MODE_DO_NOT_USE:
						case SettingsManager::FULLCOLOR: {
							color = WHITE;
							break;
						}
						case SettingsManager::GRAYSCALE: {
							color = WHITE_GRAYSCALE;
							break;
						}
						case SettingsManager::GREENSCALE: {
							color = WHITE_GREENSCALE;
							break;
						}
						case SettingsManager::AMBERSCALE: {
							color = WHITE_AMBERSCALE;
							break;
						}
					}
					pixelImage->setPixel( 0, 0, color, false ); //Which is faster on a 1x1 pixel image: setPixel() or fill()?
				}
				
				irr::video::ITexture* pixelTexture = driver->addTexture( "pixel", pixelImage );
				ps->setMaterialTexture( 0, pixelTexture );
				break;
			}
			case STAR_TRAILS: { //"STARTRAILS" background (deliberately falls through to starfield, do not add a break). This smearing background started out as a bug, but I liked the look so much I decided to replicate it as a feature.
				if( driver->queryFeature( irr::video::EVDF_RENDER_TO_TARGET  ) ) {
					fillBackgroundTextureAfterLoading = true;
					haveFilledBackgroundTextureAfterLoading = false;
					backgroundTexture = driver->addRenderTargetTexture( screenSize );
					
					{ //Fill the texture with the background color;
						driver->beginScene( false, false, backgroundColor );
						driver->setRenderTarget( backgroundTexture, true, true, backgroundColor );
						driver->setRenderTarget( irr::video::ERT_FRAME_BUFFER, false, false, backgroundColor );
						driver->endScene();
					}
					
				} else {
					backgroundChosen = ROTATING_STARFIELD;
				}
			}
			case ROTATING_STARFIELD: { //New starfield: rotates the camera around.
				// create a particle system
				irr::scene::ICameraSceneNode* camera = backgroundSceneManager->addCameraSceneNode();
				
				//Decide which direction to rotate
				float x, y, z;
				float magnitude = 0.02;
				switch( getRandomNumber() % 3 ) {
					case 0: {
						x = -magnitude;
						break;
					}
					case 1: {
						x = 0;
						break;
					}
					case 2: {
						x = magnitude;
						break;
					}
				}
				switch( getRandomNumber() % 3 ) {
					case 0: {
						y = -magnitude;
						break;
					}
					case 1: {
						y = 0;
						break;
					}
					case 2: {
						y = magnitude;
						break;
					}
				}
				switch( getRandomNumber() % 3 ) {
					case 0: {
						z = -magnitude;
						break;
					}
					case 1: {
						z = 0;
						break;
					}
					case 2: {
						z = magnitude;
						break;
					}
				}
				
				//Create rotation animator and bind it to the camera
				irr::scene::ISceneNodeAnimator* rotator = backgroundSceneManager->createRotationAnimator( irr::core::vector3df( x, y, z ) );
				if( rotator ) {
					camera->bindTargetAndRotation( true );
					camera->addAnimator( rotator );
					if( settingsManager.debug ) {
						std::wcout << "Camera rotator added" << std::endl;
					}
					rotator->drop();
				}
				
				camera->setPosition( irr::core::vector3df( 0, 0, -150 ) );
				irr::scene::IParticleSystemSceneNode* ps = backgroundSceneManager->addParticleSystemSceneNode( false ); //False means don't use the default particle emitter; we're using our own.
				
				irr::video::SColor darkStarColor;
				irr::video::SColor lightStarColor;
				
				switch( getRandomNumber() % 8 ) { //Not a magic number: count the cases
					case 0: {
						switch( settingsManager.colorMode ) {
							case SettingsManager::COLOR_MODE_DO_NOT_USE:
							case SettingsManager::FULLCOLOR: {
								darkStarColor = BLACK;
								lightStarColor = WHITE;
								break;
							}
							case SettingsManager::GRAYSCALE: {
								darkStarColor = BLACK_GRAYSCALE;
								lightStarColor = WHITE_GRAYSCALE;
								break;
							}
							case SettingsManager::GREENSCALE: {
								darkStarColor = BLACK_GREENSCALE;
								lightStarColor = WHITE_GREENSCALE;
								break;
							}
							case SettingsManager::AMBERSCALE: {
								darkStarColor = BLACK_AMBERSCALE;
								lightStarColor = WHITE_AMBERSCALE;
								break;
							}
						}
						
						break;
					}
					case 1: {
						switch( settingsManager.colorMode ) {
							case SettingsManager::COLOR_MODE_DO_NOT_USE:
							case SettingsManager::FULLCOLOR: {
								darkStarColor = BLUE;
								lightStarColor = LIGHTBLUE;
								break;
							}
							case SettingsManager::GRAYSCALE: {
								darkStarColor = BLUE_GRAYSCALE;
								lightStarColor = LIGHTBLUE_GRAYSCALE;
								break;
							}
							case SettingsManager::GREENSCALE: {
								darkStarColor = BLUE_GREENSCALE;
								lightStarColor = LIGHTBLUE_GREENSCALE;
								break;
							}
							case SettingsManager::AMBERSCALE: {
								darkStarColor = BLUE_AMBERSCALE;
								lightStarColor = LIGHTBLUE_AMBERSCALE;
								break;
							}
						}
						break;
					}
					case 2: {
						switch( settingsManager.colorMode ) {
							case SettingsManager::COLOR_MODE_DO_NOT_USE:
							case SettingsManager::FULLCOLOR: {
								darkStarColor = GREEN;
								lightStarColor = LIGHTGREEN;
								break;
							}
							case SettingsManager::GRAYSCALE: {
								darkStarColor = GREEN_GRAYSCALE;
								lightStarColor = LIGHTGREEN_GRAYSCALE;
								break;
							}
							case SettingsManager::GREENSCALE: {
								darkStarColor = GREEN_GREENSCALE;
								lightStarColor = LIGHTGREEN_GREENSCALE;
								break;
							}
							case SettingsManager::AMBERSCALE: {
								darkStarColor = GREEN_AMBERSCALE;
								lightStarColor = LIGHTGREEN_AMBERSCALE;
								break;
							}
						}
						break;
					}
					case 3: {
						switch( settingsManager.colorMode ) {
							case SettingsManager::COLOR_MODE_DO_NOT_USE:
							case SettingsManager::FULLCOLOR: {
								darkStarColor = CYAN;
								lightStarColor = LIGHTCYAN;
								break;
							}
							case SettingsManager::GRAYSCALE: {
								darkStarColor = CYAN_GRAYSCALE;
								lightStarColor = LIGHTCYAN_GRAYSCALE;
								break;
							}
							case SettingsManager::GREENSCALE: {
								darkStarColor = CYAN_GREENSCALE;
								lightStarColor = LIGHTCYAN_GREENSCALE;
								break;
							}
							case SettingsManager::AMBERSCALE: {
								darkStarColor = CYAN_AMBERSCALE;
								lightStarColor = LIGHTCYAN_AMBERSCALE;
								break;
							}
						}
						break;
					}
					case 4: {
						switch( settingsManager.colorMode ) {
							case SettingsManager::COLOR_MODE_DO_NOT_USE:
							case SettingsManager::FULLCOLOR: {
								darkStarColor = RED;
								lightStarColor = LIGHTRED;
								break;
							}
							case SettingsManager::GRAYSCALE: {
								darkStarColor = RED_GRAYSCALE;
								lightStarColor = LIGHTRED_GRAYSCALE;
								break;
							}
							case SettingsManager::GREENSCALE: {
								darkStarColor = RED_GREENSCALE;
								lightStarColor = LIGHTRED_GREENSCALE;
								break;
							}
							case SettingsManager::AMBERSCALE: {
								darkStarColor = RED_AMBERSCALE;
								lightStarColor = LIGHTRED_AMBERSCALE;
								break;
							}
						}
						break;
					}
					case 5: {
						switch( settingsManager.colorMode ) {
							case SettingsManager::COLOR_MODE_DO_NOT_USE:
							case SettingsManager::FULLCOLOR: {
								darkStarColor = MAGENTA;
								lightStarColor = LIGHTMAGENTA;
								break;
							}
							case SettingsManager::GRAYSCALE: {
								darkStarColor = MAGENTA_GRAYSCALE;
								lightStarColor = LIGHTMAGENTA_GRAYSCALE;
								break;
							}
							case SettingsManager::GREENSCALE: {
								darkStarColor = MAGENTA_GREENSCALE;
								lightStarColor = LIGHTMAGENTA_GREENSCALE;
								break;
							}
							case SettingsManager::AMBERSCALE: {
								darkStarColor = MAGENTA_AMBERSCALE;
								lightStarColor = LIGHTMAGENTA_AMBERSCALE;
								break;
							}
						}
						break;
					}
					case 6: {
						switch( settingsManager.colorMode ) {
							case SettingsManager::COLOR_MODE_DO_NOT_USE:
							case SettingsManager::FULLCOLOR: {
								darkStarColor = GRAY;
								lightStarColor = LIGHTGRAY;
								break;
							}
							case SettingsManager::GRAYSCALE: {
								darkStarColor = GRAY_GRAYSCALE;
								lightStarColor = LIGHTGRAY_GRAYSCALE;
								break;
							}
							case SettingsManager::GREENSCALE: {
								darkStarColor = GRAY_GREENSCALE;
								lightStarColor = LIGHTGRAY_GREENSCALE;
								break;
							}
							case SettingsManager::AMBERSCALE: {
								darkStarColor = GRAY_AMBERSCALE;
								lightStarColor = LIGHTGRAY_AMBERSCALE;
								break;
							}
						}
						break;
					}
					case 7: {
						switch( settingsManager.colorMode ) {
							case SettingsManager::COLOR_MODE_DO_NOT_USE:
							case SettingsManager::FULLCOLOR: {
								darkStarColor = YELLOW;
								lightStarColor = BROWN;
								break;
							}
							case SettingsManager::GRAYSCALE: {
								darkStarColor = YELLOW_GRAYSCALE;
								lightStarColor = BROWN_GRAYSCALE;
								break;
							}
							case SettingsManager::GREENSCALE: {
								darkStarColor = YELLOW_GREENSCALE;
								lightStarColor = BROWN_GREENSCALE;
								break;
							}
							case SettingsManager::AMBERSCALE: {
								darkStarColor = YELLOW_AMBERSCALE;
								lightStarColor = BROWN_AMBERSCALE;
								break;
							}
						}
						break;
					}
				}
				
				if( backgroundChosen == ROTATING_STARFIELD ) {
					backgroundColor = BLACK;
				} else { //backgroundChosen == STAR_TRAILS;
					backgroundColor = darkStarColor;
				}
				
				irr::scene::IParticleEmitter* em = ps->createBoxEmitter(
												  camera->getViewFrustum()->getBoundingBox(), //core::aabbox3d< float >(-7,-7,-7,7,7,7), // emitter size
												  irr::core::vector3df( 0.0f, 0.0f, -0.1f ), // initial direction
												  100, 500,							// Min & max emit rate
												  darkStarColor,	   // darkest color
												  lightStarColor,	   // brightest color
												  4000, 40000, 0,					   // min and max age, angle
												  irr::core::dimension2df( 1.f, 1.f ),	  // min size
												  irr::core::dimension2df( 20.f, 20.f ) );	// max size
				
				ps->setEmitter( em ); // this grabs the emitter
				em->drop(); // so we can drop it here without deleting it
				
				ps->setPosition( irr::core::vector3df( 0, 0, 40 ) );
				ps->setScale( irr::core::vector3df( 1, 1, 1 ) );
				ps->setMaterialFlag( irr::video::EMF_LIGHTING, false );
				//ps->setMaterialFlag( irr::video::EMF_ZWRITE_ENABLE, false ); //Don't remember why I put this here; it doesn't seem to make any difference
				//ps->setMaterialTexture( 0, driver->getTexture( "star.png" ) );
				ps->setMaterialType( irr::video::EMT_TRANSPARENT_ALPHA_CHANNEL );
				
				irr::video::IImage* pixelImage = driver->createImage( irr::video::ECF_A8R8G8B8, irr::core::dimension2d< irr::u32 >( 1, 1 ) );
				//pixelImage->fill( WHITE );
				pixelImage->setPixel( 0, 0, WHITE, false ); //Which is faster on a 1x1 pixel image: setPixel() or fill()?
				irr::video::ITexture* pixelTexture = driver->addTexture( "pixel", pixelImage );
				ps->setMaterialTexture( 0, pixelTexture );
				break;
			}
			case PLAIN_COLOR: {
				switch( getRandomNumber() % 3 ) { //Black, blue, and gray are the only CGA colors that don't make your eyes bleed when they fill the screen
					case 0: {
						
						switch( settingsManager.colorMode ) {
							case SettingsManager::COLOR_MODE_DO_NOT_USE:
							case SettingsManager::FULLCOLOR: {
								backgroundColor = BLACK;
								break;
							}
							case SettingsManager::GRAYSCALE: {
								backgroundColor = BLACK_GRAYSCALE;
								break;
							}
							case SettingsManager::GREENSCALE: {
								backgroundColor = BLACK_GREENSCALE;
								break;
							}
							case SettingsManager::AMBERSCALE: {
								backgroundColor = BLACK_AMBERSCALE;
								break;
							}
						}
						
						break;
					}
					case 1: {
						
						switch( settingsManager.colorMode ) {
							case SettingsManager::COLOR_MODE_DO_NOT_USE:
							case SettingsManager::FULLCOLOR: {
								backgroundColor = BLUE;
								break;
							}
							case SettingsManager::GRAYSCALE: {
								backgroundColor = BLUE_GRAYSCALE;
								break;
							}
							case SettingsManager::GREENSCALE: {
								backgroundColor = BLUE_GREENSCALE;
								break;
							}
							case SettingsManager::AMBERSCALE: {
								backgroundColor = BLUE_AMBERSCALE;
								break;
							}
						}
						
						break;
					}
					case 2: {
						
						switch( settingsManager.colorMode ) {
							case SettingsManager::COLOR_MODE_DO_NOT_USE:
							case SettingsManager::FULLCOLOR: {
								backgroundColor = GRAY;
								break;
							}
							case SettingsManager::GRAYSCALE: {
								backgroundColor = GRAY_GRAYSCALE;
								break;
							}
							case SettingsManager::GREENSCALE: {
								backgroundColor = GRAY_GREENSCALE;
								break;
							}
							case SettingsManager::AMBERSCALE: {
								backgroundColor = GRAY_AMBERSCALE;
								break;
							}
						}
						
						break;
					}
				}
				break;
			}
			case IMAGES: { //Image files
				std::vector< boost::filesystem::path > backgroundList;
				
				auto folderList = system.getImageFolders();
				//boost::filesystem::path backgroundPath( boost::filesystem::current_path()/L"Images/backgrounds" );
				//folderList.push_back( boost::filesystem::path( boost::filesystem::current_path()/L"Images/backgrounds" ) );
				
				//If backgroundPath (defined inside the for loop inside the following for loop) has a backgrounds subfolder, we want to find backgrounds there first.
				bool useBackgroundsSubfolder = true;
				for( uint_fast8_t repeat = 0; repeat < 2; ++repeat ) {
					for( auto folderListIterator = folderList.begin(); folderListIterator != folderList.end(); ++folderListIterator ) {
						auto backgroundPath = *folderListIterator;
						
						if( useBackgroundsSubfolder ) {
							backgroundPath = backgroundPath/L"backgrounds";
						}
						
						//Which is better: system_complete() or absolute()? On my computer they seem to do the same thing. Both are part of Boost Filesystem.
						backgroundPath = system_complete( backgroundPath );
						//backgroundPath = absolute( backgroundPath );

						if( settingsManager.debug ) {
							std::wcout << L"background path is absolute? " << backgroundPath.is_absolute() << std::endl;
						}

						/*while( ( not exists( backgroundPath ) or not is_directory( backgroundPath ) ) and backgroundPath.has_parent_path() and not useBackgroundsSubfolder ) {
							if( settingsManager.debug ) {
								std::wcout << L"Path " << backgroundPath.wstring() << L" does not exist or is not a directory. Checking parent path " << backgroundPath.parent_path().wstring() << std::endl;
							}

							backgroundPath = backgroundPath.parent_path();
						}*/

						if( backgroundList.empty() and exists( backgroundPath ) ) {
							boost::filesystem::recursive_directory_iterator end;
							
							for( boost::filesystem::recursive_directory_iterator i( backgroundPath ); i not_eq end; ++i ) {
								if( not is_directory( i->path() ) ) { //We've found a file
									if( settingsManager.debug ) {
										std::wcout << i->path().wstring() << std::endl;
									}

									//Asks Irrlicht if the file is loadable. This way the game is certain to accept any file formats the library can use.
									for( decltype( driver->getImageLoaderCount() ) loaderNum = 0; loaderNum < driver->getImageLoaderCount(); ++loaderNum ) { //Irrlicht uses a different image loader for each file type. Loop through them all, ask each if it can load the file.

										irr::video::IImageLoader* loader = driver->getImageLoader( loaderNum );
										irr::io::IFileSystem* fileSystem = device->getFileSystem();
										irr::io::path filePath = stringConverter.toIrrlichtStringW( i->path().wstring() );

										if( loader->isALoadableFileExtension( filePath ) ) { //Comment this out because extensions don't always reflect the file's contents. Uncomment it for a minor speed improvement since not all files would need to be opened.
											irr::io::IReadFile* file = fileSystem->createAndOpenFile( filePath );
											
											if( not isNull( file ) ) {
												if( loader->isALoadableFileFormat( file ) ) {
													backgroundList.push_back( i->path() );
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
					
					useBackgroundsSubfolder = backgroundList.empty();
				}
				
				if( not backgroundList.empty() ) {
					std::vector< boost::filesystem::path >::iterator newEnd = std::unique( backgroundList.begin(), backgroundList.end() ); //unique "removes all but the first element from every consecutive group of equivalent elements in the range [first,last)." (source: http://www.cplusplus.com/reference/algorithm/unique/ )
					backgroundList.resize( std::distance( backgroundList.begin(), newEnd ) );

					//Pick a random background and load it
					backgroundFilePath = stringConverter.toIrrlichtStringW( backgroundList.at( getRandomNumber() % backgroundList.size() ).wstring() );
					backgroundTexture = driver->getTexture( backgroundFilePath );
					if( backgroundTexture == nullptr or backgroundTexture == NULL ) {
						throw( CustomException( L"Could not load background texture" ) );
					} else {
						ImageModifier resizer;
						
						if( backgroundTexture->getSize() not_eq screenSize ) {
							auto newTexture = resizer.resize( backgroundTexture, screenSize.Width, screenSize.Height, driver );
							driver->removeTexture( backgroundTexture );
							backgroundTexture = newTexture;
						}
						
						irr::video::IImage* image = resizer.textureToImage( driver, backgroundTexture );
						irr::core::stringw textureName = backgroundTexture->getName().getInternalName(); //Needed when converting the image back to a texture
						driver->removeTexture( backgroundTexture );
						backgroundTexture = nullptr;
						
						adjustImageColors( image );
						
						textureName += L"-recolored";
						backgroundTexture = resizer.imageToTexture( driver, image, textureName );
					}
				} else {
					std::wcerr << L"Could not find any background images." << std::endl;
				}
				
				break;
			}

			default: {
				StringConverter sc;
				std::wstring error = L"Background chosen (#" + sc.toStdWString( backgroundChosen ) + L") is not in switch statement.";
				throw CustomException( error );
			}
		}
	} catch( std::exception &e ) {
		std::wcerr << L"Error in MainGame::setupBackground(): " << e.what() << std::endl;
	}
	
	if( settingsManager.debug ) {
		std::wcout << L"end of setupBackground()" << std::endl;
	}
}

void MainGame::setupDevice() {
	if( settingsManager.fullscreen ) {
		irr::video::IVideoModeList* vmList = device->getVideoModeList();
		
		auto desiredResolution = settingsManager.getFullscreenResolution();
		
		if( settingsManager.autoDetectFullscreenResolution ) {
			desiredResolution = vmList->getDesktopResolution();
		}
		
		if( settingsManager.allowSmallSize ) {
			screenSize = vmList->getVideoModeResolution( irr::core::dimension2d< irr::u32 >( 1, 1 ), desiredResolution ); //Gets a video resolution between minimum (1,1) and maximum (fullscreen resolution setting)
		} else {
			screenSize = vmList->getVideoModeResolution( settingsManager.getMinimumWindowSize(), desiredResolution );
		}
	} else {
		screenSize = settingsManager.getWindowSize();
	}
	
	device->closeDevice(); //Signals to the existing device that it needs to close itself on next run() so that we can create a new device
	device->run(); //This is next run()
	device->drop(); //Cleans up after the device
	
	{
		irr::SIrrlichtCreationParameters params;
		params.DriverType = settingsManager.driverType;
		params.WindowSize = screenSize;
		params.Bits = settingsManager.getBitsPerPixel();
		params.Fullscreen = settingsManager.fullscreen;
		params.Stencilbuffer = false;
		params.Vsync = settingsManager.vsync;
		params.EventReceiver = this;
		
		if( isScreenSaver ) {
			//std::wcerr << L"Running as screensaver" << std::endl;
			try {
				auto idString = system.getEnvironmentVariable( L"XSCREENSAVER_WINDOW" );
				//std::wcout << L"idString: \"" << idString << L"\"" << std::endl;
				void * idPointer = nullptr;
				int idInt = swscanf( idString.c_str(), L"%p", &idPointer );
				//std::wcout << L"idInt: " << idInt << L" idPointer: " << idPointer << std::endl;
				
				if( idInt >= 1 ) {
					params.WindowId = idPointer;
				}
			} catch( std::exception &e ) {
				std::cout << e.what() << std::endl;
				exit( EXIT_FAILURE );
			}
		} else {
			//std::wcerr << L"Not running as screensaver" << std::endl;
		}
		
		device = createDeviceEx( params );
		
		if( isNull( device ) ) {
			std::wcerr << L"Error: Cannot create device. Trying other driver types." << std::endl;
			
			//Driver types included in the E_DRIVER_TYPE enum may not actually be supported; it depends on how Irrlicht is compiled.
			for( auto i = ( uint_fast8_t ) irr::video::EDT_COUNT; isNull( device ) and i not_eq ( uint_fast8_t ) irr::video::EDT_NULL; --i ) {
				if( device->isDriverSupported( ( irr::video::E_DRIVER_TYPE ) i ) ) {
					settingsManager.driverType = ( irr::video::E_DRIVER_TYPE ) i;
					params.DriverType = settingsManager.driverType;
					device = createDeviceEx( params );
				}
			}
			
			if( isNull( device ) ) {
				std::wcerr << L"Error: No graphical output driver types are available. Using NULL type!! Also enabling debug." << std::endl;
				settingsManager.debug = true;
				params.DriverType = irr::video::EDT_NULL;
				device = createDeviceEx( params );
			
				if( isNull( device ) ) {
					throw( CustomException( L"Unable to create the device using any driver types" ) );
				}
			}
			
		} else if ( settingsManager.debug ) {
			std::wcout << L"Got the new device" << std::endl;
		}
	}
}

/**
 * @brief Sets settings for the video driver.
 */
void MainGame::setupDriver() {
	driver = device->getVideoDriver();
	if( isNull( driver ) ) {
		throw( CustomException( std::wstring( L"Cannot get video driver" ) ) );
	} else if ( settingsManager.debug ) {
		std::wcout << L"Got the video driver" << std::endl;
	}
	
	driver->setTextureCreationFlag( irr::video::ETCF_NO_ALPHA_CHANNEL, false );
	driver->setTextureCreationFlag( irr::video::ETCF_CREATE_MIP_MAPS, false );
	if( settingsManager.driverType == irr::video::EDT_SOFTWARE or settingsManager.driverType == irr::video:: EDT_BURNINGSVIDEO ) {
		driver->setTextureCreationFlag( irr::video::ETCF_OPTIMIZED_FOR_SPEED, true );
	} else {
		driver->setTextureCreationFlag( irr::video::ETCF_OPTIMIZED_FOR_QUALITY, true );
	}
	if( settingsManager.driverType == irr::video::EDT_SOFTWARE ) {
		driver->setTextureCreationFlag( irr::video::ETCF_ALLOW_NON_POWER_2, false );
	}
}

/**
 * @brief Initializes the audio driver.
 */
void MainGame::setupMusicStuff() {
	if( SDL_Init( SDL_INIT_AUDIO ) == -1 ) {
		std::wcerr << L"Cannot initialize SDL audio." << std::endl;
		settingsManager.setPlayMusic( false );
	}
	
	if( settingsManager.getPlayMusic() ) { //Set the audio properties we hope to get: sample rate, channels, etc.
		int audioRate = MIX_DEFAULT_FREQUENCY; //MIX_DEFAULT_FREQUENCY is 22050 Hz, half the standard sample rate for CDs, and so makes a good 'lowest common denominator' for anything related to audio.
		Uint16 audioFormat = MIX_DEFAULT_FORMAT; //AUDIO_S16SYS according to documentation. CDs use signed 16-bit audio. SYS means use the system's native endianness.
		int audioChannels = MIX_DEFAULT_CHANNELS; //2 according to documentation. Almost everything uses stereo. I wish surround sound were more common.
		int audioChunkSize = 4096; //Magic number! Change it if you dare, and see what happens. SDL_Mixer has no default, but its documentation says 4096 is good if all we're playing is music. Too small and sound may skip on a slow system, too large and sound effects may lag behind the action.

		if( Mix_OpenAudio( audioRate, audioFormat, audioChannels, audioChunkSize ) not_eq 0 ) {
			std::wcerr << L"Unable to initialize audio: " << Mix_GetError() << std::endl;
			settingsManager.setPlayMusic( false );
		} else if( settingsManager.debug ) {
			std::wcout << L"Initialized audio" << std::endl;
		}
	
		if( settingsManager.debug ) {
			Mix_QuerySpec( &audioRate, &audioFormat, &audioChannels );//Don't assume we got everything we asked for above
			std::wcout << L"Audio sample rate: " << audioRate << L" Hertz. Format: ";
			// cppcheck-suppress duplicateIf
			if( audioFormat == AUDIO_U16SYS ) {
				std::wcout << L"AUDIO_U16SYS (equivalent to ";
				//cppcheck-suppress duplicateIf
				if( AUDIO_U16SYS == AUDIO_U16LSB ) {
					std::wcout << L"AUDIO_U16LSB";
				} else if( AUDIO_U16SYS == AUDIO_U16MSB ) {
					std::wcout << L"AUDIO_U16MSB";
				} else if( AUDIO_U16SYS == AUDIO_U16 ) {
					std::wcout << L"AUDIO_U16";
				} else {
					std::wcout << L"unknown";
				}
				std::wcout << L")";
			//cppcheck-suppress duplicateIf
			} else if( audioFormat == AUDIO_S16SYS ) {
				std::wcout << L"AUDIO_S16SYS (equivalent to ";
				//cppcheck-suppress duplicateIf
				if( AUDIO_S16SYS == AUDIO_S16LSB ) {
					std::wcout << L"AUDIO_S16LSB";
				} else if( AUDIO_S16SYS == AUDIO_S16MSB ) {
					std::wcout << L"AUDIO_S16MSB";
				} else if( AUDIO_S16SYS == AUDIO_S16 ) {
					std::wcout << L"AUDIO_S16";
				} else {
					std::wcout << L"unknown";
				}
				std::wcout << L")";
			} else if( audioFormat == AUDIO_U8 ) {
				std::wcout << L"AUDIO_U8";
			} else if( audioFormat == AUDIO_S8 ) {
				std::wcout << L"AUDIO_S8";
			} else if( audioFormat == AUDIO_U16LSB ) {
				std::wcout << L"AUDIO_U16LSB";
			} else if( audioFormat == AUDIO_S16LSB ) {
				std::wcout << L"AUDIO_S16LSB";
			} else if( audioFormat == AUDIO_U16MSB ) {
				std::wcout << L"AUDIO_U16MSB";
			} else if( audioFormat == AUDIO_S16MSB ) {
				std::wcout << L"AUDIO_S16MSB";
			} else if( audioFormat == AUDIO_U16 ) {
				std::wcout << L"AUDIO_U16";
			} else if( audioFormat == AUDIO_S16 ) {
				std::wcout << L"AUDIO_S16";
			} else {
				std::wcout << L"unknown";
			}
	
			std::wcout << " channels: " << audioChannels  << L" chunk size: " << audioChunkSize << std::endl;
		}
	}
	
	music = nullptr;
	
	if( settingsManager.getPlayMusic() ) { //No sense in making the music list if we've been unable to initialize the audio.
		makeMusicList();
		if( settingsManager.getPlayMusic() ) {  //playMusic may be set to false if makeMusicList() can't find any songs to play
			loadNextSong();
		}
	}
}

/**
 * Called by menuManager.
 */
/*void MainGame::setExitConfirmation( irr::gui::IGUIWindow* newWindow ) {
	exitConfirmation = newWindow;
}*/

void MainGame::displayExitConfirmation() {
	irr::core::stringw question;
	
	if( exitConfirmations.empty() ) {
		question = L"Are you sure you want to exit?";
	} else {
		question = exitConfirmations.at( currentExitConfirmation );
		currentExitConfirmation = ( currentExitConfirmation + 1 ) % exitConfirmations.size();
	}
	
	exitConfirmation = gui->addMessageBox( L"Exit?", question.c_str(), true, ( irr::gui::EMBF_YES bitor irr::gui::EMBF_NO ) );
}

/**
 * Sets loadingProgress
 * Arguments: Yes please.
 **/
void MainGame::setLoadingPercentage( float newPercent ) {
 	if( newPercent < 100 and newPercent > 0 ) {
		
		/*if( settingsManager.debug and ( newPercent > 97.5 and newPercent < 98.5 ) ) { //Every once in a while, loading gets stuck at 98%. I think I've fixed it.
			std::wcout << L"Loading percentage is " << newPercent << std::endl;
			std::wcout << L"Maze size is " << mazeManager.cols << L"x" << mazeManager.rows << std::endl;
			std::wcout << L"Number of locks is " << numLocks << std::endl;
		}*/
		
		loadingProgress = newPercent;
 	} else if( newPercent >= 100 ) {
 		loadingProgress = 100;
 	} else {
 		loadingProgress = 0;
 	}
 }

/**
 * Sets which player this client controls
 * Arguments: Yes please.
 **/
void MainGame::setMyPlayer( uint_fast8_t newPlayer ) {
	try {
		for( uint_fast8_t c = 0; c < controls.size(); ++c ) {
			if( controls.at( c ).controlsAPlayer and controls.at( c ).getPlayer() == newPlayer ) { //Eliminate duplicate controls
				controls.erase( controls.begin() + c );
				--c;
			}
		}
		
		for( uint_fast8_t c = 0; c < controls.size(); ++c ) {
			if( controls.at( c ).getPlayer() == myPlayer ) {
				controls.at( c ).setPlayer( newPlayer );
			}
		}
		
		myPlayer = newPlayer;
		
		if( playerAssigned.size() > 0 ) { //There's one point in the MainGame constructor where setMyPlayer() gets called before playerAssigned has been given a nonzero size.
			playerAssigned.at( myPlayer ) = true;
		}
	} catch( std::exception &e ) {
		std::wcerr << L"Error in MainGame::setMyPlayer(): " << e.what() << std::endl;
	}
}


void MainGame::setNumBots( uint_fast8_t newNumBots ) {
	int_fast16_t diff = ( int_fast16_t ) newNumBots - ( int_fast16_t ) settingsManager.getNumBots();
	
	bot.resize( newNumBots );
	
	{
		decltype( settingsManager.getNumPlayers() ) botControlsPlayer = settingsManager.getNumPlayers() - settingsManager.getNumBots();
		for( decltype( settingsManager.getNumBots() ) b = 0; b < settingsManager.getNumBots(); ++b ) {
			bot.at( b ).setPlayer( ( int_fast16_t ) bot.at( b ).getPlayer() + diff );
			//bot.at( b ).setPlayer( botControlsPlayer );
			//bot.at( b ).setPlayer( 0 );
			botControlsPlayer += 1;
		}
	}
}

void MainGame::setNumPlayers( uint_fast8_t newNumPlayers ) {
	int_fast16_t diff = ( int_fast16_t ) newNumPlayers - ( int_fast16_t ) settingsManager.getNumPlayers();
	
	player.resize( newNumPlayers );
	playerStart.resize( newNumPlayers );
	
	for( decltype( newNumPlayers) ps = 0; ps < newNumPlayers; ++ps ) {
		switch( settingsManager.colorMode ) {
			case SettingsManager::COLOR_MODE_DO_NOT_USE:
			case SettingsManager::GRAYSCALE: {
				playerStart.at( ps ).setColors( GRAY_GRAYSCALE, LIGHTRED_GRAYSCALE );
				break;
			}
			case SettingsManager::FULLCOLOR: {
				playerStart.at( ps ).setColors( GRAY, LIGHTRED );
				break;
			}
			case SettingsManager::GREENSCALE: {
				playerStart.at( ps ).setColors( GRAY_GREENSCALE, LIGHTRED_GREENSCALE );
				break;
			}
			case SettingsManager::AMBERSCALE: {
				playerStart.at( ps ).setColors( GRAY_AMBERSCALE, LIGHTRED_AMBERSCALE );
				break;
			}
		}
	}
	
	if( settingsManager.getNumBots() > newNumPlayers ) {
		setNumBots( newNumPlayers );
	}
	
	for( decltype( settingsManager.getNumBots() ) b = 0; b < settingsManager.getNumBots(); ++b ) {
		bot.at( b ).setPlayer( ( int_fast16_t ) bot.at( b ).getPlayer() + diff );
	}
}

irr::video::SColor MainGame::getColorBasedOnNum( uint_fast8_t num ) {
	irr::video::SColor colorOne;
	
	switch( num ) {
		case 0: {
			switch( settingsManager.colorMode ) {
				case SettingsManager::COLOR_MODE_DO_NOT_USE:
				case SettingsManager::GRAYSCALE: {
					colorOne = BLACK_GRAYSCALE;
					break;
				}
				case SettingsManager::FULLCOLOR: {
					colorOne = BLACK;
					break;
				}
				case SettingsManager::GREENSCALE: {
					colorOne = BLACK_GREENSCALE;
					break;
				}
				case SettingsManager::AMBERSCALE: {
					colorOne = BLACK_AMBERSCALE;
					break;
				}
			}
			
			break;
		}
		case 1: {
			switch( settingsManager.colorMode ) {
				case SettingsManager::COLOR_MODE_DO_NOT_USE:
				case SettingsManager::GRAYSCALE: {
					colorOne = BLUE_GRAYSCALE;
					break;
				}
				case SettingsManager::FULLCOLOR: {
					colorOne = BLUE;
					break;
				}
				case SettingsManager::GREENSCALE: {
					colorOne = BLUE_GREENSCALE;
					break;
				}
				case SettingsManager::AMBERSCALE: {
					colorOne = BLUE_AMBERSCALE;
					break;
				}
			}
			
			break;
		}
		case 2: {
			switch( settingsManager.colorMode ) {
				case SettingsManager::COLOR_MODE_DO_NOT_USE:
				case SettingsManager::GRAYSCALE: {
					colorOne = GREEN_GRAYSCALE;
					break;
				}
				case SettingsManager::FULLCOLOR: {
					colorOne = GREEN;
					break;
				}
				case SettingsManager::GREENSCALE: {
					colorOne = GREEN_GREENSCALE;
					break;
				}
				case SettingsManager::AMBERSCALE: {
					colorOne = GREEN_AMBERSCALE;
					break;
				}
			}
			
			break;
		}
		case 3: {
			switch( settingsManager.colorMode ) {
				case SettingsManager::COLOR_MODE_DO_NOT_USE:
				case SettingsManager::GRAYSCALE: {
					colorOne = CYAN_GRAYSCALE;
					break;
				}
				case SettingsManager::FULLCOLOR: {
					colorOne = CYAN;
					break;
				}
				case SettingsManager::GREENSCALE: {
					colorOne = CYAN_GREENSCALE;
					break;
				}
				case SettingsManager::AMBERSCALE: {
					colorOne = CYAN_AMBERSCALE;
					break;
				}
			}
			
			break;
		}
		case 4: {
			switch( settingsManager.colorMode ) {
				case SettingsManager::COLOR_MODE_DO_NOT_USE:
				case SettingsManager::GRAYSCALE: {
					colorOne = RED_GRAYSCALE;
					break;
				}
				case SettingsManager::FULLCOLOR: {
					colorOne = RED;
					break;
				}
				case SettingsManager::GREENSCALE: {
					colorOne = RED_GREENSCALE;
					break;
				}
				case SettingsManager::AMBERSCALE: {
					colorOne = RED_AMBERSCALE;
					break;
				}
			}
			
			break;
		}
		case 5: {
			switch( settingsManager.colorMode ) {
				case SettingsManager::COLOR_MODE_DO_NOT_USE:
				case SettingsManager::GRAYSCALE: {
					colorOne = MAGENTA_GRAYSCALE;
					break;
				}
				case SettingsManager::FULLCOLOR: {
					colorOne = MAGENTA;
					break;
				}
				case SettingsManager::GREENSCALE: {
					colorOne = MAGENTA_GREENSCALE;
					break;
				}
				case SettingsManager::AMBERSCALE: {
					colorOne = MAGENTA_AMBERSCALE;
					break;
				}
			}
			
			break;
		}
		case 6: {
			switch( settingsManager.colorMode ) {
				case SettingsManager::COLOR_MODE_DO_NOT_USE:
				case SettingsManager::GRAYSCALE: {
					colorOne = BROWN_GRAYSCALE;
					break;
				}
				case SettingsManager::FULLCOLOR: {
					colorOne = BROWN;
					break;
				}
				case SettingsManager::GREENSCALE: {
					colorOne = BROWN_GREENSCALE;
					break;
				}
				case SettingsManager::AMBERSCALE: {
					colorOne = BROWN_AMBERSCALE;
					break;
				}
			}
			
			break;
		}
		case 7: {
			switch( settingsManager.colorMode ) {
				case SettingsManager::COLOR_MODE_DO_NOT_USE:
				case SettingsManager::GRAYSCALE: {
					colorOne = GRAY_GRAYSCALE;
					break;
				}
				case SettingsManager::FULLCOLOR: {
					colorOne = GRAY;
					break;
				}
				case SettingsManager::GREENSCALE: {
					colorOne = GRAY_GREENSCALE;
					break;
				}
				case SettingsManager::AMBERSCALE: {
					colorOne = GRAY_AMBERSCALE;
					break;
				}
			}
			
			break;
		}
		case 8: {
			switch( settingsManager.colorMode ) {
				case SettingsManager::COLOR_MODE_DO_NOT_USE:
				case SettingsManager::GRAYSCALE: {
					colorOne = LIGHTGRAY_GRAYSCALE;
					break;
				}
				case SettingsManager::FULLCOLOR: {
					colorOne = LIGHTGRAY;
					break;
				}
				case SettingsManager::GREENSCALE: {
					colorOne = LIGHTGRAY_GREENSCALE;
					break;
				}
				case SettingsManager::AMBERSCALE: {
					colorOne = LIGHTGRAY_AMBERSCALE;
					break;
				}
			}
			
			break;
		}
		case 9: {
			switch( settingsManager.colorMode ) {
				case SettingsManager::COLOR_MODE_DO_NOT_USE:
				case SettingsManager::GRAYSCALE: {
					colorOne = LIGHTBLUE_GRAYSCALE;
					break;
				}
				case SettingsManager::FULLCOLOR: {
					colorOne = LIGHTBLUE;
					break;
				}
				case SettingsManager::GREENSCALE: {
					colorOne = LIGHTBLUE_GREENSCALE;
					break;
				}
				case SettingsManager::AMBERSCALE: {
					colorOne = LIGHTBLUE_AMBERSCALE;
					break;
				}
			}
		
			break;
		}
		case 10: {
			switch( settingsManager.colorMode ) {
				case SettingsManager::COLOR_MODE_DO_NOT_USE:
				case SettingsManager::GRAYSCALE: {
					colorOne = LIGHTGREEN_GRAYSCALE;
					break;
				}
				case SettingsManager::FULLCOLOR: {
					colorOne = LIGHTGREEN;
					break;
				}
				case SettingsManager::GREENSCALE: {
					colorOne = LIGHTGREEN_GREENSCALE;
					break;
				}
				case SettingsManager::AMBERSCALE: {
					colorOne = LIGHTGREEN_AMBERSCALE;
					break;
				}
			}
			
			break;
		}
		case 11: {
			switch( settingsManager.colorMode ) {
				case SettingsManager::COLOR_MODE_DO_NOT_USE:
				case SettingsManager::GRAYSCALE: {
					colorOne = LIGHTCYAN_GRAYSCALE;
					break;
				}
				case SettingsManager::FULLCOLOR: {
					colorOne = LIGHTCYAN;
					break;
				}
				case SettingsManager::GREENSCALE: {
					colorOne = LIGHTCYAN_GREENSCALE;
					break;
				}
				case SettingsManager::AMBERSCALE: {
					colorOne = LIGHTCYAN_AMBERSCALE;
					break;
				}
			}
			
			break;
		}
		case 12: {
			switch( settingsManager.colorMode ) {
				case SettingsManager::COLOR_MODE_DO_NOT_USE:
				case SettingsManager::GRAYSCALE: {
					colorOne = LIGHTRED_GRAYSCALE;
					break;
				}
				case SettingsManager::FULLCOLOR: {
					colorOne = LIGHTRED;
					break;
				}
				case SettingsManager::GREENSCALE: {
					colorOne = LIGHTRED_GREENSCALE;
					break;
				}
				case SettingsManager::AMBERSCALE: {
					colorOne = LIGHTRED_AMBERSCALE;
					break;
				}
			}
			
			break;
		}
		case 13: {
			switch( settingsManager.colorMode ) {
				case SettingsManager::COLOR_MODE_DO_NOT_USE:
				case SettingsManager::GRAYSCALE: {
					colorOne = LIGHTMAGENTA_GRAYSCALE;
					break;
				}
				case SettingsManager::FULLCOLOR: {
					colorOne = LIGHTMAGENTA;
					break;
				}
				case SettingsManager::GREENSCALE: {
					colorOne = LIGHTMAGENTA_GREENSCALE;
					break;
				}
				case SettingsManager::AMBERSCALE: {
					colorOne = LIGHTMAGENTA_AMBERSCALE;
					break;
				}
			}
			
			break;
		}
		case 14: {
			switch( settingsManager.colorMode ) {
				case SettingsManager::COLOR_MODE_DO_NOT_USE:
				case SettingsManager::GRAYSCALE: {
					colorOne = YELLOW_GRAYSCALE;
					break;
				}
				case SettingsManager::FULLCOLOR: {
					colorOne = YELLOW;
					break;
				}
				case SettingsManager::GREENSCALE: {
					colorOne = YELLOW_GREENSCALE;
					break;
				}
				case SettingsManager::AMBERSCALE: {
					colorOne = YELLOW_AMBERSCALE;
					break;
				}
			}
			
			break;
		}
		case 15: {
			switch( settingsManager.colorMode ) {
				case SettingsManager::COLOR_MODE_DO_NOT_USE:
				case SettingsManager::GRAYSCALE: {
					colorOne = WHITE_GRAYSCALE;
					break;
				}
				case SettingsManager::FULLCOLOR: {
					colorOne = WHITE;
					break;
				}
				case SettingsManager::GREENSCALE: {
					colorOne = WHITE_GREENSCALE;
					break;
				}
				case SettingsManager::AMBERSCALE: {
					colorOne = WHITE_AMBERSCALE;
					break;
				}
			}
			
			break;
		}
		default: { //Just adding this to be extra careful
			switch( settingsManager.colorMode ) {
				case SettingsManager::COLOR_MODE_DO_NOT_USE:
				case SettingsManager::GRAYSCALE: {
					colorOne = BLACK_GRAYSCALE;
					break;
				}
				case SettingsManager::FULLCOLOR: {
					colorOne = BLACK;
					break;
				}
				case SettingsManager::GREENSCALE: {
					colorOne = BLACK_GREENSCALE;
					break;
				}
				case SettingsManager::AMBERSCALE: {
					colorOne = BLACK_AMBERSCALE;
					break;
				}
			}
			
			break;
		}
	}
	
	return colorOne;
}

void MainGame::setObjectColorBasedOnNum( Object* object, uint_fast8_t num ) {
	try {
		uint_fast8_t maxNum = NUMCOLORS - 1; //Numbers can start at zero, thus the maximum is one less than the total
		uint_fast8_t numOne = num % maxNum;
		uint_fast8_t numTwo = ( num / NUMCOLORS ) % maxNum;
		
		numTwo = maxNum - numTwo;
		if( numOne == numTwo ) {
			numTwo = ( numTwo + 1 ) % maxNum;
		}
		
		irr::video::SColor colorOne;
		irr::video::SColor colorTwo;
		
		colorOne = getColorBasedOnNum( numOne );
		colorTwo = getColorBasedOnNum( numTwo );
		
		object->setColors( colorOne, colorTwo );
		
	} catch ( CustomException &e ) {
		std::wcerr << L"Error in MainGame::setObjectColorBasedOnNum(): " << e.what() << std::endl;
	} catch ( std::exception &e ) {
		std::wcerr << L"Error in MainGame::setObjectColorBasedOnNum(): " << e.what() << std::endl;
	}
}

/**
 * Sets the random number generator's seed.
 * Arguments:
 * std::minstd_rand::result_type newSeed - The new seed to use. CPlusPlus.com says this type is equal to uint_fast32_t.
 **/
void MainGame::setRandomSeed( std::minstd_rand::result_type newSeed ) {
	randomSeed = newSeed;
	randomNumberGenerator.seed( randomSeed );
	
	if( settingsManager.debug ) {
		std::wcout << L"New random seed: " << randomSeed << std::endl;
	}
	
}

/**
 * Creates a file selection dialog for loading the maze
 */
void MainGame::showLoadMazeDialog() {
	if( not isNull( loadMazeDialog ) ) {
		delete loadMazeDialog;
	}
	loadMazeDialog = new FileSelectorDialog( L"Load Maze", gui, gui->getRootGUIElement(), 0, FileSelectorDialog::EFST_OPEN_DIALOG );
	loadMazeDialog->addFileFilter( mazeManager.getFileTypeName(), mazeManager.getFileTypeExtension(), driver->getTexture( L"Images/icon.png" ) );
}

/**
 * Creates a file selection dialog for saving the maze
 */
void MainGame::showSaveMazeDialog() {
	if( not isNull( saveMazeDialog ) ) {
		delete saveMazeDialog;
	}
	saveMazeDialog = new FileSelectorDialog( L"Save Maze", gui, gui->getRootGUIElement(), 1, FileSelectorDialog::EFST_SAVE_DIALOG );
	//saveMazeDialog->addFileFilter( mazeManager.getFileTypeName(), mazeManager.getFileTypeExtension(), driver->getTexture( L"Images/icon.png" ) );
}

/**
 * Sets showingLoadingScreen to true and timeStartedLoading to the current time, then calls drawLoadingScreen().
 */
void MainGame::startLoadingScreen() {
	if( settingsManager.debug ) {
		std::wcout << L"startLoadingScreen() called" << std::endl;
	}
	drawAll();
	
	currentScreen = LOADINGSCREEN;
	timeStartedLoading = timer->getRealTime();
	//drawLoadingScreen();
	
	if( settingsManager.debug ) {
		std::wcout << L"end of startLoadingScreen()" << std::endl;
	}
}

/**
 * Takes a screenshot and saves it to a time-stamped png file.
 */
void MainGame::takeScreenShot() {
	try {
		if( settingsManager.debug ) {
			std::wcout << L"takeScreenShot() called" << std::endl;
		}
		
		irr::video::IImage* image = nullptr;
		if( not isNull( driver ) ) {
			image = driver->createScreenShot();
		}
		
		if( not isNull( image ) ) {
			irr::core::stringw filename = stringConverter.toIrrlichtStringW( PACKAGE_NAME );
			filename.append( L" screenshot " );
			
			{
				time_t currentTime = time( nullptr );
				size_t maxSize = std::max( settingsManager.dateFormat.length() * 2, ( size_t ) UINT_FAST8_MAX );
				wchar_t clockTime[ maxSize ];
				
				auto numChars = wcsftime( clockTime, maxSize, settingsManager.dateFormat.c_str(), localtime( &currentTime ) );
				if( numChars == 0 ) {
					
					numChars = wcsftime( clockTime, maxSize, settingsManager.dateFormatDefault.c_str(), localtime( &currentTime ) );
					if( numChars == 0 ) {
						
						numChars = wcsftime( clockTime, maxSize, L"%FT%T", localtime( &currentTime ) );
						if( numChars == 0 ) {
							throw( CustomException( std::wstring( L"Could not convert the time to either the specified format, nor to the default format, nor to ISO 8601.") ) );
						}
					}
				}
				filename.append( clockTime );
			}
			filename.append( L".png" );
			
			if( not driver->writeImageToFile( image, filename ) ) {
				throw( CustomException( std::wstring( L"Failed to save screen shot to file " + stringConverter.toStdWString( filename ) ) ) );
			} else {
				irr::core::stringw success = L"Screen shot saved as \"";
				success.append( filename );
				success.append( L"\"" );
				if( settingsManager.debug ) {
					std::wcout <<  stringConverter.toStdWString( success ) << std::endl;
				}
				if( not isNull( gui ) ) {
					gui->addMessageBox( L"Screenshot saved", success.c_str() );
				}
			}
			
			image->drop();
		} else {
			throw( CustomException( std::wstring( L"takeScreenShot(): Failed to take screen shot" ) ) );
		}
	} catch ( CustomException &e ) {
		std::wcerr << L"Error in MainGame::takeScreenShot(): " << e.what() << std::endl;
	} catch( std::exception &e ) {
		std::wcerr << L"Error in MainGame::takeScreenShot(): " << e.what() << std::endl;
	}
	
	if( settingsManager.debug ) {
		std::wcout << L"end of takeScreenShot()" << std::endl;
	}
}

