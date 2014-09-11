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
 * The GameManager class is the overseer of all other classes. It's also where the game's main loop is.
 */

#include "GameManager.h"
#include <boost/filesystem/fstream.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/trim_all.hpp>
#include <boost/lexical_cast.hpp>
#include <SDL.h>
#include <fileref.h>
#include <tag.h>

#if defined WINDOWS //Networking stuff
	#include <winsock>
#elif defined LINUX
	//don't know what to include here if anything
#endif //What about other operating systems? I don't know what to include for BSD etc.

//Custom user events for Irrlicht
enum user_event_t : uint_fast8_t { USER_EVENT_WINDOW_RESIZE };

//TODO: Implement an options screen
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
 * Figures out which players are human, then figures out whether they're all at the goal.
 * Returns: True if all humans are at the goal, false otherwise.
 */
bool GameManager::allHumansAtGoal() {
	try {
		std::vector< uint_fast8_t > humanPlayers; //Get a list of players

		for( decltype( numPlayers ) p = 0; p < numPlayers; ++p ) {
			humanPlayers.push_back( p );
		}

		bool result = false;

		for( decltype( numBots ) b = 0; b < numBots; ++b ) { //Remove bots from the list
			decltype( numPlayers ) botPlayer = bot.at( b ).getPlayer(); //changed decltype( bot.at( b ).getPlayer() ) to decltype( numPlayers ) because getPlayer can never exceed numPlayers, and this avoids needless function calls.

			for( decltype( numPlayers ) p = 0; p < humanPlayers.size(); ++p ) { //changed decltype( humanPlayers.size() ) to decltype( numPlayers ) because humanPlayers.size() can never exceed numPlayers but can be stored in a needlessly slow integer type.
				if( humanPlayers.at( p ) == botPlayer ) {
					humanPlayers.erase( humanPlayers.begin() + p );
				}
			}
		}

		if( humanPlayers.size() > 0 ) {
			result = true;

			for( decltype( numPlayers ) p = 0; ( p < humanPlayers.size() and result == true ); ++p ) { //changed decltype( humanPlayers.size() ) to decltype( numPlayers ) because humanPlayers.size() can never exceed numPlayers but can be stored in a needlessly slow integer type.
				if( not ( player.at( humanPlayers.at( p ) ).getX() == goal.getX() and player.at( humanPlayers.at( p ) ).getY() == goal.getY() ) ) {
					result = false;
				}
			}

		}
		return result;
	} catch( std::exception &e ) {
		std::wcerr << L"Error in GameManager::allHumansAtGoal(): " << e.what() << std::endl;
		return false;
	}
}

/**
 * Draws everything onto the screen. Calls other draw functions, including those of objects.
 */
void GameManager::drawAll() {
	try {
		driver->beginScene( true, true, backgroundColor );
		
		if( not showingLoadingScreen ) {
			if( showBackgrounds ) {
				drawBackground();
			}
			
			//Draws player trails ("footprints")
			if( markTrails ) {
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
			
			for( decltype( numPlayers ) ps = 0; ps < playerStart.size(); ++ps ) { //Put this in a separate loop from the players (below) so that the players would all be drawn after the playerStarts. Changed decltype( playerStart.size() ) to decltype( numPlayers ) because playerStart.size() can never exceed numPlayers but can be stored in a needlessly slow integer type.
				playerStart.at( ps ).draw( device, cellWidth, cellHeight );
			}
			
			//Drawing bots before human players makes it easier to play against large numbers of bots
			for( decltype( numBots ) i = 0; i < numBots; ++i ) {
				player.at( bot.at( i ).getPlayer() ).draw( device, cellWidth, cellHeight );
			}
			
			//Now we draw the players
			for( decltype( numPlayers ) p = 0; p < numPlayers; ++p ) {
				if( player.at( p ).isHuman ) {
					player.at( p ).draw( device, cellWidth, cellHeight );
				}
			}

			//We used to draw Collectables before the players due to a texture resizing bug in Irrlicht's software renderer (the bug still exists AFAIK). Collectables generally use pre-created images whereas players generally use dynamically generated images. This made players potentially get covered by Collectables and thus invisible. Now that players can hold Collectables, we want them drawn on top of the players.
			for( decltype( stuff.size() ) i = 0; i < stuff.size(); ++i ) {
				stuff.at( i ).draw( device, cellWidth, cellHeight );
			}

			goal.draw( device, cellWidth, cellHeight );

			mazeManager.draw( driver, cellWidth, cellHeight );

			if( showingMenu ) {
				menuManager.draw( driver );
			}


			uint_fast32_t spaceBetween = windowSize.Height / 30;
			uint_fast32_t textY = spaceBetween;
			irr::core::dimension2d< irr::u32 > tempDimensions;

			{
				time_t currentTime = time( nullptr );
				wchar_t clockTime[ 9 ];
				wcsftime( clockTime, 9, L"%H:%M:%S", localtime( &currentTime ) );
				clockTime[ 8 ] = '\0';
				tempDimensions = clockFont->getDimension( clockTime );
				irr::core::rect< irr::s32 > tempRectangle( viewportSize.Width + 1, textY, tempDimensions.Width + ( viewportSize.Width + 1 ), tempDimensions.Height + textY );
				clockFont->draw( clockTime, tempRectangle, LIGHTMAGENTA, true, true, &tempRectangle );
			}

			{
				irr::core::stringw timeLabel( L"Time:" );
				textY += tempDimensions.Height;
				tempDimensions = textFont->getDimension( stringConverter.toStdWString( timeLabel ).c_str() ); //stringConverter.toWCharArray( timeLabel ) );
				irr::core::rect< irr::s32 > tempRectangle( viewportSize.Width + 1, textY, tempDimensions.Width + ( viewportSize.Width + 1 ), tempDimensions.Height + textY );
				textFont->draw( L"Time:", tempRectangle, YELLOW, true, true, &tempRectangle );
			}

			{
				irr::core::stringw timerStr( "" );
				timerStr += ( timer->getTime() / 1000 );
				timerStr += L" seconds";
				textY += tempDimensions.Height;
				tempDimensions = textFont->getDimension( stringConverter.toStdWString( timerStr ).c_str() ); //stringConverter.toWCharArray( timerStr ) );
				irr::core::rect< irr::s32 > tempRectangle( viewportSize.Width + 1, textY, tempDimensions.Width + ( viewportSize.Width + 1 ), tempDimensions.Height + textY );
				textFont->draw( timerStr, tempRectangle, YELLOW, true, true, &tempRectangle );
			}

			{
				irr::core::stringw keysFoundStr( L"Keys found:" );
				textY += tempDimensions.Height;
				tempDimensions = textFont->getDimension( stringConverter.toStdWString( keysFoundStr ).c_str() ); //stringConverter.toWCharArray( keysFoundStr ) );
				irr::core::rect< irr::s32 > tempRectangle( viewportSize.Width + 1, textY, tempDimensions.Width + ( viewportSize.Width + 1 ), tempDimensions.Height + textY );
				textFont->draw( keysFoundStr, tempRectangle, YELLOW, true, true, &tempRectangle );
			}

			{
				irr::core::stringw keyStr;
				keyStr += numKeysFound;
				keyStr += L"/";
				keyStr += numLocks;
				textY += tempDimensions.Height;
				tempDimensions = textFont->getDimension( stringConverter.toStdWString( keyStr ).c_str() ); //stringConverter.toWCharArray( keyStr ) );
				irr::core::rect< irr::s32 > tempRectangle( viewportSize.Width + 1, textY, tempDimensions.Width + ( viewportSize.Width + 1 ), tempDimensions.Height + textY );
				textFont->draw( keyStr, tempRectangle, YELLOW, true, true, &tempRectangle );
			}

			{
				irr::core::stringw seedLabel( L"Random seed:" );
				textY += tempDimensions.Height;
				tempDimensions = textFont->getDimension( stringConverter.toStdWString( seedLabel ).c_str() );// stringConverter.toWCharArray( seedLabel ) );
				irr::core::rect< irr::s32 > tempRectangle( viewportSize.Width + 1, textY, tempDimensions.Width + ( viewportSize.Width + 1 ), tempDimensions.Height + textY );
				textFont->draw( seedLabel, tempRectangle, YELLOW, true, true, &tempRectangle );
			}

			{
				irr::core::stringw seedStr( randomSeed );
				textY += tempDimensions.Height;
				tempDimensions = textFont->getDimension( stringConverter.toStdWString( seedStr ).c_str() ); //stringConverter.toWCharArray( seedStr ) );
				irr::core::rect< irr::s32 > tempRectangle( viewportSize.Width + 1, textY, tempDimensions.Width + ( viewportSize.Width + 1 ), tempDimensions.Height + textY );
				textFont->draw( seedStr, tempRectangle, YELLOW, true, true, &tempRectangle );
			}

			{
				irr::core::stringw headfor( L"Head for" );
				tempDimensions = textFont->getDimension( stringConverter.toStdWString( headfor ).c_str() ); //stringConverter.toWCharArray( headfor ) );
				textY += tempDimensions.Height;
				if( textY < ( ( windowSize.Height / 2 ) - tempDimensions.Height ) ) {
					textY = ( ( windowSize.Height / 2 ) - tempDimensions.Height );
				}
				if( numKeysFound >= numLocks ) {
					irr::core::rect< irr::s32 > tempRectangle( viewportSize.Width + 1, textY, tempDimensions.Width + ( viewportSize.Width + 1 ), tempDimensions.Height + textY );
					textFont->draw( headfor, tempRectangle, LIGHTMAGENTA, true, true, &tempRectangle );
				}
			}

			{
				irr::core::stringw theexit( L"the exit!" );
				tempDimensions = textFont->getDimension( stringConverter.toStdWString( theexit ).c_str() ); //stringConverter.toWCharArray( theexit ) );
				textY += tempDimensions.Height;
				if( numKeysFound >= numLocks ) {
					irr::core::rect< irr::s32 > tempRectangle( viewportSize.Width + 1, textY, tempDimensions.Width + ( viewportSize.Width + 1 ), tempDimensions.Height + textY );
					textFont->draw( theexit, tempRectangle, LIGHTCYAN, true, true, &tempRectangle );
				}
			}

			if( playMusic ) {
				{
					irr::core::stringw nowplaying( L"Now playing:" );
					textY += tempDimensions.Height;
					tempDimensions = textFont->getDimension( stringConverter.toStdWString( nowplaying ).c_str() ); //stringConverter.toWCharArray( nowplaying ) );
					irr::core::rect< irr::s32 > tempRectangle( viewportSize.Width + 1, textY, tempDimensions.Width + ( viewportSize.Width + 1 ), tempDimensions.Height + textY );
					textFont->draw( nowplaying, tempRectangle, YELLOW, true, true, &tempRectangle );
				}

				{
					textY += tempDimensions.Height;
					tempDimensions = musicTagFont->getDimension( stringConverter.toStdWString( musicTitle ).c_str() ); //stringConverter.toWCharArray( musicTitle ) );
					irr::core::rect< irr::s32 > tempRectangle( viewportSize.Width + 1, textY, tempDimensions.Width + ( viewportSize.Width + 1 ), tempDimensions.Height + textY );
					musicTagFont->draw( musicTitle, tempRectangle, LIGHTGREEN, true, true, &tempRectangle );
				}

				{
					irr::core::stringw by( L"by" );
					textY += tempDimensions.Height;
					tempDimensions = textFont->getDimension( stringConverter.toStdWString( by ).c_str() ); //stringConverter.toWCharArray( by ) );
					irr::core::rect< irr::s32 > tempRectangle( viewportSize.Width + 1, textY, tempDimensions.Width + ( viewportSize.Width + 1 ), tempDimensions.Height + textY );
					textFont->draw( by, tempRectangle, YELLOW, true, true, &tempRectangle );
				}

				{
					textY += tempDimensions.Height;
					tempDimensions = musicTagFont->getDimension( stringConverter.toStdWString( musicArtist ).c_str() ); //stringConverter.toWCharArray( musicArtist ) );
					irr::core::rect< irr::s32 > tempRectangle( viewportSize.Width + 1, textY, tempDimensions.Width + ( viewportSize.Width + 1 ), tempDimensions.Height + textY );
					musicTagFont->draw( musicArtist, tempRectangle, LIGHTGREEN, true, true, &tempRectangle );
				}

				{
					irr::core::stringw fromalbum( L"from album" );
					textY += tempDimensions.Height;
					tempDimensions = textFont->getDimension( stringConverter.toStdWString( fromalbum ).c_str() ); //stringConverter.toWCharArray( fromalbum ) );
					irr::core::rect< irr::s32 > tempRectangle( viewportSize.Width + 1, textY, tempDimensions.Width + ( viewportSize.Width + 1 ), tempDimensions.Height + textY );
					textFont->draw( fromalbum, tempRectangle, YELLOW, true, true, &tempRectangle );
				}

				{
					textY += tempDimensions.Height;
					tempDimensions = musicTagFont->getDimension( stringConverter.toStdWString( musicAlbum ).c_str() ); //stringConverter.toWCharArray( musicAlbum ) );
					irr::core::rect< irr::s32 > tempRectangle( viewportSize.Width + 1, textY, tempDimensions.Width + ( viewportSize.Width + 1 ), tempDimensions.Height + textY );
					musicTagFont->draw( musicAlbum, tempRectangle, LIGHTGRAY, true, true, &tempRectangle );
				}

				{
					irr::core::stringw volume( L"Volume:" );
					textY += tempDimensions.Height;
					tempDimensions = textFont->getDimension( stringConverter.toStdWString( volume ).c_str() ); //stringConverter.toWCharArray( volume ) );
					irr::core::rect< irr::s32 > tempRectangle( viewportSize.Width + 1, textY, tempDimensions.Width + ( viewportSize.Width + 1 ), tempDimensions.Height + textY );
					textFont->draw( volume, tempRectangle, YELLOW, true, true, &tempRectangle );
				}

				{
					irr::core::stringw volumeNumber( musicVolume );
					volumeNumber.append( L"%" );
					textY += tempDimensions.Height;
					tempDimensions = textFont->getDimension( stringConverter.toStdWString( volumeNumber ).c_str() ); //stringConverter.toWCharArray( volumeNumber ) );
					irr::core::rect< irr::s32 > tempRectangle( viewportSize.Width + 1, textY, tempDimensions.Width + ( viewportSize.Width + 1 ), tempDimensions.Height + textY );
					textFont->draw( volumeNumber, tempRectangle, LIGHTRED, true, true, &tempRectangle );
				}
			}

			gui->drawAll();
		} else {
			drawLoadingScreen();
		}
		
		driver->endScene();
	} catch( std::exception &e ) {
		std::wcerr << L"Error in GameManager::drawAll(): " << e.what() << std::endl;
	}
}

/**
 * Draws background animations. Assumes that driver->beginScene() has already been called. Should be called by drawAll().
 */
void GameManager::drawBackground() {
	try {
		switch( backgroundChosen ) {
			case 0:
			case 1: {
				backgroundSceneManager->drawAll();
				break;
			}
			case 2: {
				if( backgroundTexture not_eq 0 ) {
					if( backgroundTexture->getSize() not_eq windowSize ) {
						backgroundTexture = resizer.resize( backgroundTexture, windowSize.Width, windowSize.Height, driver );
					}
					driver->draw2DImage( backgroundTexture, irr::core::position2d< irr::s32 >( 0, 0 ) );
				}
				break;
			}
			case 3: {
				driver->setRenderTarget( backgroundTexture, false, true, backgroundColor );
				backgroundSceneManager->drawAll();
				driver->setRenderTarget( irr::video::ERT_FRAME_BUFFER, false, false, backgroundColor );
				driver->draw2DImage( backgroundTexture, irr::core::position2d< irr::s32 >( 0, 0 ) );
				break;
			}
		}
	} catch( std::exception &e ) {
		std::wcerr << L"Error in GameManager::drawBackground(): " << e.what() << std::endl;
	}
}

/**
 * Draws the loading screen. Assumes that driver->beginScene() has already been called. Should be called by drawAll().
 */
void GameManager::drawLoadingScreen() {
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
				int_fast32_t textX = ( windowSize.Width / 2 ) - ( loadingDimensions.Width / 2 );
				irr::core::rect< irr::s32 > tempRectangle( textX, Y, ( windowSize.Width / 2 ) + ( loadingDimensions.Width / 2 ), loadingDimensions.Height + Y );
				loadingFont->draw( loading, tempRectangle, YELLOW, true, true, &tempRectangle );
				Y += loadingDimensions.Height + 1;
			}
			
			{
				std::wstring percentString = stringConverter.toStdWString( loadingProgress, L"%05.1f%%", 7 ); //7 is the length that L"%05.1f%%" expands to plus one extra to terminate the resulting string with a null
				auto percentDimensions = loadingFont->getDimension( percentString.c_str() );
				irr::core::recti progressBarOutline( 0, Y, windowSize.Width, Y + percentDimensions.Height );
				driver->draw2DRectangleOutline( progressBarOutline, GRAY );
				irr::core::recti progressBarFilled( 0, Y, windowSize.Width * loadingProgress / 100, Y + percentDimensions.Height );
				driver->draw2DRectangle( LIGHTGRAY, progressBarFilled );
				int_fast32_t textX = ( windowSize.Width / 2 ) - ( percentDimensions.Width / 2 );
				irr::core::recti percentRectangle( textX, Y, ( windowSize.Width / 2 ) + ( percentDimensions.Width / 2 ), percentDimensions.Height + Y );
				loadingFont->draw( stringConverter.toIrrlichtStringW( percentString ), percentRectangle, YELLOW, true, true, &percentRectangle );
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
						tipFont->draw( proTipPrefix, tempRectangle, LIGHTCYAN, true, true, &tempRectangle );
					}
					
					{
						irr::core::rect< irr::s32 > tempRectangle( textX + proTipPrefixDimensions.Width, Y, proTipDimensions.Width + textX + proTipPrefixDimensions.Width, proTipDimensions.Height + Y );
						tipFont->draw( proTips.at( currentProTip ), tempRectangle, WHITE, true, true, &tempRectangle );
					}
					
					Y += proTipHeight;
				}
			}
			
			drawStats( Y );
		}
	} catch( std::exception &e ) {
		std::wcerr << L"Error in GameManager::drawLoadingScreen(): " << e.what() << std::endl;
	}
	catch( std::wstring e ) {
		std::wcerr << L"Error in GameManager::drawLoadingScreen(): " << e << std::endl;
	}
}

/**
 * Randomly selects a logo and draws it on screen. Should NOT be called by drawAll().
 */
 void GameManager::drawLogo() {
	try {
		if( isNull( logoTexture ) ) {
			std::wstring error = L"drawLogo() called but logoTexture is null.";
			throw error;
		}

		if( logoTexture->getSize() not_eq windowSize ) {
			logoTexture = resizer.resize( logoTexture, windowSize.Width, windowSize.Height, driver );
		}

		if( isNull( logoTexture ) ) {
			std::wstring error = L"Cannot resize logo texture.";
			throw error;
		} else {
			driver->draw2DImage( logoTexture, irr::core::position2d< irr::s32 >( 0, 0 ) );
		}
	} catch( std::wstring error ) {
		std::wcerr << L"Error in drawLogo(): " << error << std::endl;
	} catch ( std::exception &error ) {
		std::wcerr << L"Error in drawLogo(): " << error.what() << std::endl;
	}
 }

/**
 * Should only be called from drawLoadingScreen(). Just putting it here for code separation/readability.
 * //TODO: Add more stats (estimated difficulty of maze, number of cells backtracked, etc) to loading screen.
 */
void GameManager::drawStats( uint_fast32_t textY ) {
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
			statsFont->draw( winnersLabel, tempRectangle, WHITE, true, true, &tempRectangle );

			if( tempDimensions.Width + textXOriginal > textX ) {
				textX = tempDimensions.Width + textXOriginal;
			}
			textYSteps = textYOriginal + tempDimensions.Height;
		}

		{
			decltype( statsFont->getDimension( L"" ) ) tempDimensions = statsFont->getDimension( stringConverter.toStdWString( steps ).c_str() );// stringConverter.toWCharArray( steps ) );
			irr::core::rect< irr::s32 > tempRectangle( textXOriginal, textYSteps, tempDimensions.Width + textXOriginal, tempDimensions.Height + textYSteps );
			statsFont->draw( steps, tempRectangle, WHITE, true, true, &tempRectangle );

			if( tempDimensions.Width + textXOriginal > textX ) {
				textX = tempDimensions.Width + textXOriginal;
			}
			textYTimes = textYSteps + tempDimensions.Height;
		}

		{
			decltype( statsFont->getDimension( L"" ) ) tempDimensions = statsFont->getDimension( stringConverter.toStdWString( times ).c_str() ); //stringConverter.toWCharArray( times ) );
			irr::core::rect< irr::s32 > tempRectangle( textXOriginal, textYTimes, tempDimensions.Width + textXOriginal, tempDimensions.Height + textYTimes );
			statsFont->draw( times, tempRectangle, WHITE, true, true, &tempRectangle );

			if( tempDimensions.Width + textXOriginal > textX ) {
				textX = tempDimensions.Width + textXOriginal;
			}
			textYKeys = textYTimes + tempDimensions.Height;
		}

		{
			decltype( statsFont->getDimension( L"" ) ) tempDimensions = statsFont->getDimension( stringConverter.toStdWString( keysFoundPerPlayer ).c_str() ); //stringConverter.toWCharArray( keysFoundPerPlayer ) );
			irr::core::rect< irr::s32 > tempRectangle( textXOriginal, textYKeys, tempDimensions.Width + textXOriginal, tempDimensions.Height + textYKeys );
			statsFont->draw( keysFoundPerPlayer, tempRectangle, WHITE, true, true, &tempRectangle );

			if( tempDimensions.Width + textXOriginal > textX ) {
				textX = tempDimensions.Width + textXOriginal;
			}
			textYScores = textYKeys + tempDimensions.Height;
		}

		{
			decltype( statsFont->getDimension( L"" ) ) tempDimensions = statsFont->getDimension( stringConverter.toStdWString( scores ).c_str() ); //stringConverter.toWCharArray( scores ) );
			irr::core::rect< irr::s32 > tempRectangle( textXOriginal, textYScores, tempDimensions.Width + textXOriginal, tempDimensions.Height + textYScores );
			statsFont->draw( scores, tempRectangle, WHITE, true, true, &tempRectangle );

			if( tempDimensions.Width + textXOriginal > textX ) {
				textX = tempDimensions.Width + textXOriginal;
			}
			textYScoresTotal = textYScores + tempDimensions.Height;
		}

		{
			decltype( statsFont->getDimension( L"" ) ) tempDimensions = statsFont->getDimension( stringConverter.toStdWString( scoresTotal ).c_str() ); //stringConverter.toWCharArray( scoresTotal ) );
			irr::core::rect< irr::s32 > tempRectangle( textXOriginal, textYScoresTotal, tempDimensions.Width + textXOriginal, tempDimensions.Height + textYScoresTotal );
			statsFont->draw( scoresTotal, tempRectangle, WHITE, true, true, &tempRectangle );

			if( tempDimensions.Width + textXOriginal > textX ) {
				textX = tempDimensions.Width + textXOriginal;
			}
			//textYScoresTotal = textYScores + tempDimensions.Height;
		}

		textY = textYOriginal;
		//Now we go through and draw the actual player stats
		for( decltype( numPlayers ) p = 0; p < winnersLoadingScreen.size(); ++p ) { //changed decltype( winnersLoadingScreen.size() ) to decltype( numPlayers ) because winnersLoadingScreen.size() can never exceed numPlayers but can be stored in a needlessly slow integer type.
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
				if( textX >= windowSize.Width - tempDimensions.Width ) {
					textX = 0;
					textXOld = 0;
					decltype( textY ) separatorY = textYScoresTotal + tempDimensions.Height + 1;
					driver->draw2DLine( irr::core::position2d< irr::s32  >( 0, separatorY ), irr::core::position2d< irr::s32 >( windowSize.Width, separatorY ) );
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
void GameManager::eraseCollectable( uint_fast8_t item ) {
	try {
		if( debug ) {
			std::wcout << L"eraseCollectable() called" << std::endl;
		}
		
		if( item < stuff.size() ) {
			stuff.erase( stuff.begin() + item );
			for( decltype( numPlayers ) p = 0; p < player.size(); ++p ) {
				if( player.at( p ).hasItem() and player.at( p ).getItem() > item ) {
					player.at( p ).giveItem( player.at( p ).getItem() - 1, player.at( p ).getItemType() );
				}
			}
		}
		if( debug ) {
				std::wcout << L"end of eraseCollectable()" << std::endl;
		}
	} catch ( std::exception &error ) {
		std::wcerr << L"Error in eraseCollectable(): " << error.what() << std::endl;
	}
}

/**
 * This object's destructor. Destroys stuff.
 * I... am... DESTRUCTOOOOOOORRRRR!!!!!!!!!
 */
GameManager::~GameManager() {
	try {
		if( debug ) {
			std::wcout << L"GameManager destructor called" << std::endl;
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

		if( playMusic ) {
			Mix_HaltMusic();
			Mix_FreeMusic( music );
			Mix_CloseAudio();

			while( Mix_Init( 0 ) ) {
				Mix_Quit();
			}

			SDL_Quit();
		}
		
		if( debug ) {
			std::wcout << L"end of GameManager destructor" << std::endl;
		}
	} catch( std::exception &e ) {
		std::wcerr << L"Error in GameManager::~GameManager(): " << e.what() << std::endl;
	}
}

/**
 * This object's constructor. Does lots of very important stuff.
 * Since this constructor is so big, maybe some parts should be split off into separate functions for readability.
 */
GameManager::GameManager() {
	try {
		#ifdef DEBUG //Not the last place debug is set to true or false; look at readPrefs()
			debug = true;
		#else
			debug = false;
		#endif
		if( debug ) {
			std::wcout << L"GameManager constructor called" << std::endl;
		}
		//Just wanted to be totally sure that these point to nullptr before being set otherwise
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

		loading = L"Loading...";
		proTipPrefix = L"Pro tip: ";
		winnersLabel = L"Winners: ";
		steps = L"Steps: ";
		times = L"Times: ";
		keysFoundPerPlayer = L"Keys found: ";
		scores = L"Scores: ";
		scoresTotal = L"Total scores: ";
		network.setGameManager( this );
		mazeManager.setGameManager( this );
		isServer = false;
		antiAliasFonts = true;
		currentProTip = 0;
		sideDisplaySizeDenominator = 6; //What fraction of the screen's width is set aside for displaying text, statistics, etc. during play.
		minWidth = 640;
		minHeight = 480;
		currentDirectory = boost::filesystem::current_path();
		haveShownLogo = false;
		showingMenu = true;
		donePlaying = false;
		lastTimeControlsProcessed = 0;
		controlProcessDelay = 100;
		backgroundColor = BLACK; //Every background should set this in setupBackground(); putting it here just in case.
		backgroundFilePath = L"";
		
		device = irr::createDevice( irr::video::EDT_NULL ); //Must create a device before calling readPrefs();
		
		if( isNull( device ) ) {
			throw( std::wstring( L"Cannot create null device. Something is definitely wrong here!" ) );
		}
		
		readPrefs();
		
		if ( debug ) {
			std::wcout << L"Read prefs, now setting controls" << std::endl;
		}
		
		setControls();
		
		if( fullscreen ) {
			irr::video::IVideoModeList* vmList = device->getVideoModeList();
			if( allowSmallSize ) {
				windowSize = vmList->getVideoModeResolution( irr::core::dimension2d< irr::u32 >( 1, 1 ), device->getVideoModeList()->getDesktopResolution() ); //Gets a video resolution between minimum (1,1) and maximum (desktop resolution)
			} else {
				windowSize = vmList->getVideoModeResolution( irr::core::dimension2d< irr::u32 >( minWidth, minHeight ), device->getVideoModeList()->getDesktopResolution() );
			}
		}
		
		viewportSize.set( windowSize.Width - ( windowSize.Width / sideDisplaySizeDenominator ), windowSize.Height - 1 );
		
		device->closeDevice(); //Signals to the existing device that it needs to close itself on next run() so that we can create a new device
		device->run(); //This is next run()
		device->drop(); //Cleans up after the device
		
		bool sbuffershadows = false; //Would not be visible anyway since this game is 2D
		IEventReceiver* receiver = this;
		
		device = createDevice( driverType, windowSize, bitsPerPixel, fullscreen, sbuffershadows, vsync, receiver ); //Most of these parameters were read from the preferences file
		
		if( isNull( device ) ) {
			std::wcerr << L"Error: Cannot create device. Trying other driver types." << std::endl;
			
			//Driver types included in the E_DRIVER_TYPE enum may not actually be supported; it depends on how Irrlicht is compiled.
			for( auto i = ( uint_fast8_t ) irr::video::EDT_COUNT; isNull( device ) and i not_eq ( uint_fast8_t ) irr::video::EDT_NULL; --i ) {
				if( device->isDriverSupported( ( irr::video::E_DRIVER_TYPE ) i ) ) {
					driverType = ( irr::video::E_DRIVER_TYPE ) i;
					device = createDevice( driverType, windowSize, bitsPerPixel, fullscreen, sbuffershadows, vsync, receiver );
				}
			}
			
			if( isNull( device ) ) {
				std::wcerr << L"Error: No graphical output driver types are available. Using NULL type!! Also enabling debug." << std::endl;
				device = createDevice( irr::video::EDT_NULL, windowSize, bitsPerPixel, fullscreen, sbuffershadows, vsync, receiver );
				debug = true;
			}
		} else if ( debug ) {
			std::wcout << L"Got the new device" << std::endl;
		}
		
		driver = device->getVideoDriver(); //Not sure if this would be possible with a null device, which is why we don't exit
		if( isNull( driver ) ) {
			throw( std::wstring( L"Cannot get video driver" ) );
		} else if ( debug ) {
			std::wcout << L"Got the video driver" << std::endl;
		}
		
		driver->setTextureCreationFlag( irr::video::ETCF_NO_ALPHA_CHANNEL, false );
		driver->setTextureCreationFlag( irr::video::ETCF_CREATE_MIP_MAPS, false );
		if( driverType == irr::video::EDT_SOFTWARE or driverType == irr::video:: EDT_BURNINGSVIDEO ) {
			driver->setTextureCreationFlag( irr::video::ETCF_OPTIMIZED_FOR_SPEED, true );
		} else {
			driver->setTextureCreationFlag( irr::video::ETCF_OPTIMIZED_FOR_QUALITY, true );
		}
		if( driverType == irr::video::EDT_SOFTWARE ) {
			driver->setTextureCreationFlag( irr::video::ETCF_ALLOW_NON_POWER_2, false );
		}

		pickLogo();
		driver->beginScene( false, false ); //These falses specify whether the back buffer and z buffer should be cleared. Since this is the first time drawing anything, there's no need to clear anything beforehand.
		drawLogo(); //Why the fuck isn't this working consistently? Sometimes it draws, sometimes it only thinks it draws. Had to hack the drawLoadingScreen() function (which gets called several times, therefore is likely to work at least once).
		driver->endScene();

		gui = device->getGUIEnvironment();
		if( isNull( gui ) ) {
			throw( std::wstring( L"Cannot get GUI environment" ) );
		} else {
			if ( debug ) {
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
		
		if( debug ) {
			device->getLogger()->setLogLevel( irr::ELL_INFORMATION );
		} else {
			device->getLogger()->setLogLevel( irr::ELL_ERROR );
		}
		
		backgroundSceneManager = device->getSceneManager(); //Not sure if this would be possible with a null device, which is why we don't exit
		if( isNull( backgroundSceneManager ) ) {
			throw( std::wstring( L"Cannot get scene manager" ) );
		} else if ( debug ) {
			std::wcout << L"Got the scene manager" << std::endl;
		}
		
		
		if( playMusic ) {
			
			if( SDL_Init( SDL_INIT_AUDIO ) == -1 ) {
				std::wcerr << L"Cannot initialize SDL audio." << std::endl;
				playMusic = false;
			}
			
			{//Set the audio properties we hope to get: sample rate, channels, etc.
				int audioRate = MIX_DEFAULT_FREQUENCY; //MIX_DEFAULT_FREQUENCY is 22050 Hz, half the standard sample rate for CDs, and so makes a good 'lowest common denominator' for anything related to audio.
				Uint16 audioFormat = MIX_DEFAULT_FORMAT; //AUDIO_S16SYS according to documentation. CDs use signed 16-bit audio. SYS means use the system's native endianness.
				int audioChannels = MIX_DEFAULT_CHANNELS; //2 according to documentation. Almost everything uses stereo. I wish surround sound were more common.
				int audioChunkSize = 4096; //Magic number! Change it if you dare, and see what happens. SDL_Mixer has no default, but its documentation says 4096 is good if all we're playing is music. Too small and sound may skip on a slow system, too large and sound effects may lag behind the action.

				if( Mix_OpenAudio( audioRate, audioFormat, audioChannels, audioChunkSize ) not_eq 0 ) {
					std::wcerr << L"Unable to initialize audio: " << Mix_GetError() << std::endl;
					playMusic = false;
				} else if( debug ) {
					std::wcout << L"Initialized audio" << std::endl;
				}

				if( debug ) {
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

			if( playMusic ) { //No sense in making the music list if we've been unable to initialize the audio.
				makeMusicList();
				if( playMusic ) {  //playMusic may be set to false if makeMusicList() can't find any songs to play
					loadNextSong();
				}
			}
		}

		loadFonts();
		
		menuManager.setPositions( windowSize.Height );

		if ( debug ) {
			std::wcout << L"Resizing player and playerStart vectors to " << numPlayers << std::endl;
		}

		player.resize( numPlayers );
		playerStart.resize( numPlayers );

		if( numBots > numPlayers ) {
			numBots = numPlayers;
		}

		if( numBots > 0 ) {
			if ( debug ) {
				std::wcout << L"Resizing numBots vector to " << numBots << std::endl;
			}

			bot.resize( numBots );

			for( decltype( numBots ) i = 0; i < numBots; ++i ) {
				bot.at( i ).setPlayer( numPlayers - ( i + 1 ) ) ;
				player.at( bot.at( i ).getPlayer() ).isHuman = false;
				//bot.at( i ).setup( mazeManager.maze, mazeManager.cols, mazeManager.rows, this, botsKnowSolution, botAlgorithm, botMovementDelay );
				bot.at( i ).setup( this, botsKnowSolution, botAlgorithm, botMovementDelay );
			}
		}

		for( decltype( numPlayers ) p = 0; p < numPlayers; ++p ) {
			player.at( p ).setPlayerNumber( p );
			player.at( p ).setColorBasedOnNum();
			player.at( p ).loadTexture( device );
			player.at( p ).setGM( this );
		}

		goal.loadTexture( device );

		if( enableController and device->activateJoysticks( controllerInfo ) and debug ) { //activateJoysticks fills controllerInfo with info about each controller
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
		} else if( debug ) {
			std::wcout << L"controller support is not enabled." << std::endl;
		}

		//Set up networking
		network.setup( isServer );

		timer = device->getTimer();

		if( debug ) {
			std::wcout << L"end of GameManager constructor" << std::endl;
		}
	} catch( std::exception &e ) {
		std::wcerr << L"Error in GameManager::GameManager(): " << e.what() << std::endl;
	} catch( std::wstring &e ) {
		std::wcerr << L"Error in GameManager::GameManager(): " << e << std::endl;
	}
}

/**
 * Lets other objects get a pointer to one of the collectables, probably to see if a player has touched one.
 * @param uint_fast8_t collectable: The number of the item desired.
 * @return A pointer to a Collectable.
 */
Collectable* GameManager::getCollectable( uint_fast8_t collectable ) {
	try {
		return &stuff.at( collectable );
	} catch( std::exception &e ) {
		std::wcerr << L"Error in GameManager::getCollectable(): " << e.what() << std::endl;
	}
	return nullptr;
}

/**
 * Lets other objects know whether we're in debug mode.
 * Returns: True if debug is true, false otherwise.
 */
bool GameManager::getDebugStatus() {
	try {
		return debug;
	} catch( std::exception &e ) {
		std::wcerr << L"Error in GameManager::getDebugStatus(): " << e.what() << std::endl;
		return true;
	}
}

/**
 * Lets other objects get a pointer to the goal, perhaps to get its location.
 * Returns: A pointer to the goal object.
 */
Goal* GameManager::getGoal() {
	return &goal;
}

/**
 * Lets other objects get a pointer to one of the keys, probably to see if a player has touched one.
 * @param uint_fast8_t key: The number of the key desired.
 * @return A pointer to a key.
 */
Collectable* GameManager::getKey( uint_fast8_t key ) {
	try {
		if( debug ) {
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
			
			if( debug ) {
				std::wcout << L"end of getKey() (key is valid)" << std::endl;
			}
			
			return result;
		} else {
			std::wstring error = L"Collectable ";
			error += static_cast< unsigned int >( key );
			error += L" is not a key.";
			throw error;
		}
	} catch( std::exception &e ) {
		std::wcerr << L"Error in GameManager::getKey(): " << e.what() << std::endl;
	} catch( std::wstring e ) {
		std::wcerr << L"Error in GameManager::getKey(): " << e << std::endl;
	}
	
	if( debug ) {
		std::wcout << L"end of getKey() (key is not valid)" << std::endl;
	}
	return nullptr;
}

/**
 * Other objects can't properly add to the loading percentage if they can't see what it is first
 */
float GameManager::getLoadingPercentage() {
	return loadingProgress;
}

/**
 * Returns the highest number that the random number generator can output
 **/
std::minstd_rand::result_type GameManager::getMaxRandomNumber() {
	return randomNumberGenerator.max();
}

/**
 * Lets other objects get a pointer to the maze manager, perhaps to get the maze.
 * Returns: A pointer to the mazeManager object.
 */
MazeManager* GameManager::getMazeManager() {
	try {
		return &mazeManager;
	} catch( std::exception &e ) {
		std::wcerr << L"Error in GameManager::getMazeManager(): " << e.what() << std::endl;
	}
	return nullptr;
}

/**
 * Lets other objects know how many collectables there are.
 * Returns: the number of Collectables.
 */
 uint_fast8_t GameManager::getNumCollectables() {
	return stuff.size();
 }

/**
 * Lets other objects know how many locks there are.
 * Returns: the number of keys.
 */
uint_fast8_t GameManager::getNumKeys() {
	try {
		
		uint_fast8_t result = 0;
		for( decltype( stuff.size() ) s = 0; s < stuff.size(); ++s ) {
			if( stuff.at( s ).getType() == Collectable::KEY ) {
				result++;
			}
		}
		
		return result;
	} catch( std::exception &e ) {
		std::wcerr << L"Error in GameManager::getNumKeys(): " << e.what() << std::endl;
		return UINT_FAST8_MAX;
	}
}

/**
 * Lets other objects get a pointer to a player object.
 * Arguments:
 * --- uint_fast8_t p: the desired player
 * Returns: A pointer to the desired player if that player exists and if no exception is caught, nullptr otherwise.
 **/
Player* GameManager::getPlayer( uint_fast8_t p ) {
	try {
		if( p < numPlayers ) {
			return &player.at( p );
		} else {
			throw( std::wstring( L"Request for player (" ) + stringConverter.toStdWString( p ) + L") >= numPlayers (" + stringConverter.toStdWString( numPlayers ) + L")" );
		}
	} catch( std::exception &e ) {
		std::wcerr << L"Error in GameManager::getPlayer(): " << e.what() << std::endl;
		return nullptr;
	} catch( std::wstring &e ) {
		std::wcerr << L"Error in GameManager::getPlayer(): " << e << std::endl;
		return nullptr;
	}
}

/**
 * Lets objects get random numbers using this object's generator.
 **/
std::minstd_rand::result_type GameManager::getRandomNumber() {
	return randomNumberGenerator();
}

/**
 * Lets objects see what the RNG was seeded with.
 **/
std::minstd_rand::result_type GameManager::getRandomSeed() {
	return randomSeed;
}

/**
 * Lets other objects get a pointer to the player start objects.
 * Arguments:
 * --- uint_fast8_t ps: The desired player start
 * Returns: A pointer to the desired player start object if it exists, nullptr otherwise.
 */
PlayerStart* GameManager::getStart( uint_fast8_t ps ) {
	try {
		if( debug ) {
			std::wcout << L"getStart() called" << std::endl;
		}
		
		return &playerStart.at( ps );
	} catch( std::exception &e ) {
		std::wcerr << L"Error in GameManager::getStart(): " << e.what() << std::endl;
		return nullptr;
	}
}

/**
 * Checks whether a pointer is equal to any of the values likely to represent null.
 * Arguments:
 * --- void* ptr: Some pointer.
 * Returns: Guess.
 */
bool GameManager::isNull( void* ptr ) {
	// cppcheck-suppress duplicateExpression
	return ( ptr == 0 and ptr == NULL and ptr == nullptr );
}

/**
 * Loads fonts. Calls loadMusicFont() and loadTipFont().
 */
void GameManager::loadFonts() {
	try {
		if( debug ) {
			std::wcout << L"loadFonts() called" << std::endl;
		}
		//TODO: When a font is loaded, use its size ratio to estimate what the next size to try should be
		fontFile = "";
		boost::filesystem::recursive_directory_iterator end;
		std::vector< boost::filesystem::path > fontFolders = system.getFontFolders(); // Flawfinder: ignore

		if( debug ) {
			std::wcout << L"fontFolders.size(): " << fontFolders.size() << std::endl;
		}

		//for( std::vector< boost::filesystem::path >::iterator o = fontFolders.begin(); o not_eq fontFolders.end(); o++ ) {
		for( decltype( fontFolders.size() ) o = 0; o < fontFolders.size(); ++o ) {
			//for( boost::filesystem::recursive_directory_iterator i( *o ); i not_eq end; ++i ) {
			if( debug ) {
				std::wcout << L"Looking for fonts in folder " << fontFolders.at( o ) << std::endl;
			}
			for( boost::filesystem::recursive_directory_iterator i( fontFolders.at( o ) ); i not_eq end; ++i ) {
				if( fontManager.canLoadFont( i->path() ) ) {
					if( debug ) {
						std::wcout << L"SUCCESS: " << i->path() << L" is a loadable font." << std::endl;
					}
					fontFile = stringConverter.toIrrlichtStringW( i->path().wstring() );
					break;
				}
			}
			if( fontFile not_eq "" ) { //We've found a loadable file
				break;
			}
		}
		
		heightTestString = L"()*^&v.ygj";
		
		//These were split off into separate functions because they are needed more often than loadFonts()
		loadTipFont();
		loadMusicFont();

		{ //Load loadingFont
			irr::core::dimension2d< uint_fast32_t > fontDimensions;
			uint_fast32_t size = windowSize.Width / 30; //30 found through experimentation: much larger and it takes too long to load fonts, much smaller and the font doesn't get as big as it should. Feel free to change at will if your computer's faster than mine.

			if( fontFile not_eq "" ) {
				do { //TODO: Repeatedly loading fonts like this seems like a waste of time. Is there a way we could load the font only once and still get this kind of size adjustment?
					loadingFont = fontManager.GetTtFont( driver, fontFile, size, antiAliasFonts );
					if( not isNull( loadingFont ) ) {
						fontDimensions = loadingFont->getDimension( stringConverter.toStdWString( loading ).c_str() ); //stringConverter.toWCharArray( loading ) );
						size -= 2;
					}
				} while( not isNull( loadingFont ) and ( fontDimensions.Width > ( windowSize.Width / sideDisplaySizeDenominator ) or fontDimensions.Height > ( windowSize.Height / 5 ) ) );

				size += 3;

				do {
					loadingFont = fontManager.GetTtFont( driver, fontFile, size, antiAliasFonts );
					if( not isNull( loadingFont ) ) {
						fontDimensions = loadingFont->getDimension( stringConverter.toStdWString( loading ).c_str() ); //stringConverter.toWCharArray( loading ) );
						size -= 1;
					}
				} while( not isNull( loadingFont ) and ( fontDimensions.Width > ( windowSize.Width / sideDisplaySizeDenominator ) or fontDimensions.Height > ( windowSize.Height / 5 ) ) );
			}

			if( fontFile == "" or isNull( loadingFont ) or size <= gui->getBuiltInFont()->getDimension( heightTestString.c_str() ).Height ) {
				loadingFont = gui->getBuiltInFont();
			}

			if( debug ) {
				std::wcout << L"loadingFont is loaded" << std::endl;
			}
		}


		{ //load textFont
			irr::core::dimension2d< uint_fast32_t > fontDimensions;
			if( fontFile not_eq "" ) {
				uint_fast32_t size = ( windowSize.Width / sideDisplaySizeDenominator ) / 6; //found through experimentation, adjust it however you like and see how many times the font gets loaded

				do {
					textFont = fontManager.GetTtFont( driver, fontFile, size, antiAliasFonts );

					if( not isNull( textFont ) ) {
						if( debug ) {
							std::wcout << L"Loaded textFont in loop (size " << size << L")" << std::endl;
						}
						fontDimensions = textFont->getDimension( L"Random seed: " );
						if( debug ) {
							std::wcout << L"fontDimensions set to " << fontDimensions.Width << L"x" << fontDimensions.Height << std::endl;
						}
						size -= 2;
					}
				} while( not isNull( textFont ) and ( fontDimensions.Width + viewportSize.Width > windowSize.Width ) );

				size += 3;

				do {
					textFont = fontManager.GetTtFont( driver, fontFile, size, antiAliasFonts );

					if( not isNull( textFont ) ) {
						if( debug ) {
							std::wcout << L"Loaded textFont in loop (size " << size << L")" << std::endl;
						}
						fontDimensions = textFont->getDimension( L"Random seed: " );
						if( debug ) {
							std::wcout << L"fontDimensions set to " << fontDimensions.Width << L"x" << fontDimensions.Height << std::endl;
						}
						size -= 1;
					}
				} while( not isNull( textFont ) and ( fontDimensions.Width + viewportSize.Width > windowSize.Width ) );
			}

			if( fontFile == "" or isNull( textFont ) or textFont->getDimension( heightTestString.c_str() ).Height <= gui->getBuiltInFont()->getDimension( heightTestString.c_str() ).Height ) {
				textFont = gui->getBuiltInFont();
			}

			if( debug ) {
				std::wcout << L"textFont is loaded" << std::endl;
			}
		}


		{ //Load clockFont
			irr::core::dimension2d< uint_fast32_t > fontDimensions;
			if( fontFile not_eq "" ) {
				uint_fast32_t size = ( windowSize.Width / sideDisplaySizeDenominator );

				do {
					clockFont = fontManager.GetTtFont( driver, fontFile, size, antiAliasFonts );
					if( not isNull( clockFont ) ) {
						fontDimensions = clockFont->getDimension( L"00:00:00" );
						size -= 2;
					}
				} while( not isNull( clockFont ) and ( fontDimensions.Width + viewportSize.Width > windowSize.Width  or fontDimensions.Height > ( windowSize.Height / 5 ) ) );

				size += 3;

				do {
					clockFont = fontManager.GetTtFont( driver, fontFile, size, antiAliasFonts );
					if( not isNull( clockFont ) ) {
						fontDimensions = clockFont->getDimension( L"00:00:00" );
						size -= 1;
					}
				} while( not isNull( clockFont ) and ( fontDimensions.Width + viewportSize.Width > windowSize.Width  or fontDimensions.Height > ( windowSize.Height / 5 ) ) );
			}

			if( fontFile == "" or isNull( clockFont ) or clockFont->getDimension( heightTestString.c_str() ).Height <= gui->getBuiltInFont()->getDimension( heightTestString.c_str() ).Height ) {
				clockFont = gui->getBuiltInFont();
			}

			if( debug ) {
				std::wcout << L"clockFont is loaded" << std::endl;
			}
		}


		{ //Load statsFont
			irr::core::dimension2d< uint_fast32_t > fontDimensions;
			
			if( fontFile not_eq "" ) {
				auto aboveStats = loadingFont->getDimension( loading.c_str() ).Height * 2 + std::max( tipFont->getDimension( proTipPrefix.c_str() ).Height, tipFont->getDimension( proTips.at( currentProTip ).c_str() ).Height );
			
				uint_fast32_t size = windowSize.Width / numPlayers / 3; //A quick approximation of the size we'll need the text to be. This is not exact because size is actually an indicator of font height, but numPlayers and hence the needed width are more likely to vary.
				uint_fast8_t builtInFontHeight = gui->getBuiltInFont()->getDimension( heightTestString.c_str() ).Height;
				if( size > builtInFontHeight ) { //If the text needs to be that small, go with the built-in font because it's readable at that size.
					do {
						size -= 2;
						statsFont = fontManager.GetTtFont( driver, fontFile, size, antiAliasFonts );
						if( not isNull( statsFont ) ) {
							irr::core::dimension2d< uint_fast32_t > tempDimensions;
							std::wstring tempString;
							if( numPlayers <= 10 ) {
								tempString = L"0.P";
							} else if( numPlayers <= 100 ) {
								tempString = L"00.P";
							} else {
								tempString = L"000.P";
							}
							tempString += stringConverter.toStdWString( numPlayers );
							
							tempDimensions = statsFont->getDimension( tempString.c_str() );
							fontDimensions = irr::core::dimension2d< uint_fast32_t >( tempDimensions.Width * numPlayers, tempDimensions.Height );
							tempDimensions = statsFont->getDimension( keysFoundPerPlayer.c_str() ); //stringConverter.toWCharArray( winnersLabel ) );
							fontDimensions = irr::core::dimension2d< uint_fast32_t >( fontDimensions.Width + tempDimensions.Width, std::max( fontDimensions.Height, tempDimensions.Height ) * 6 ); //6 = the number of rows of stats displayed on the loading screen
						}
					} while( size > builtInFontHeight and not isNull( statsFont ) and ( fontDimensions.Width >= windowSize.Width  or fontDimensions.Height + aboveStats >= windowSize.Height ) );

					size += 3;

					do {
						size -= 1;
						statsFont = fontManager.GetTtFont( driver, fontFile, size, antiAliasFonts );
						if( not isNull( statsFont ) ) {
							irr::core::dimension2d< uint_fast32_t > tempDimensions;
							std::wstring tempString;
							if( numPlayers <= 10 ) {
								tempString = L"0.P";
							} else if( numPlayers <= 100 ) {
								tempString = L"00.P";
							} else {
								tempString = L"000.P";
							}
							tempString += stringConverter.toStdWString( numPlayers );
							
							tempDimensions = statsFont->getDimension( tempString.c_str() );
							fontDimensions = irr::core::dimension2d< uint_fast32_t >( tempDimensions.Width * numPlayers, tempDimensions.Height );
							tempDimensions = statsFont->getDimension( keysFoundPerPlayer.c_str() ); //stringConverter.toWCharArray( winnersLabel ) );
							fontDimensions = irr::core::dimension2d< uint_fast32_t >( fontDimensions.Width + tempDimensions.Width, std::max( fontDimensions.Height, tempDimensions.Height ) * 6 ); //6 = the number of rows of stats displayed on the loading screen
						}
					} while( size > builtInFontHeight and not isNull( statsFont ) and ( fontDimensions.Width >= windowSize.Width  or fontDimensions.Height + aboveStats >= windowSize.Height ) );
				}
			}

			if( fontFile == "" or isNull( statsFont ) or statsFont->getDimension( heightTestString.c_str() ).Height <= gui->getBuiltInFont()->getDimension( heightTestString.c_str() ).Height ) {
				statsFont = gui->getBuiltInFont();
			}

			if( debug ) {
				std::wcout << L"statsFont is loaded" << std::endl;
			}
		}
		
		menuManager.setFont( clockFont );

		uint_fast32_t size = 12; //The GUI adjusts window sizes based on the text within them, so no need (hopefully) to use different font sizes for different window sizes. May affect readability on large or small screens, but it's better on large screens than the built-in font.
		gui->getSkin()->setFont( fontManager.GetTtFont( driver, fontFile, size, antiAliasFonts ) );
		
		if( debug ) {
			std::wcout << L"end of loadFonts()" << std::endl;
		}
	} catch( std::exception &e ) {
		std::wcerr << L"Error in GameManager::loadFonts(): " << e.what() << std::endl;
	}
}

/**
 * Loads the music font. Like loadTipFont() below, this guesses a good font size, then repeatedly adjusts the size and reloads the font until everything fits.
 */
void GameManager::loadMusicFont() {
	try {
		if( debug ) {
			std::wcout << L"loadMusicFont() called" << std::endl;
		}
		
		uint_fast32_t size = 0; //The height (I think) of the font to be loaded
		
		if( playMusic ) {
			if( fontFile not_eq "" ) {
				uint_fast32_t maxWidth = ( windowSize.Width / sideDisplaySizeDenominator );
				
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
				musicTagFont = gui->getBuiltInFont();
			}
		}
		
		if( debug ) {
			std::wcout << L"end of loadMusicFont()" << std::endl;
		}
	} catch( std::exception &e ) {
		std::wcerr << L"Error in GameManager::loadMusicFont(): " << e.what() << std::endl;
	}
}

/**
 * Loads the next song on the list.
 */
void GameManager::loadNextSong() {
	try {
		if( debug ) {
			std::wcout << L"loadNextSong() called" << std::endl;
		}

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
			throw( std::wstring( L"Unable to load music file: " ) + stringConverter.toStdWString( Mix_GetError() ) );
		} else {
			auto musicStatus = Mix_PlayMusic( music, 0 ); //The second argument tells how many times to *repeat* the music. -1 means infinite, 0 means don't repeat.

			if( musicStatus == -1 ) {
				throw( std::wstring( L"Unable to play music file: " ) + stringConverter.toStdWString( Mix_GetError() ) );
			} else {
				if( debug ) {
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

					if( debug ) {
						std::wcout << L"Now playing: " << stringConverter.toStdWString( musicTitle ) << L" by " << stringConverter.toStdWString( musicArtist ) << L" from album " << stringConverter.toStdWString( musicAlbum ) << std::endl;
					}
				}

				loadMusicFont();
			}

			Mix_VolumeMusic( musicVolume * MIX_MAX_VOLUME / 100 );
		}
		
		if( debug ) {
			std::wcout << L"end of loadNextSong()" << std::endl;
		}
	} catch( std::exception &e ) {
		std::wcerr << L"Error in GameManager::loadNextSong(): " << e.what() << std::endl;
	} catch( std::wstring &e ) {
		std::wcerr << L"Error in GameManager::loadNextSong(): " << e << std::endl;
	}
}

/**
 * Loads "pro tips" from file proTips.txt if that file exists.
 */
void GameManager::loadProTips() {
	try {
		if( debug ) {
			std::wcout << L"loadProTips() called" << std::endl;
		}
		
		proTips.clear(); //This line is unnecessary because loadProTips() is only called once, but I just feel safer clearing this anyway.
		boost::filesystem::path proTipsPath( boost::filesystem::current_path()/L"protips.txt" );

		if( exists( proTipsPath ) ) {
			if( not is_directory( proTipsPath ) ) {
				if( debug ) {
					std::wcout << L"Loading pro tips from file " << proTipsPath.wstring() << std::endl;
				}
				boost::filesystem::wifstream proTipsFile;
				proTipsFile.open( proTipsPath );
				
				if( proTipsFile.is_open() ) {
					std::wstring line;
					uint_fast16_t lineNum = 0;
					
					while( proTipsFile.good() ) {
						++lineNum;
						getline( proTipsFile, line );

						if( not line.empty() ) {
							proTips.push_back( stringConverter.toIrrlichtStringW( line ) ); //StringConverter converts between wstring (which is what getLine needs) and core::stringw (which is what Irrlicht needs)

							if( debug ) {
								std::wcout << line << std::endl;
							}
						}
					}
					
					proTipsFile.close();
					
					setRandomSeed( time( nullptr ) ); //srand( time( nullptr ) ); // Flawfinder: ignore
					random_shuffle( proTips.begin(), proTips.end() );
				} else {
					throw( std::wstring( L"Unable to open pro tips file even though it exists. Check its access permissions." ) );
				}
			} else {
				throw( std::wstring( L"Pro tips file is a directory. Cannot load pro tips." ) );
			}
		} else {
			throw( std::wstring( L"Pro tips file does not exist. Cannot load pro tips." ) );
		}
		
		if( debug ) {
			std::wcout << L"end of loadProTips()" << std::endl;
		}
	} catch( std::exception &e ) {
		std::wcerr << L"Error in GameManager::loadProTips(): " << e.what() << std::endl;
	} catch( std::wstring &e ) {
		std::wcerr << L"Error in GameManager::loadProTips(): " << e << std::endl;
	}
}

/**
 * Loads the tip font. Guesses a size that will work, keeps adjusting the size and reloading the font until everything fits.
 */
void GameManager::loadTipFont() {
	try {
		if( debug ) {
			std::wcout << L"loadTipFont() called" << std::endl;
		}
		
		uint_fast32_t size; //The height (I think) of the font to be loaded
		
		if( fontFile not_eq "" ) {
			uint_fast32_t maxWidth = windowSize.Width;
			
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
		
		if( debug ) {
			std::wcout << L"end of loadTipFont()" << std::endl;
		}
	} catch( std::exception &e ) {
		std::wcerr << L"Error in GameManager::loadTipFont(): " << e.what() << std::endl;
	}
}

/**
 * Finds all playable music files in the ./music folder and compiles them into a list. If ./music does not exist or is not a folder, it uses the parent path instead.
 */
void GameManager::makeMusicList() {
	try {
		if( debug ) {
			std::wcout << L"makeMusicList() called" << std::endl;
		}
		
		musicList.clear(); //The music list should be empty anyway, since makeMusicList() only gets called once, but just in case...

		if( debug ) {
			std::wcout << L"makeMusicList() called" << std::endl;
			decltype( Mix_GetNumMusicDecoders() ) numMusicDecoders = Mix_GetNumMusicDecoders();
			std::wcout << L"There are " << numMusicDecoders << L" music decoders available. They are:" << std::endl;

			for( decltype( numMusicDecoders ) decoder = 0; decoder < numMusicDecoders; ++decoder ) {
				std::wcout << decoder << L": " << Mix_GetMusicDecoder( decoder ) << std::endl;
			}
		}

		boost::filesystem::path musicPath( boost::filesystem::current_path()/L"music" );

		//Which is better: system_complete() or absolute()? On my computer they seem to do the same thing. Both are part of Boost Filesystem.
		musicPath = system_complete( musicPath );
		//musicPath = absolute( musicPath );

		if( debug ) {
			std::wcout << L"music path is absolute? " << musicPath.is_absolute() << std::endl;
		}

		while( ( not exists( musicPath ) or not is_directory( musicPath ) ) and musicPath.has_parent_path() ) {
			if( debug ) {
				std::wcout << L"Path " << musicPath.wstring() << L" does not exist or is not a directory. Checking parent path " << musicPath.parent_path().wstring() << std::endl;
			}

			musicPath = musicPath.parent_path();
		}

		if( exists( musicPath ) ) {
			boost::filesystem::recursive_directory_iterator end;

			for( boost::filesystem::recursive_directory_iterator i( musicPath ); i not_eq end; ++i ) {
				if( not is_directory( i->path() ) ) { //We've found a file
					//Attempts to load a file as music. If successful, unload the file and add it to musicList.
					//This way the game is certain to accept any file formats the music library can use.
					Mix_Music* temp = Mix_LoadMUS( i->path().c_str() );

					if( not isNull( temp ) ) {
						musicList.push_back( i->path() );
						Mix_FreeMusic( temp );
					}
				}
			}
		}

		if( musicList.size() > 0 ) {
			//Do we want music sorted or random?
			//sort( musicList.begin(), musicList.end() );
			setRandomSeed( time( nullptr ) ); //srand( time( nullptr ) ); // Flawfinder: ignore
			random_shuffle( musicList.begin(), musicList.end() );
			
			currentMusic = musicList.back();
		} else {
			std::wcerr << L"Could not find any music to play. Turning off playback." << std::endl;
			playMusic = false;
		}
		
		if( debug ) {
			std::wcout << L"end of makeMusicList()" << std::endl;
		}
	} catch( std::exception &e ) {
		std::wcerr << L"Error in GameManager::makeMusicList(): " << e.what() << std::endl;
	}
}

/**
 * Anything that should be done when a player moves, whichever axis they move on. Should only be called by movePlayerOnX() or movePlayerOnY().
 * Arguments:
 * --- p: The player to move
 */
 void GameManager::movePlayerCommon( uint_fast8_t p ) {
 	network.sendPlayerPos( p, player.at( p ).getX(), player.at( p ).getY() );
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
 */
void GameManager::movePlayerOnX( uint_fast8_t p, int_fast8_t direction ) {
	try {
		if( numPlayers > p and mazeManager.cols > 0 ) {
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
				throw L"Player "+ stringConverter.toStdWString( p ) + L"'s X (" + stringConverter.toStdWString( player.at( p ).getX() ) + L") is outside mazeManager's cols (" + stringConverter.toStdWString( mazeManager.cols ) + L").";
			} else if( player.at( p ).getY() >= mazeManager.rows ) {
				throw L"Player "+ stringConverter.toStdWString( p ) + L"'s Y (" + stringConverter.toStdWString( player.at( p ).getY() ) + L") is outside mazeManager's rows (" + stringConverter.toStdWString( mazeManager.rows ) + L").";
			}
		} else {
			std::wstring e = L"Player " + stringConverter.toStdWString( p ) + L" is greater than numPlayers (" + stringConverter.toStdWString( numPlayers ) + L")";
			throw e;
		}
	} catch( std::exception &e ) {
		std::wcerr << L"Error in GameManager::movePlayerOnX(): " << e.what() << std::endl;
	} catch( std::wstring &e ) {
		std::wcerr << L"Error in GameManager::movePlayerOnX(): " << e << std::endl;
	}
}

/**
 * If the maze allows it, moves a player one unit in the indicated direction along the Y axis.
 * Arguments:
 * --- p: the player to move
 * --- direction: a signed integer representing the direction to move.
 */
void GameManager::movePlayerOnY( uint_fast8_t p, int_fast8_t direction ) {
	try {
		if( numPlayers > p and mazeManager.rows > 0 ) {
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
				throw L"Player "+ stringConverter.toStdWString( p ) + L"'s X (" + stringConverter.toStdWString( player.at( p ).getX() ) + L") is outside mazeManager's cols (" + stringConverter.toStdWString( mazeManager.cols ) + L").";
			} else if( player.at( p ).getY() >= mazeManager.rows ) {
				throw L"Player "+ stringConverter.toStdWString( p ) + L"'s Y (" + stringConverter.toStdWString( player.at( p ).getY() ) + L") is outside mazeManager's rows (" + stringConverter.toStdWString( mazeManager.rows ) + L").";
			}

			movePlayerCommon( p );
		} else {
			std::wstring e = L"Player " + stringConverter.toStdWString( p ) + L" is greater than numPlayers (" + stringConverter.toStdWString( numPlayers ) + L")";
			throw e;
		}
	} catch( std::exception &e ) {
		std::wcerr << L"Error in GameManager::movePlayerOnY(): " << e.what() << std::endl;
	} catch( std::wstring &e ) {
		std::wcerr << L"Error in GameManager::movePlayerOnY(): " << e << std::endl;
	}
}

/**
 * Calls the other newMaze()
 */
void GameManager::newMaze() {
	try {
		if( debug ) {
			std::wcout << L"newMaze() called with no arguments" << std::endl;
		}
		
		newMaze( getRandomNumber() );
		
		if( debug ) {
			std::wcout << L"end of newMaze() with no arguments" << std::endl;
		}
	} catch( std::exception &e ) {
		std::wcerr << L"Error in GameManager::newMaze(): " << e.what() << std::endl;
	}
}

/**
 * Calls resetThings(), makes the maze manager load a maze from a file, then adjusts cellWidth and cellHeight.
 * Arguments:
 * --- boost::filesystem::path src: the file from which to load the maze.
 */
void GameManager::newMaze( boost::filesystem::path src ) {
	if( not mazeManager.loadFromFile( src ) ) {
		//If we get this far, it's an error. Probably a file not found. Fail gracefully by starting a new maze anyway.
		gui->addMessageBox( L"Could not use file", L"Unable to load maze from file. Generating a new maze." );
		newMaze( getRandomNumber() );
	}
}

/**
 * Calls resetThings(), makes the maze manager load a maze from a file, then adjusts cellWidth and cellHeight.
 * Arguments:
 * --- std::minstd_rand::result_type newRandomSeed: The random seed to use.
 */
void GameManager::newMaze( std::minstd_rand::result_type newRandomSeed ) {
	try {
		if( debug ) {
			std::wcout << L"newMaze() called with an argument" << std::endl;
		}
		
		resetThings();
		setRandomSeed( newRandomSeed );
		
		mazeManager.makeRandomLevel();
		
		cellWidth = ( viewportSize.Width ) / mazeManager.cols;
		cellHeight = ( viewportSize.Height ) / mazeManager.rows;
		for( decltype( numBots ) b = 0; b < numBots; ++b ) {
			bot.at( b ).setup( this, botsKnowSolution, botAlgorithm, botMovementDelay );
		}
		
		if( numLocks == 0 ) {
			for( decltype( numBots ) b = 0; b < numBots; ++b ) {
				bot.at( b ).allKeysFound(); //Lets all the bots know they don't need to search for keys
			}
		}
		
		setLoadingPercentage( 100 );
		
		if( debug ) {
			std::wcout << L"end of newMaze() with an argument" << std::endl;
		}
	} catch( std::exception &e ) {
		std::wcerr << L"Error in GameManager::newMaze(): " << e.what() << std::endl;
	}
}

/**
 * Called by Irrlicht every time an event (keyboard, mouse, controller, etc.) occurs.
 * Arguments:
 * --- const SEvent& event: an object representing the event that happened.
 * Returns: True if the event was handled by this function and does not involve any game controls (since the relationship of controls:actions can be many:many), false otherwise.
 */
//cppcheck-suppress unusedFunction
bool GameManager::OnEvent( const irr::SEvent& event ) {
	try {
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
				if( showingMenu ) {
					switch( event.MouseInput.Event ) {
						case irr::EMIE_MOUSE_MOVED: {
							menuManager.findHighlights( event.MouseInput.X, event.MouseInput.Y );
							break;
						}
						case irr::EMIE_LMOUSE_PRESSED_DOWN: {
							if( ( gui->getRootGUIElement()->getChildren().getSize() == 0 ) ) {
								menuManager.processSelection( this );
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
						windowSize.set( driver->getScreenSize().Width, driver->getScreenSize().Height );

						//TODO: Find a way to resize the window or set a minimum size
						/*if (!allowSmallSize) {
							bool sizeChanged = false;

							if (windowSize.Height < minHeight) {
								windowSize.Height = minHeight;
								sizeChanged = true;
							}

							if (windowSize.Width < minWidth) {
								windowSize.Width = minWidth;
								sizeChanged = true;
							}

							if (sizeChanged) {
								driver->OnResize( windowSize );
							}
						}*/

						viewportSize.set( windowSize.Width - ( windowSize.Width / sideDisplaySizeDenominator ), windowSize.Height - 1 );
						cellWidth = ( viewportSize.Width ) / mazeManager.cols;
						cellHeight = ( viewportSize.Height ) / mazeManager.rows;
						loadFonts();
						
						menuManager.setPositions( windowSize.Height );
						
						if( showBackgrounds ) {
							
							irr::scene::ICameraSceneNode* camera = backgroundSceneManager->getActiveCamera();
							if( not isNull( camera ) ) {
								camera->setAspectRatio( static_cast< decltype( camera->getAspectRatio() ) >( windowSize.Width ) / windowSize.Height );
							}
							
							//See setupBackgrounds() to find out what each background number means
							if( backgroundChosen == 2 and not isNull( backgroundTexture ) and backgroundTexture->getSize() not_eq windowSize ) {
								if( backgroundFilePath.size() > 0 ) {// not backgroundFilePath.empty() ) { Irrlicht 1.8+ has .empty() but Raspbian only has 1.7 in its repositories
									backgroundTexture = driver->getTexture( backgroundFilePath );
								}
								if( not isNull( backgroundTexture ) and backgroundTexture->getSize() not_eq windowSize ) {
									backgroundTexture = resizer.resize( backgroundTexture, windowSize.Width, windowSize.Height, driver );
								}
							} else if( backgroundChosen == 3 and backgroundTexture->getSize() not_eq windowSize ) {
								driver->removeTexture( backgroundTexture );
								backgroundTexture = driver->addRenderTargetTexture( windowSize );
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
							if( debug ) {
								std::wcout << L"File selected for loading. Folder: " << stringConverter.toStdWString( loadMazeDialog->getDirectoryName() ) << L"\tFile: " << loadMazeDialog->getFileName() << std::endl;
							}
							boost::filesystem::current_path( currentDirectory ); //Resets the actual current directory to currentDirectory, since the file chooser can change the current directory.

							newMaze( loadMazeDialog->getFileName() );
							loadMazeDialog = nullptr;
							return true;
						} else if( not isNull( saveMazeDialog ) and event.GUIEvent.Caller->getID() == saveMazeDialog->getID() ) {
							if( debug ) {
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
							if( debug ) {
								std::wcout << L"Folder selected." << std::endl;
							}
							return true;
						} else if( not isNull( saveMazeDialog ) and event.GUIEvent.Caller->getID() == saveMazeDialog->getID() ) {
							if( debug ) {
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
			default:
				break;
		}

	} catch( std::exception &e ) {
		std::wcerr << L"Error in GameManager::OnEvent(): " << e.what() << std::endl;
	}

	return false;
}

/**
 * Randomly selects a logo and draws it on screen. Should NOT be called by drawAll().
 */
 void GameManager::pickLogo() {
	try {
		if( debug ) {
			std::wcout << L"pickLogo() called" << std::endl;
		}
		
		std::vector< boost::filesystem::path > logoList;

		boost::filesystem::path logoPath( boost::filesystem::current_path()/L"images/logos" );

		//Which is better: system_complete() or absolute()? On my computer they seem to do the same thing. Both are part of Boost Filesystem.
		logoPath = system_complete( logoPath );
		//logoPath = absolute( logoPath );

		while( ( not exists( logoPath ) or not is_directory( logoPath ) ) and logoPath.has_parent_path() ) {
			if( debug ) {
				std::wcout << L"Path " << logoPath.wstring() << L" does not exist or is not a directory. Checking parent path " << logoPath.parent_path().wstring() << std::endl;
			}

			logoPath = logoPath.parent_path();
		}

		if( exists( logoPath ) ) {
			boost::filesystem::recursive_directory_iterator end;

			for( boost::filesystem::recursive_directory_iterator i( logoPath ); i not_eq end; ++i ) {
				if( not is_directory( i->path() ) ) { //We've found a file
					if( debug ) {
						std::wcout << i->path().wstring() << std::endl;
					}

					//Asks Irrlicht if the file is loadable. This way the game is certain to accept any file formats the library can use.
					for( decltype( driver->getImageLoaderCount() ) loaderNum = 0; loaderNum < driver->getImageLoaderCount(); ++loaderNum ) { //Irrlicht uses a different image loader for each file type. Loop through them all, ask each if it can load the file.

						irr::video::IImageLoader* loader = driver->getImageLoader( loaderNum );
						irr::io::IFileSystem* fileSystem = device->getFileSystem();
						irr::io::path filePath = stringConverter.toIrrlichtStringW( i->path().wstring() );

						//if( loader->isALoadableFileExtension( filePath ) ) { //Commenting this out because extensions don't always reflect the file's contents. Uncomment it for a minor speed improvement since not all files would need to be opened.
							irr::io::IReadFile* file = fileSystem->createAndOpenFile( filePath );
							if( loader->isALoadableFileFormat( file ) ) {
								logoList.push_back( i->path() );
								file->drop();
								break;
							}
							file->drop();
						/*} else {
							if( debug ) {
								bool isLoadableExtension = loader->isALoadableFileExtension( filePath );
								bool isLoadableFormat = loader->isALoadableFileFormat( fileSystem->createAndOpenFile( filePath ) );
								std::wcout << "is loadable extension? " << isLoadableExtension << " is loadable format? " << isLoadableFormat << std::endl;
							}
						}*/
					}
				}
			}
		}

		if( logoList.size() > 0 ) {
			std::vector< boost::filesystem::path >::iterator newEnd = std::unique( logoList.begin(), logoList.end() ); //unique "removes all but the first element from every consecutive group of equivalent elements in the range [first,last)." (source: http://www.cplusplus.com/reference/algorithm/unique/ )
			logoList.resize( std::distance( logoList.begin(), newEnd ) );

			//Pick a random logo and load it
			auto logoChosen = getRandomNumber() % logoList.size();
			if( debug ) {
				std::wcout << L"Logo chosen: #" << logoChosen;
				std::wcout << L" " << logoList.at( logoChosen ).wstring() << std::endl;
			}
			irr::io::path logoFilePath = stringConverter.toIrrlichtStringW( logoList.at( logoChosen ).wstring() );
			logoTexture = driver->getTexture( logoFilePath );
			if( isNull( logoTexture ) ) {
				std::wstring error = L"Cannot load logo texture, even though Irrlicht said it was loadable?!?";
				throw error;
			}

			drawLogo();
		} else {
			std::wcerr << L"Could not find any logo images." << std::endl;
		}
		
		if( debug ) {
			std::wcout << L"end of pickLogo()" << std::endl;
		}
	} catch( std::wstring error ) {
		std::wcerr << L"Error in drawLogo(): " << error << std::endl;
	} catch ( std::exception &error ) {
		std::wcerr << L"Error in drawLogo(): " << error.what() << std::endl;
	}
 }

/**
 * Created because so many preferences are booleans
 * Arguments:
 * --- std::wstring choice: a string which should be either "true" or "false"
 * Returns: A Boolean indicating whether choice is closer to "true" or to "false"
 */
bool GameManager::prefIsTrue( std::wstring choice ) {
	std::vector< std::wstring > possibleChoices = { L"true", L"false" };
	auto choiceNum = spellChecker.indexOfClosestString( choice, possibleChoices );
	return( choiceNum == 0 );
}

/**
* Should be called only by run().
* Arguments:
* None.
*/
void GameManager::processControls() {
	try {
		for( decltype( controls.size() ) k = 0; k < controls.size(); ++k ) {
			if( controls.at( k ).activated ) {
				if ( controls.at( k ).getMouseEvent() == irr::EMOUSE_INPUT_EVENT::EMIE_MOUSE_WHEEL ) { //There's no event for when the wheel stops moving, so we're manually deactivating those controls
					controls.at( k ).activated = false;
				}
				
				switch( controls.at( k ).getAction() ) {
					case ControlMapping::ACTION_MENU_ACTIVATE: {
						if( not showingMenu ) {
							showingMenu = true;
						} else {
							menuManager.processSelection( this );
						}
						break;
					}
					case ControlMapping::ACTION_MENU_UP: {
						menuManager.scrollSelection( true );
						break;
					}
					case ControlMapping::ACTION_MENU_DOWN: {
						menuManager.scrollSelection( false );
						break;
					}
					case ControlMapping::ACTION_SCREENSHOT: {
						takeScreenShot();
						break;
					}
					case ControlMapping::ACTION_VOLUME_UP: {
						musicVolume += 5;
						
						if( musicVolume > 100 ) {
							musicVolume = 100;
						}
						
						Mix_VolumeMusic( musicVolume * MIX_MAX_VOLUME / 100 );
						break;
					}
					case ControlMapping::ACTION_VOLUME_DOWN: {
						if( musicVolume >= 5 ) {
							musicVolume -= 5;
						} else {
							musicVolume = 0;
						}
						
						Mix_VolumeMusic( musicVolume * MIX_MAX_VOLUME / 100 );
						break;
					}
					default: { //Handle player controls
						bool ignoreKey = false;
						
						if( showingMenu ) { //Don't move the players if the game is paused.
							ignoreKey = true;
						}
						
						for( decltype( numBots ) b = 0; not ignoreKey and b < numBots; ++b ) { //Ignore controls that affect bots
							if( controls.at( k ).getPlayer() == bot.at( b ).getPlayer() ) {
								ignoreKey = true;
							}
						}

						if( not ignoreKey ) {
							switch( controls.at( k ).getAction() ) {
								case ControlMapping::ACTION_PLAYER_UP: {
									movePlayerOnY( controls.at( k ).getPlayer(), -1);
									break;
								}
								case ControlMapping::ACTION_PLAYER_DOWN: {
									movePlayerOnY( controls.at( k ).getPlayer(), 1);
									break;
								}
								case ControlMapping::ACTION_PLAYER_RIGHT: {
									movePlayerOnX( controls.at( k ).getPlayer(), 1);
									break;
								}
								case ControlMapping::ACTION_PLAYER_LEFT: {
									movePlayerOnX( controls.at( k ).getPlayer(), -1);
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
		std::wcerr << L"Error in GameManager::processControls(): " << e.what() << std::endl;
	}
}

/**
 * Reads preferences from prefs.cfg. Sets defaults for any preference not found in the file. If the file does not exist, creates it.
 */
void GameManager::readPrefs() {
	try {
		if( debug ) {
			std::wcout << L"readPrefs() called" << std::endl;
		}

		//Set default prefs, in case we can't get them from the file
		showBackgrounds = true;
		fullscreen = false;
		bitsPerPixel = 8;
		vsync = true;
		driverType = irr::video::EDT_OPENGL;
		windowSize = irr::core::dimension2d< decltype( windowSize.Height ) >( minWidth, minHeight );
		allowSmallSize = false;
		playMusic = true;
		numBots = 0;
		numPlayers = 1;
		markTrails = false;
		musicVolume = 50;
		network.setPort( 61187 );
		isServer = true;
		botsKnowSolution = false;
		botAlgorithm = AI::DEPTH_FIRST_SEARCH;
		botMovementDelay = 300;
		mazeManager.hideUnseen = false;
		#if defined DEBUG
			debug = true;
		#else
			debug = false;
		#endif
		
		std::vector< std::wstring > possiblePrefs = { L"bots' solving algorithm", L"volume", L"number of bots", L"show backgrounds",
									L"fullscreen", L"mark player trails", L"debug", L"bits per pixel", L"wait for vertical sync", L"driver type", L"number of players",
									L"window size", L"play music", L"network port", L"always server", L"bots know the solution", L"bot movement delay", L"hide unseen maze areas" };
		auto prefsNotFound = possiblePrefs;
		
		std::vector< boost::filesystem::path > configFolders = system.getConfigFolders(); // Flawfinder: ignore
		bool prefsFileFound = false;
		
		for( auto it = configFolders.begin(); it not_eq configFolders.end(); ++it ) {
			boost::filesystem::path prefsPath( *it/L"prefs.cfg" );
			
			if( ( exists( prefsPath ) and not is_directory( prefsPath ) ) or not exists( prefsPath ) ) {
				if( debug ) {
					std::wcout << L"Loading preferences from file " << prefsPath.wstring() << std::endl;
				}
				prefsFileFound = true;
				boost::filesystem::wfstream prefsFile;
				prefsFile.open( prefsPath, boost::filesystem::wfstream::in );
				
				if( prefsFile.is_open() ) {
					std::wstring line;
					uintmax_t lineNum = 0; //This used to be a uint_fast8_t, which should be good enough. However, when dealing with user input (such as a file), we don't want to make assumptions.
					
					while( prefsFile.good() ) {
						++lineNum;
						getline( prefsFile, line );
						line = line.substr( 0, line.find( L"//" ) ); //Filters out comments
						boost::algorithm::trim_all( line ); //Removes trailing and leading spaces, and spaces in the middle are reduced to one character
						boost::algorithm::to_lower( line );
						if( debug ) {
							std::wcout << L"Line " << lineNum << L": \"" << line << "\"" << std::endl;
						}
						
						
						if( not line.empty() ) {
							try {
								std::wstring preference = boost::algorithm::trim_copy( line.substr( 0, line.find( L'\t' ) ) );
								std::wstring choice = boost::algorithm::trim_copy( line.substr( line.find( L'\t' ) ) );
								
								if( debug ) {
									std::wcout << L"Preference \"" << preference << L"\" choice \"" << choice << L"\""<< std::endl;
								}
								
								//preference = possiblePrefs.at( spellChecker.indexOfClosestString( preference, possiblePrefs ) );
								auto preferenceNum = spellChecker.indexOfClosestString( preference, possiblePrefs );
								
								if( debug ) {
									std::wcout << L"Preference after spellchecking \"" << preference << std::endl;
								}
								
								{
									auto toRemove = std::find( prefsNotFound.begin(), prefsNotFound.end(), possiblePrefs.at( preferenceNum ) );
									if( toRemove not_eq prefsNotFound.end() ) {
										prefsNotFound.erase( toRemove );
									}
								}
								
								switch( preferenceNum ) {
									case 0: { //L"bots' solving algorithm"
										AI temp;
										temp.setup( this, false, AI::ALGORITHM_DO_NOT_USE, 0 );
										botAlgorithm = temp.algorithmFromString( choice );
										break;
									}
									
									case 1: { //L"volume"
										try {
											uint_fast16_t choiceAsInt = boost::lexical_cast< uint_fast16_t >( choice );
											
											if( choiceAsInt <= 100 ) {
												musicVolume = choiceAsInt;
												Mix_VolumeMusic( musicVolume * MIX_MAX_VOLUME / 100 );
												if( debug ) {
													std::wcout << L"Volume should be " << choiceAsInt << "%" << std::endl;
													std::wcout << L"Volume is really " << 100 * Mix_VolumeMusic( -1 ) / MIX_MAX_VOLUME << "%" << std::endl;
												}
											} else {
												std::wcerr << L"Warning: Volume greater than 100%: " << choiceAsInt << std::endl;
												Mix_VolumeMusic( MIX_MAX_VOLUME );
												musicVolume = 100;
											}
										} catch( boost::bad_lexical_cast &e ) {
											std::wcerr << L"Error reading volume preference (is it not a number?) on line " << lineNum << L": " << e.what() << std::endl;
										}
										break;
									}
									
									case 2: { //L"number of bots"
										try {
											decltype( numBots ) choiceAsInt = boost::lexical_cast< unsigned short int >( choice ); //uint_fast8_t is typedef'd as a kind of char apparently, at least on my raspberry pi, and Boost lexical_cast() won't convert from wchar_t to char.
											
											if( choiceAsInt <= numPlayers ) {
												numBots = choiceAsInt;
												if( debug ) {
													std::wcout << L"Number of bots is " << choiceAsInt << std::endl;
												}
											} else {
												std::wcerr << L"Warning: Number of bots not less than or equal to number of players (number of players may not have been read yet): " << choiceAsInt << std::endl;
												numBots = choiceAsInt;
											}
										} catch( boost::bad_lexical_cast &e ) {
											std::wcerr << L"Error reading number of bots preference (is it not a number?) on line " << lineNum << L": " << e.what() << std::endl;
										}
										break;
									}
									
									case 3: { //L"show backgrounds"
										showBackgrounds = prefIsTrue( choice );
										break;
									}
									
									case 4: { //L"fullscreen"
										fullscreen = prefIsTrue( choice );
										break;
									}
									
									case 5: { //L"mark player trails"
										markTrails = prefIsTrue( choice );
										break;
									}
									
									case 6: { //L"debug"
										#ifndef DEBUG
											debug = prefIsTrue( choice );
										#endif
										
										if( debug ) {
											std::wcout << L"Debug is ON" << std::endl;
										}
										break;
									}
									
									case 7: { //L"bits per pixel"
										try {
											uint_fast16_t choiceAsInt = boost::lexical_cast< uint_fast16_t >( choice );
											
											if( choiceAsInt <= 16 ) {
												bitsPerPixel = choiceAsInt;
												if( debug ) {
													std::wcout << L"Bits per pixel is " << choiceAsInt << std::endl;
												}
											} else {
												std::wcerr << L"Warning: Bits per pixel not less than or equal to 16: " << choiceAsInt << std::endl;
												bitsPerPixel = choiceAsInt;
											}
										} catch( boost::bad_lexical_cast &e ) {
											std::wcerr << L"Error reading bitsPerPixel preference (is it not a number?) on line " << lineNum << L": " << e.what() << std::endl;
										}
										break;
									}
									
									case 8: { //L"wait for vertical sync"
										vsync = prefIsTrue( choice );
										break;
									}
									
									case 9: { //L"driver type"
										std::vector< std::wstring > possibleChoices = { L"opengl", L"direct3d9", L"direct3d8", L"burning's video", L"software", L"null" };
										choice = possibleChoices.at( spellChecker.indexOfClosestString( choice, possibleChoices ) );
										
										if( choice == possibleChoices.at( 0 ) ) { //L"opengl"
											driverType = irr::video::EDT_OPENGL;
										} else if( choice == possibleChoices.at( 1 ) ) { //L"direct3d9"
											driverType = irr::video::EDT_DIRECT3D9;
										} else if( choice == possibleChoices.at( 2 ) ) { //L"direct3d8"
											driverType = irr::video::EDT_DIRECT3D8;
										} else if( choice == possibleChoices.at( 3 ) ) { //L"burning's video"
											driverType = irr::video::EDT_BURNINGSVIDEO;
										} else if( choice == possibleChoices.at( 4 ) ) { //L"software"
											driverType = irr::video::EDT_SOFTWARE;
										} else if( choice == possibleChoices.at( 5 ) ) { //L"null"
											driverType = irr::video::EDT_NULL;
										}
										
										if( debug ) {
											std::wcout << L"Selected driver type is " << choice << std::endl;
										}
										
										break;
									}
									
									case 10: { //L"number of players"
										try {
											decltype( numPlayers ) choiceAsInt = boost::lexical_cast< unsigned short int >( choice ); //uint_fast8_t is typedef'd as a kind of char apparently, at least on my raspberry pi, and Boost lexical_cast() won't convert from wchar_t to char.
											
											if( choiceAsInt <= 4 and choiceAsInt > 0 ) {
												numPlayers = choiceAsInt;
												if( debug ) {
													std::wcout << L"Number of players is " << choiceAsInt << std::endl;
												}
											} else if( choiceAsInt > 4 ) {
												std::wcerr << L"Warning: Number of players not less than or equal to 4: " << choiceAsInt << std::endl;
												numPlayers = choiceAsInt;
											} else {
												std::wcerr << L"Warning: Number of players is zero or not a number: " << choiceAsInt << L". Setting number of players to default." << std::endl;
											}
										} catch( boost::bad_lexical_cast &e ) {
											std::wcerr << L"Error reading number of players preference (is it not a number?) on line " << lineNum << L": " << e.what() << std::endl;
										}
										break;
									}
									
									case 11: { //L"window size"
										size_t locationOfX = choice.find( L"x" );
										std::wstring width = choice.substr( 0, locationOfX );
										std::wstring height = choice.substr( locationOfX + 1 );
										if( debug ) {
											std::wcout << L"Window size: " << width << L"x" << height << std::endl;
										}
										
										decltype( windowSize.Width ) widthAsInt = boost::lexical_cast< decltype( windowSize.Width ) >( width );
										decltype( windowSize.Height ) heightAsInt = boost::lexical_cast< decltype( windowSize.Height ) >( height );
										
										if( widthAsInt < 160 or heightAsInt < 240 ) {
											std::wcerr << L"Error reading window size: Width and/or height are really really tiny. Sorry but you'll have to recompile the game yourself if you want a window that small." << std::endl;
										} else if( widthAsInt == 160 and heightAsInt == 240 ) {
											std::wcout << L"Rock on, CGA graphics. Rock on." << std::endl;
											windowSize = irr::core::dimension2d< decltype( windowSize.Height ) >( widthAsInt, heightAsInt );
										} else {
											windowSize = irr::core::dimension2d< decltype( windowSize.Height ) >( widthAsInt, heightAsInt );
										}
										break;
									}
									
									case 12: { //L"play music"
										playMusic = prefIsTrue( choice );
										break;
									}
									
									case 13: { //L"network port"
										if( debug ) {
											std::wcout << L"Network port: " << choice << std::endl;
										}
										
										try {
											uint_fast16_t choiceAsInt = boost::lexical_cast< uint_fast16_t >( choice );
											network.setPort( choiceAsInt );
										} catch( boost::bad_lexical_cast &e ) {
											std::wcerr << L"Error reading network port (is it not a number?) on line " << lineNum << L": " << e.what() << std::endl;
										}
										break;
									}
									
									case 14: { //L"always server"
										isServer = prefIsTrue( choice );
										break;
									}
									
									case 15: { //L"bots know the solution"
										botsKnowSolution = prefIsTrue( choice );
										break;
									}
									
									case 16: { //L"bot movement delay"
										try {
											botMovementDelay = boost::lexical_cast< uint_fast16_t >( choice );
										} catch( boost::bad_lexical_cast &e ) {
											std::wcerr << L"Error reading botMovementDelay preference (is it not a number?) on line " << lineNum << L": " << e.what() << std::endl;
										}
										break;
									}
									
									case 17: { //L"hide unseen maze areas"
										mazeManager.hideUnseen = prefIsTrue( choice );
										break;
									}
								}
								
							} catch ( std::exception &e ) {
								std::wcerr << L"Error: " << e.what() << L". Does line " << lineNum << L" not have a tab character separating preference and value? The line says " << line << std::endl;
							}
						}
					}
					
					prefsFile.close();
					
					if( debug ) {
						botMovementDelay = 0;
					}
				}
				
				if( not device->isDriverSupported( driverType ) ) {
					std::wcerr << L"Warning: Chosen driver type is not supported on this system. Auto-picking a new type.";
					
					driverType = irr::video::EDT_NULL;
					//Driver types included in the E_DRIVER_TYPE enum may not actually be supported; it depends on how Irrlicht is compiled.
					for( auto i = ( uint_fast8_t ) irr::video::EDT_COUNT; i not_eq ( uint_fast8_t ) irr::video::EDT_NULL; --i ) {
						if( device->isDriverSupported( ( irr::video::E_DRIVER_TYPE ) i ) ) {
							driverType = ( irr::video::E_DRIVER_TYPE ) i;
							break;
						}
					}
					
					//Note: Just because the library supports a driver type doesn't mean we can actually use it. A loop similar to the above is used in the GameManager constructor where we call createDevice(). Therefore, the final driverType may not be what is set here.
					if( driverType == irr::video::EDT_NULL ) {
						std::wcerr << L"Error: No graphical output driver types are available. Using NULL type!! Also enabling debug." << std::endl;
						debug = true;
					}
				}
				
				prefsFile.open( prefsPath, boost::filesystem::wfstream::out bitor boost::filesystem::wfstream::app );
				
				if( prefsFile.is_open() ) {
					prefsFile << std::boolalpha;
					uintmax_t lineNum = 0;
					
					while( prefsFile.good() and not prefsNotFound.empty() ) {
						prefsFile << std::endl;
						
						auto preferenceNum = spellChecker.indexOfClosestString( prefsNotFound.at( 0 ), possiblePrefs );
						
						prefsFile << prefsNotFound.at( 0 ) << L"\t";
						prefsNotFound.erase( prefsNotFound.begin() );
						
						switch( preferenceNum ) {
							case 0: { //L"bots' solving algorithm"
								AI temp;
								temp.setup( this, false, AI::ALGORITHM_DO_NOT_USE, 0 );
								prefsFile << temp.stringFromAlgorithm( botAlgorithm );
								break;
							}
							case 1: { //L"volume"
								prefsFile << musicVolume;
								break;
							}
							case 2: { //L"number of bots"
								prefsFile << numBots;
								break;
							}
							case 3: { //L"show backgrounds"
								prefsFile << showBackgrounds;
								break;
							}
							case 4: { //L"fullscreen"
								prefsFile << fullscreen;
								break;
							}
							case 5: { //L"mark player trails"
								prefsFile << markTrails;
								break;
							}
							case 6: { //L"debug"
								prefsFile << debug;
								break;
							}
							case 7: { //L"bits per pixel"
								prefsFile << bitsPerPixel;
								break;
							}
							case 8: { //L"wait for vertical sync"
								prefsFile << vsync;
								break;
							}
							case 9: { //L"driver type"
								switch( driverType ) { //{ L"opengl", L"direct3d9", L"direct3d8", L"burning's video", L"software", L"null" };
									case irr::video::EDT_OPENGL: {
										prefsFile << L"opengl";
										break;
									}
									case irr::video::EDT_DIRECT3D9: {
										prefsFile << L"direct3d9";
										break;
									}
									case irr::video::EDT_DIRECT3D8: {
										prefsFile << L"direct3d8";
										break;
									}
									case irr::video::EDT_BURNINGSVIDEO: {
										prefsFile << L"burning's video";
										break;
									}
									case irr::video::EDT_SOFTWARE: {
										prefsFile << L"software";
										break;
									}
									case irr::video::EDT_NULL: {
										prefsFile << L"null";
										break;
									}
									case irr::video::EDT_COUNT: {
										break;
									}
								}
								break;
							}
							case 10: { //L"number of players"
								prefsFile << numPlayers;
								break;
							}
							case 11: { //L"window size"
								prefsFile << windowSize.Width << L"x" << windowSize.Height;
								break;
							}
							case 12: { //L"play music"
								prefsFile << playMusic;
								break;
							}
							case 13: { //L"network port"
								prefsFile << network.getPort();
								break;
							}
							case 14: { //L"always server"
								prefsFile << isServer;
								break;
							}
							case 15: { //L"bots know the solution"
								prefsFile << botsKnowSolution;
								break;
							}
							case 16: { //L"bot movement delay"
								prefsFile << botMovementDelay;
								break;
							}
							case 17: { //L"hide unseen maze areas"
								prefsFile << mazeManager.hideUnseen;
								break;
							}
						}
						
						++lineNum;
					}
					
					prefsFile.close();
				}
			}
		}

		if( not isServer ) {
			std::wcout << L"(S)erver or (c)lient? " << std::endl;
			wchar_t a;
			std::wcin >> a;
			isServer = ( a == L's' or a == L'S' );
			myPlayer = 0;
		}
		
		if( not prefsFileFound ) {
			throw( L"prefs.cfg does not exist or is not readable in any of the folders that were searched." );
		}
	} catch( std::exception &e ) {
		std::wcerr << L"Error in GameManager::readPrefs(): " << e.what() << std::endl;
	} catch( std::wstring &e ) {
		std::wcerr << L"Error in GameManager::readPrefs(): " << e << std::endl;
	}
	
	if( debug ) {
		std::wcout << L"end of readPrefs()" << std::endl;
	}
}

/**
 * Resets miscellaneous stuff between mazes.
 */
void GameManager::resetThings() {
	try {
		if( debug ) {
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

		for( decltype( numPlayers ) p = 0; p < numPlayers; ++p ) {
			playerStart.at( p ).reset();
			player.at( p ).reset();
		}

		//Calculate players' total scores
		for( decltype( numPlayers ) w = 0; w < winnersLoadingScreen.size(); ++w ) { //changed decltype( winnersLoadingScreen.size() ) to decltype( numPlayers ) because winnersLoadingScreen.size() can never exceed numPlayers but can be stored in a needlessly slow integer type.
			decltype( player.at( winnersLoadingScreen.at( w ) ).getScoreTotal() ) score = 0;
			decltype( player.at( winnersLoadingScreen.at( w ) ).getScoreTotal() ) additiveMultiplier = 10; //So the scores don't get too negative, numbers that add to the score get multiplied by a magic number.
			decltype( player.at( winnersLoadingScreen.at( w ) ).getScoreTotal() ) subtractiveDivisor = 10; //Likewise, numbers that subtract from the score get divided by a magic number.
			
			if( debug ) {
				std::wcout << L"Setting player " << winnersLoadingScreen.at( w ) << L"'s score to ";
			}
			{
				decltype( score ) temp = ( winnersLoadingScreen.size() - w ) * additiveMultiplier;
				score += temp;
				if( debug ) {
					std::wcout << temp;
				}
			}
			{
				decltype( score ) temp = ( player.at( winnersLoadingScreen.at( w ) ).stepsTakenLastMaze ) / subtractiveDivisor;
				score -= temp;
				if( debug ) {
					std::wcout << L" - " << temp;
				}
			}
			{
				decltype( score ) temp = player.at( winnersLoadingScreen.at( w ) ).timeTakenLastMaze / 1000 / subtractiveDivisor; //The 1000 is for converting the time to seconds
				score -= temp;
				if( debug ) {
					std::wcout << L" - " << temp;
				}
			}
			{
				decltype( score ) temp = player.at( winnersLoadingScreen.at( w ) ).keysCollectedLastMaze * additiveMultiplier;
				score += temp;
				if( debug ) {
					std::wcout << L" + " << temp << L" for a total of " << score << std::endl;
				}
			}
			
			player.at( winnersLoadingScreen.at( w ) ).setScore( score );
		}

		for( decltype( numBots ) b = 0; b < numBots; ++b ) {
			bot.at( b ).reset();
		}

		for( decltype( stuff.size() ) i = 0; i < stuff.size(); ++i ) {
			stuff.at( i ).loadTexture( device );
		}

		for( decltype( mazeManager.cols ) x = 0; x < mazeManager.cols; ++x ) {
			for( decltype( mazeManager.rows ) y = 0; y < mazeManager.rows; ++y ) {
				mazeManager.maze[ x ][ y ].visited = false;
			}
		}

		won = false;
		backgroundSceneManager->clear();

		if( showBackgrounds ) {
			setupBackground();
		}
		timer->setTime( 0 );
		timer->start();
		currentProTip = ( currentProTip + 1 ) % proTips.size();
		loadTipFont();
		startLoadingScreen();
		lastTimeControlsProcessed = timer->getTime();
	} catch( std::exception &e ) {
		std::wcerr << L"Error in GameManager::resetThings(): " << e.what() << std::endl;
	}
	
	if( debug ) {
		std::wcout << L"end of resetThings()" << std::endl;
	}
}

/**
 * The game's main loop. Should only be called by main() in main.cpp
 * Returns: EXIT_SUCCESS if the game exits normally, EXIT_FAILURE if an exception is caught.
 */
uint_fast8_t GameManager::run( std::wstring fileToLoad ) {
	try {
		if( debug ) {
			std::wcout << L"run() called" << std::endl;
		}
		
		bool firstFileLoaded; //Indicates whether fileToLoad was loaded at the start of the program. If a file to load is specified on the command line, this will only be false until that file gets loaded and then will remain true for the rest of the program's run. If no file is specified, this will always be true.
		if( not fileToLoad.empty() ) {
			firstFileLoaded = false;
		} else {
			firstFileLoaded = true; //No file specified, so no need to try to load it.
		}
		
		while( device->run() and not donePlaying ) {
			if( firstFileLoaded ) {
				newMaze();
			} else {
				newMaze( fileToLoad );
				firstFileLoaded = true;
			}
			
			haveShownLogo = true; //This should only ever be false at the start of the program.

			while( device->run() and not won and not donePlaying ) {

				if( showingLoadingScreen and ( timer->getRealTime() > timeStartedLoading + loadingDelay ) ) {
					showingLoadingScreen = false;
				}

				if( driver->getFPS() > 60 ) {
					/*These next 3 lines are for limiting processor usage. I really
					 *doubt they're all needed, so comment one or another if you
					 *experience slowness. Two are provided by Irrlicht and the other by SDL.*/
					device->yield();
					//device->sleep( 17 ); //17 = 1/60 of a second, rounded up, in milliseconds. My monitor refreshes at 60 Hz.
					//SDL_Delay( 17 );
				}

				if( driver->getScreenSize() not_eq windowSize ) { //If the window has been resized. Only here until Irrlicht implements proper window resize events.
					irr::SEvent temp;
					temp.EventType = irr::EET_USER_EVENT;
					temp.UserEvent.UserData1 = USER_EVENT_WINDOW_RESIZE;
					device->postEventFromUser( temp );
				}

				if( playMusic and not Mix_PlayingMusic() ) { //If we've finished playing a song.
					loadNextSong();
				}
				
				{
					auto time = timer->getRealTime(); //getRealTime() works even if the timer is stopped, as it is when the game is paused.
					if( time >= lastTimeControlsProcessed + controlProcessDelay or time < lastTimeControlsProcessed ) {
						processControls();
						lastTimeControlsProcessed = time;
					}
				}
				
				
				if( ( not showingLoadingScreen and device->isWindowActive() ) or debug ) {
					//It's the bots' turn to move now.
					if( not ( showingMenu or showingLoadingScreen ) and numBots > 0 ) {
						for( decltype( numBots ) i = 0; i < numBots; ++i ) {
							if( not bot.at( i ).atGoal() and ( allHumansAtGoal() or bot.at( i ).doneWaiting() ) ) {
								bot.at( i ).move();
							}
						}
					}
					
					device->getCursorControl()->setVisible( showingMenu or showingLoadingScreen or debug );
					drawAll();
					
					//Check if any of the players have landed on a collectable item
					for( decltype( numPlayers ) p = 0; p < numPlayers; ++p ) {
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

											for( decltype( numBots ) b = 0; b < numBots; ++b ) {
												bot.at( b ).allKeysFound();
											}
										} else {
											for( decltype( numBots ) b = 0; b < numBots; ++b ) {
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


					for( decltype( numPlayers ) p = 0; p < numPlayers; ++p ) {
						if( ( player.at( p ).getX() == goal.getX() ) and player.at( p ).getY() == goal.getY() ) { //Make a list of who finished in what order
							bool alreadyFinished = false; //Indicates whether the player is already on the winners list

							for( decltype( numPlayers ) i = 0; i < winners.size() and not alreadyFinished; ++i ) { //changed decltype( winners.size() ) to decltype( numPlayers ) because winners.size() can never exceed numPlayers but can be stored in a needlessly slow integer type.
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

					won = ( winners.size() >= numPlayers ); //If all the players are on the winners list, we've won.

				} else if( not device->isWindowActive() ) { //if(( not showingLoadingScreen and device->isWindowActive() ) or debug )
					showingMenu = true;
					device->yield();
				}

				if( showingMenu and not timer->isStopped() ) {
					timer->stop();
				} else if( not showingMenu and timer->isStopped() ) {
					timer->start();
				}

				//TODO: add networking stuff here
				/*if( isServer ) {
					if( network.checkForConnections() not_eq 0 ) {
						std::wcerr << L"Networking error." << std::endl;
					} else {
						if( network.hasNewPlayerConnected() ) {
							if( debug ) {
								std::wcout << L"New connections exist." << std::endl;
							}
							network.sendMaze( mazeManager.maze, mazeManager.cols, mazeManager.rows );
							network.sendGoal( goal );
							network.sendPlayerStarts( playerStart );
							network.sendU8( numKeysFound, L"NUMKEYSFOUND" );
							network.sendU8( numLocks, L"NUMLOCKS" );
							network.sendCollectables( stuff );
						}
					}
				}

				if( network.checkForConnections() < 0 ) {
					std::wcerr << L"Networking error." << std::endl;
				} else {
					if( network.receiveData() ) {
						if( debug ) {
							std::wcout << L"Received data" << std::endl;
						}
					} else {
						if( debug ) {
							//std::wcout << L"Did not receive data" << std::endl;
						}
					}
				}*/
			}

			timer->stop();

			if( not donePlaying ) {
				if( debug ) {
					std::wcout << L"On to the next levelnot " << std::endl;
					std::wcout << L"Winners:";

					for( decltype( numPlayers ) i = 0; i < winners.size(); ++i ) { //changed decltype( winners.size() ) to decltype( numPlayers ) because winners.size() can never exceed numPlayers but can be stored in a needlessly slow integer type.
						std::wcout << L" " << winners.at( i );
					}

					std::wcout << std::endl;
				}
				startLoadingScreen();
			}
		}
	} catch( std::exception &e ) {
		std::wcerr << L"Error in GameManager::run(): " << e.what() << std::endl;
		return EXIT_FAILURE;
	}
	
	if( debug ) {
		std::wcout << L"end of run()" << std::endl;
	}
	
	return EXIT_SUCCESS;
}

/**
 * Loads control settings from controls.cfg.
 */
void GameManager::setControls() {
	try {
		if( debug ) {
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
				if( debug ) {
					std::wcout << L"Loading controls from file " << controlsPath.wstring() << std::endl;
				}
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
						if( debug ) {
							std::wcout << L"Line " << lineNum << L": \"" << line << "\"" << std::endl;
						}
						
						if( not line.empty() ) { //The line may be empty, either in the actual file or the result of removing comments and spaces
							try {
								std::wstring preference = boost::algorithm::trim_copy( line.substr( 0, line.find( L'\t' ) ) );
								std::wstring choiceStr = boost::algorithm::trim_copy( line.substr( line.find( L'\t' ) ) );
								if( debug ) {
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
										
										if( debug ) {
											std::wcout << L"controller number (string) \"" << controllerNumStr << L"\"" << std::endl;
										}
										
										uint_fast8_t choice;
										
										choice = static_cast< uint_fast8_t >( boost::lexical_cast< unsigned short int >( controllerNumStr ) ); //Boost lexical cast can't convert directly to uint_fast8_t, at least on my computer
										
										controls.back().setControllerNumber( choice );
										
										if( debug ) {
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
											
											if( debug ) {
												std::wcout << L"controller button number (string) \"" << controllerButtonStr << L"\"" << std::endl;
											}
											
											choice = static_cast< uint_fast8_t >( boost::lexical_cast< unsigned short int >( controllerButtonStr ) ); //Boost lexical cast can't convert directly to uint_fast8_t, at least on my computer
											
											controls.back().setControllerButton( choice );
											
											if( debug ) {
												std::wcout << L" converts to integer " << choice << std::endl;
											}
										} else { //L"joystick"
											std::wstring joystickAxisStr = boost::algorithm::trim_copy( choiceStr.substr( buttonOrJoystick.length(), 2 ) );
											
											if( debug ) {
												std::wcout << L"joystick axis (string) \"" << joystickAxisStr << L"\"" << std::endl;
											}
											
											{
												std::vector< std::wstring > possibleChoices = { L"x", L"y", L"z", L"r", L"u", L"v" };
												std::wstring choice = possibleChoices.at( spellChecker.indexOfClosestString( joystickAxisStr, possibleChoices ) );
												
												if( debug ) {
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
									if( debug ) {
										std::wcout << L"choiceStr: " << choiceStr;
									}
									
									{
										std::wstring moveOrButtonOrWheel = choiceStr.substr( 0, choiceStr.find( L' ' ) );
										if( debug ) {
											std::wcout << L"moveOrButtonOrWheel before spell checking: " << moveOrButtonOrWheel;
										}
										
										std::vector< std::wstring > possibleChoices = { L"wheel", L"leftbutton", L"middlebutton", L"rightbutton", L"move" };
										moveOrButtonOrWheel = possibleChoices.at( spellChecker.indexOfClosestString( moveOrButtonOrWheel, possibleChoices ) );
										
										if( debug ) {
											std::wcout << L"moveOrButtonOrWheel after spell checking: " << moveOrButtonOrWheel;
										}
										
										if( moveOrButtonOrWheel == possibleChoices.at( 0 ) ) { //L"wheel"
											controls.back().setMouseEvent( irr::EMIE_MOUSE_WHEEL );
											
											std::wstring wheelDirection = boost::algorithm::trim_copy( choiceStr.substr( moveOrButtonOrWheel.length() ) );
											if( debug ) {
												std::wcout << L"wheelDirection before spell checking: " << wheelDirection << std::endl;
											}
											std::vector< std::wstring > possibleDirections = { L"up", L"u", L"down", L"d" };
											wheelDirection = possibleDirections.at( spellChecker.indexOfClosestString( wheelDirection, possibleDirections ) );
											if( debug ) {
												std::wcout << L"wheelDirection after spell checking: " << wheelDirection << std::endl;
											}
											
											if( wheelDirection == possibleDirections.at( 0 ) or wheelDirection == possibleDirections.at( 1 ) ) { //up
												controls.back().setMouseWheelUp( true );
											} else {
												controls.back().setMouseWheelUp( false );
											}
										} else if( moveOrButtonOrWheel == possibleChoices.at( 1 ) ) { //L"leftbutton"
											std::wstring upOrDown = boost::algorithm::trim_copy( choiceStr.substr( moveOrButtonOrWheel.length() ) );
											if( debug ) {
												std::wcout << L"upOrDown before spell checking: " << upOrDown << std::endl;
											}
											std::vector< std::wstring > possibleStates = { L"up", L"u", L"down", L"d" };
											upOrDown = possibleStates.at( spellChecker.indexOfClosestString( upOrDown, possibleStates ) );
											if( debug ) {
												std::wcout << L"upOrDown after spell checking: " << upOrDown << std::endl;
											}
											
											if( upOrDown == possibleStates.at( 0 ) or upOrDown == possibleStates.at( 1 ) ) { //L"up"
												controls.back().setMouseEvent( irr::EMIE_LMOUSE_LEFT_UP );
											} else {
												controls.back().setMouseEvent( irr::EMIE_LMOUSE_PRESSED_DOWN );
											}
										} else if( moveOrButtonOrWheel == possibleChoices.at( 2 ) ) { //L"middlebutton"
											std::wstring upOrDown = boost::algorithm::trim_copy( choiceStr.substr( moveOrButtonOrWheel.length() ) );
											if( debug ) {
												std::wcout << L"upOrDown before spell checking: " << upOrDown << std::endl;
											}
											std::vector< std::wstring > possibleStates = { L"up", L"u", L"down", L"d" };
											upOrDown = possibleStates.at( spellChecker.indexOfClosestString( upOrDown, possibleStates ) );
											if( debug ) {
												std::wcout << L"upOrDown after spell checking: " << upOrDown << std::endl;
											}
											
											if( upOrDown == possibleStates.at( 0 ) or upOrDown == possibleStates.at( 1 ) ) { //L"up"
												controls.back().setMouseEvent( irr::EMIE_MMOUSE_LEFT_UP );
											} else {
												controls.back().setMouseEvent( irr::EMIE_MMOUSE_PRESSED_DOWN );
											}
										} else if( moveOrButtonOrWheel == possibleChoices.at( 3 ) ) { //L"rightbutton"
											std::wstring upOrDown = boost::algorithm::trim_copy( choiceStr.substr( moveOrButtonOrWheel.length() ) );
											if( debug ) {
												std::wcout << L"upOrDown before spell checking: " << upOrDown << std::endl;
											}
											std::vector< std::wstring > possibleStates = { L"up", L"u", L"down", L"d" };
											upOrDown = possibleStates.at( spellChecker.indexOfClosestString( upOrDown, possibleStates ) );
											if( debug ) {
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
											
											if( debug ) {
												std::wcout << L"direction before spell checking: " << direction << std::endl;
											}
											std::vector< std::wstring > possibleDirections = { L"up", L"u", L"down", L"d", L"left", L"l", L"right", L"r" };
											direction = possibleDirections.at( spellChecker.indexOfClosestString( direction, possibleDirections ) );
											if( debug ) {
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
										decltype( numPlayers ) playerNum = boost::lexical_cast< unsigned short int >( playerNumStr ); //Boost doesn't like casting to uint_fast8_t
										
										if( playerNum < numPlayers ) {
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
									if( debug ) {
										std::wcout << L"Menu action: " << menuWhat << std::endl;
									}
									if( menuWhat == possibleMenuStuff.at( 0 ) or menuWhat == possibleMenuStuff.at( 1 ) ) { //L"up"
										if( debug ) {
											std::wcout << L"Setting control to menu up" << std::endl;
										}
										controls.back().setAction( ControlMapping::ACTION_MENU_UP );
									} else if( menuWhat == possibleMenuStuff.at( 2 ) or menuWhat == possibleMenuStuff.at( 3 ) ) { //L"down"
										if( debug ) {
											std::wcout << L"Setting control to menu down" << std::endl;
										}
										controls.back().setAction( ControlMapping::ACTION_MENU_DOWN );
									} else { //L"activate"
										if( debug ) {
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
									if( debug ) {
										std::wcout << L"preference before spell checking: " << preference;
									}
									
									std::vector< std::wstring > possiblePrefs = { L"screenshot", L"enable controller" };
									preference = possiblePrefs.at( spellChecker.indexOfClosestString( preference, possiblePrefs ) );
									
									if( debug ) {
										std::wcout  << "\tand after: " << preference << std::endl;
									}
									
									if( preference == possiblePrefs.at( 0 ) ) {
										controls.back().setAction( ControlMapping::ACTION_SCREENSHOT );
									} else if( preference == possiblePrefs.at( 1 ) ) { //L"enable controller"
										std::vector< std::wstring > possibleChoices = { L"true", L"false" };
										choiceStr = possibleChoices.at( spellChecker.indexOfClosestString( choiceStr, possibleChoices ) );
										
										if( choiceStr == possibleChoices.at( 0 ) ) {
											if( debug ) {
												std::wcout << L"controller is ENABLED" << std::endl;
											}
											enableController = true;
										} else {
											if( debug ) {
												std::wcout << L"controller is DISABLED" << std::endl;
											}
											enableController = false;
										}
									}
								}
								
							} catch( std::exception &e ) {
								std::wcerr << L"Error in GameManager::setControls(): " << e.what() << std::endl;
							} catch( std::wstring &e ) {
							std::wcerr << L"Error in GameManager::setControls(): " << e << std::endl;
							}
						}
					}
					
					controlsFile.close();
				}
			}
		}
		
		if( not controlsFileFound ) {
			throw( std::wstring( L"controls.cfg does not exist or is not readable in any of the folders that were searched." ) );
		}
	} catch( std::exception &e ) {
		std::wcerr << L"Error in GameManager::setControls(): " << e.what() << std::endl;
	} catch( std::wstring &e ) {
		std::wcerr << L"Error in GameManager::setControls(): " << e << std::endl;
	}
	
	if( debug ) {
		std::wcout << L"end of setControls()" << std::endl;
	}
}

/**
 * Randomly picks a background animation and does whatever it needs to do to set it up.
 */
void GameManager::setupBackground() {
	try {
		if( debug ) {
			std::wcout << L"setupBackground() called" << std::endl;
		}
		
		uint_fast8_t availableBackgrounds = 4; //The number of different background animations to choose from
		
		if( debug ) {
			backgroundChosen = availableBackgrounds - 1; //If we're debugging, we may be testing the last background added.
			std::wcout << L"Background chosen: " << backgroundChosen << std::endl;
		} else {
			backgroundChosen = getRandomNumber() % availableBackgrounds;
		}
		
		backgroundTexture = nullptr;
		
		switch( backgroundChosen ) {
			case 0: { //Original starfield: just flies straight forward.
				backgroundColor = BLACK;
				
				// create a particle system
				irr::scene::ICameraSceneNode* camera = backgroundSceneManager->addCameraSceneNode();
				camera->setPosition( irr::core::vector3df( 0, 0, -150 ) );
				irr::scene::IParticleSystemSceneNode* ps = backgroundSceneManager->addParticleSystemSceneNode( false );

				irr::video::SColor darkStarColor;
				irr::video::SColor lightStarColor;

				switch( getRandomNumber() % 8 ) { //Not a magic number: count the cases
					case 0: {
						darkStarColor = BLACK;
						lightStarColor = WHITE;
						break;
					}
					case 1: {
						darkStarColor = BLUE;
						lightStarColor = LIGHTBLUE;
						break;
					}
					case 2: {
						darkStarColor = GREEN;
						lightStarColor = LIGHTGREEN;
						break;
					}
					case 3: {
						darkStarColor = CYAN;
						lightStarColor = LIGHTCYAN;
						break;
					}
					case 4: {
						darkStarColor = RED;
						lightStarColor = LIGHTRED;
						break;
					}
					case 5: {
						darkStarColor = MAGENTA;
						lightStarColor = LIGHTMAGENTA;
						break;
					}
					case 6: {
						darkStarColor = GRAY;
						lightStarColor = LIGHTGRAY;
						break;
					}
					case 7: {
						darkStarColor = YELLOW;
						lightStarColor = BROWN;
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
				//pixelImage->fill( WHITE );
				pixelImage->setPixel( 0, 0, WHITE, false ); //Which is faster on a 1x1 pixel image: setPixel() or fill()?
				irr::video::ITexture* pixelTexture = driver->addTexture( "pixel", pixelImage );
				ps->setMaterialTexture( 0, pixelTexture );
				break;
			}
			case 3: { //"Paint" background (deliberately falls through to starfield, do not add a break). This smearing background started out as a bug, but I liked the look so much I decided to replicate it as a feature.
				if( driver->queryFeature( irr::video::EVDF_RENDER_TO_TARGET  ) ) {
					backgroundTexture = driver->addRenderTargetTexture( windowSize );
					backgroundColor = BLACK;
					
					{ //Fill the texture with the background color;
						driver->beginScene( false, false, backgroundColor );
						driver->setRenderTarget( backgroundTexture, true, true, backgroundColor );
						driver->setRenderTarget( irr::video::ERT_FRAME_BUFFER, false, false, backgroundColor );
						driver->endScene();
					}
					
				} else {
					backgroundChosen = 1;
				}
			}
			case 1: { //New starfield: rotates the camera around.
				if( backgroundChosen == 1 ) {
					backgroundColor = BLACK;
				}
				
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
					if( debug ) {
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
						darkStarColor = BLACK;
						lightStarColor = WHITE;
						break;
					}
					case 1: {
						darkStarColor = BLUE;
						lightStarColor = LIGHTBLUE;
						break;
					}
					case 2: {
						darkStarColor = GREEN;
						lightStarColor = LIGHTGREEN;
						break;
					}
					case 3: {
						darkStarColor = CYAN;
						lightStarColor = LIGHTCYAN;
						break;
					}
					case 4: {
						darkStarColor = RED;
						lightStarColor = LIGHTRED;
						break;
					}
					case 5: {
						darkStarColor = MAGENTA;
						lightStarColor = LIGHTMAGENTA;
						break;
					}
					case 6: {
						darkStarColor = GRAY;
						lightStarColor = LIGHTGRAY;
						break;
					}
					case 7: {
						darkStarColor = YELLOW;
						lightStarColor = BROWN;
						break;
					}
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
			case 2: { //Image files
				std::vector< boost::filesystem::path > backgroundList;

				boost::filesystem::path backgroundPath( boost::filesystem::current_path()/L"images/backgrounds" );

				//Which is better: system_complete() or absolute()? On my computer they seem to do the same thing. Both are part of Boost Filesystem.
				backgroundPath = system_complete( backgroundPath );
				//backgroundPath = absolute( backgroundPath );

				if( debug ) {
					std::wcout << L"background path is absolute? " << backgroundPath.is_absolute() << std::endl;
				}

				while( ( not exists( backgroundPath ) or not is_directory( backgroundPath ) ) and backgroundPath.has_parent_path() ) {
					if( debug ) {
						std::wcout << L"Path " << backgroundPath.wstring() << L" does not exist or is not a directory. Checking parent path " << backgroundPath.parent_path().wstring() << std::endl;
					}

					backgroundPath = backgroundPath.parent_path();
				}

				if( exists( backgroundPath ) ) {
					boost::filesystem::recursive_directory_iterator end;

					for( boost::filesystem::recursive_directory_iterator i( backgroundPath ); i not_eq end; ++i ) {
						if( not is_directory( i->path() ) ) { //We've found a file
							if( debug ) {
								std::wcout << i->path().wstring() << std::endl;
							}

							//Asks Irrlicht if the file is loadable. This way the game is certain to accept any file formats the library can use.
							for( decltype( driver->getImageLoaderCount() ) loaderNum = 0; loaderNum < driver->getImageLoaderCount(); ++loaderNum ) { //Irrlicht uses a different image loader for each file type. Loop through them all, ask each if it can load the file.

								irr::video::IImageLoader* loader = driver->getImageLoader( loaderNum );
								irr::io::IFileSystem* fileSystem = device->getFileSystem();
								irr::io::path filePath = stringConverter.toIrrlichtStringW( i->path().wstring() );

								//if( loader->isALoadableFileExtension( filePath ) ) { //Commenting this out because extensions don't always reflect the file's contents. Uncomment it for a minor speed improvement since not all files would need to be opened.
									irr::io::IReadFile* file = fileSystem->createAndOpenFile( filePath );
									if( loader->isALoadableFileFormat( file ) ) {
										backgroundList.push_back( i->path() );
										file->drop();
										break;
									}
									file->drop();
								/*} else {
									if( debug ) {
										bool isLoadableExtension = loader->isALoadableFileExtension( filePath );
										bool isLoadableFormat = loader->isALoadableFileFormat( fileSystem->createAndOpenFile( filePath ) );
										std::wcout << "is loadable extension? " << isLoadableExtension << " is loadable format? " << isLoadableFormat << std::endl;
									}
								}*/
							}
						}
					}
				}

				if( not backgroundList.empty() ) {
					std::vector< boost::filesystem::path >::iterator newEnd = std::unique( backgroundList.begin(), backgroundList.end() ); //unique "removes all but the first element from every consecutive group of equivalent elements in the range [first,last)." (source: http://www.cplusplus.com/reference/algorithm/unique/ )
					backgroundList.resize( std::distance( backgroundList.begin(), newEnd ) );

					//Pick a random background and load it
					backgroundFilePath = stringConverter.toIrrlichtStringW( backgroundList.at( getRandomNumber() % backgroundList.size() ).wstring() );
					backgroundTexture = driver->getTexture( backgroundFilePath );
					if( backgroundTexture == 0 ) {
						std::wstring error = L"Cannot load background texture, even though Irrlicht said it was loadable?!?";
						throw error;
					}
				} else {
					std::wcerr << L"Could not find any background images." << std::endl;
				}

				break;
			}

			default: {
				std::wstring error = L"Background chosen is not in switch statement.";
				throw error;
			}
		}
	} catch( std::exception &e ) {
		std::wcerr << L"Error in GameManager::setupBackground(): " << e.what() << std::endl;
	} catch( std::wstring &e ) {
		std::wcerr << L"Error in GameManager::setupBackground(): " << e << std::endl;
	}
	
	if( debug ) {
		std::wcout << L"end of setupBackground()" << std::endl;
	}
}

/**
 * Called by menuManager.
 */
void GameManager::setExitConfirmation( irr::gui::IGUIWindow* newWindow ) {
	exitConfirmation = newWindow;
}

/**
 * Sets loadingProgress
 * Arguments: Yes please.
 **/
void GameManager::setLoadingPercentage( float newPercent ) {
 	if( newPercent < 100 and newPercent > 0 ) {
		loadingProgress = newPercent;
 	} else if( newPercent >= 100 ) {
 		loadingProgress = 100;
 	} else {
 		loadingProgress = 0;
 	}
 }

/**
 * Sets the random number generator's seed.
 * Arguments:
 * std::minstd_rand::result_type newSeed - The new seed to use. CPlusPlus.com says this type is equal to uint_fast32_t.
 **/
void GameManager::setRandomSeed( std::minstd_rand::result_type newSeed ) {
	randomSeed = newSeed;
	randomNumberGenerator.seed( randomSeed );
	
	if( debug ) {
		std::wcout << L"New random seed: " << randomSeed << std::endl;
	}
	
}

/**
 * Creates a file selection dialog for loading the maze
 */
void GameManager::showLoadMazeDialog() {
	if( not isNull( loadMazeDialog ) ) {
		delete loadMazeDialog;
	}
	loadMazeDialog = new FileSelectorDialog( L"Load Maze", gui, gui->getRootGUIElement(), 0, FileSelectorDialog::EFST_OPEN_DIALOG );
	loadMazeDialog->addFileFilter( mazeManager.getFileTypeName(), mazeManager.getFileTypeExtension(), driver->getTexture( L"images/icon.png" ) );
}

/**
 * Creates a file selection dialog for saving the maze
 */
void GameManager::showSaveMazeDialog() {
	if( not isNull( saveMazeDialog ) ) {
		delete saveMazeDialog;
	}
	saveMazeDialog = new FileSelectorDialog( L"Save Maze", gui, gui->getRootGUIElement(), 1, FileSelectorDialog::EFST_SAVE_DIALOG );
	//saveMazeDialog->addFileFilter( mazeManager.getFileTypeName(), mazeManager.getFileTypeExtension(), driver->getTexture( L"images/icon.png" ) );
}

/**
 * Sets showingLoadingScreen to true and timeStartedLoading to the current time, then calls drawLoadingScreen().
 */
void GameManager::startLoadingScreen() {
	if( debug ) {
		std::wcout << L"startLoadingScreen() called" << std::endl;
	}
	showingLoadingScreen = true;
	timeStartedLoading = timer->getRealTime();
	//drawLoadingScreen();
	
	if( debug ) {
		std::wcout << L"end of startLoadingScreen()" << std::endl;
	}
}

/**
 * Takes a screenshot and saves it to a time-stamped png file.
 */
void GameManager::takeScreenShot() {
	try {
		if( debug ) {
			std::wcout << L"takeScreenShot() called" << std::endl;
		}
		
		irr::video::IImage* image = nullptr;
		if( not isNull( driver ) ) {
			image = driver->createScreenShot();
		}
		
		if( not isNull( image ) ) {
			irr::core::stringw filename = stringConverter.toIrrlichtStringW( PACKAGE_NAME );
			filename.append( L" screenshot " );
			
			time_t currentTime = time( nullptr );
			wchar_t clockTime[ 20 ];
			if( wcsftime( clockTime, 20, L"%FT%T", localtime( &currentTime ) ) == 0 ) {
				throw( std::wstring( L"Could not convert the time to ISO 8601 format.") );
			}
			clockTime[ 19 ] = L'\0';
			filename.append( clockTime );
			filename.append( L".png" );
			
			if( not driver->writeImageToFile( image, filename ) ) {
				throw( std::wstring( L"Failed to save screen shot to file " + stringConverter.toStdWString( filename ) ) );
			} else {
				irr::core::stringw success = L"Screen shot saved as \"";
				success.append( filename );
				success.append( L"\"" );
				if( debug ) {
					std::wcout <<  stringConverter.toStdWString( success ) << std::endl;
				}
				if( not isNull( gui ) ) {
					gui->addMessageBox( L"Screenshot saved", success.c_str() );
				}
			}
			
			image->drop();
		} else {
			throw( std::wstring( L"takeScreenShot(): Failed to take screen shot" ) );
		}
	} catch( std::exception &e ) {
		std::wcerr << L"Error in GameManager::takeScreenShot(): " << e.what() << std::endl;
	} catch( std::wstring e ) {
		std::wcerr << L"Error in GameManager::takeScreenShot(): " << e << std::endl;
	}
	
	if( debug ) {
		std::wcout << L"end of takeScreenShot()" << std::endl;
	}
}
