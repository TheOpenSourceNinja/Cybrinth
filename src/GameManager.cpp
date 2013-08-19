/**
 * Copyright © 2013 James Dearing.
 * This file is part of Cybrinth.
 *
 * Cybrinth is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 *
 * Cybrinth is distributed in the hope that it will be fun, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License along with Cybrinth. If not, see <http://www.gnu.org/licenses/>.
*/

#define BOOST_FILESYSTEM_NO_DEPRECATED //Recommended by the Boost filesystem library documentation to prevent us from using functions which will be removed in later versions

#include "GameManager.h"
#include <filesystem/fstream.hpp>
#include <algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <SDL/SDL.h>
#include <taglib/fileref.h>
#include <taglib/tag.h>

#if defined WINDOWS //Networking stuff
#include <winsock>
#elif defined LINUX
#include <sys/socket>
#include <netinet/in>
#include <arpa/inet>
#include <netdb>
#endif //What about other operating systems? I don't know what to include for BSD etc.

//Custom user events for Irrlicht
#define USER_EVENT_WINDOW_RESIZE 1
#define USER_EVENT_JOYSTICK_UP 2
#define USER_EVENT_JOYSTICK_LEFT 3
#define USER_EVENT_JOYSTICK_DOWN 4
#define USER_EVENT_JOYSTICK_RIGHT 5

//TODO: Add control switcher item (icon: yin-yang using players' colors?)
//TODO: Find or record clock ticking sound for use with various items
//TODO: Send on connect: random seed, keys and locations, locks and locations
//TODO: Get multiplayer working online
//TODO: Add AI. Two difficulty settings ('already knows the solution' and 'finds the solution as it plays') and any solving algorithms I can think of (depth-first and breadth-first search)
//TODO: Possible idea: Hide parts of the maze that are inaccessible due to locks.
//TODO: Possible idea: Hide parts of the maze not seen yet (seen means line-of-sight to any visited cell)
//TODO: Add shader to simulate old monitor?
//TODO: Add more backgrounds.
//TODO: Consider adding stats (estimated difficulty of maze, number of steps taken, number of cells backtracked, etc)
//TODO: Add a loading screen between maps. Display pro tips on loading screen. Possibly display player stats too.
//TODO: Add try/catch blocks around each function
//TODO: See if there's a way to tell if types are defined. We want to be able to compile on systems that don't have optional types.
//TODO: Throw errors.


using namespace irr;
//using boost::asio::ip::tcp;


bool GameManager::allHumansAtGoal() {
	try {
		std::vector< uint_least8_t > humanPlayers; //Get a list of players

		for( uint_fast8_t p = 0; p < numPlayers; p++ ) {
			humanPlayers.push_back( p );
		}

		bool result = false;

		for( uint_fast8_t b = 0; b < numBots; b++ ) { //Remove bots from the list
			uint_least8_t botPlayer = bot.at( b ).getPlayer();

			for( uint_fast8_t p = 0; p < humanPlayers.size(); p++ ) {
				if( humanPlayers.at( p ) == botPlayer ) {
					humanPlayers.erase( humanPlayers.begin() + p );
				}
			}
		}

		if( humanPlayers.size() > 0 ) {
			result = true;

			for( uint_fast8_t p = 0; ( p < humanPlayers.size() && result == true ); p++ ) {
				if( !( player.at( humanPlayers.at( p ) ).getX() == goal.getX() && player.at( humanPlayers.at( p ) ).getY() == goal.getY() ) ) {
					result = false;
				}
			}

		}

		return result;
	} catch( std::exception e ) {
		std::wcerr << L"Error in GameManager::allHumansAtGoal(): " << e.what() << std::endl;
		return false;
	}
}

void GameManager::drawAll() {
	try {
		driver->beginScene();

		if( !showingLoadingScreen ) {
			if( showBackgrounds ) {
				drawBackground();
			}

			//Draws player trails ("footprints")
			if( markTrails ) {
				for( uint_fast8_t x = 0; x < mazeManager.cols; x++ ) { //It's inefficient to do this here and have similar nested loops below drawing the walls, but I want these drawn before the players, and the players drawn before the walls.
					for( uint_fast8_t y = 0; y < mazeManager.rows; y++ ) {
						if( mazeManager.maze[ x ][ y ].visited ) {
							int_fast16_t dotSize = cellWidth / 5;

							if( dotSize < 2 ) {
								dotSize = 2;
							}

							driver->draw2DRectangle( mazeManager.maze[ x ][ y ].getVisitorColor() , core::rect<s32>( core::position2d<s32>(( x * cellWidth ) + ( 0.5 * cellWidth ) - ( 0.5 * dotSize ), ( y * cellHeight ) + ( 0.5 * cellHeight ) - ( 0.5 * dotSize ) ), core::dimension2d< s32 >( dotSize, dotSize ) ) );
						}
					}
				}
			}

			for( uint_fast8_t ps = 0; ps < playerStart.size(); ps++ ) { //Put this in a separate loop from the players (below) so that the players would all be drawn after the playerStarts.
				playerStart.at( ps ).draw( driver, cellWidth, cellHeight );
			}

			//Because of a texture resizing bug in Irrlicht's software renderers, we draw the items (currently all keys, which use a large png as a texture) before the players (which generate their own texture at the correct size) and the maze walls.
			for( uint_fast16_t i = 0; i < stuff.size(); i++ ) {
				stuff.at( i ).draw( driver, cellWidth, cellHeight );
			}

			//Drawing bots before human players makes it easier to play against large numbers of bots
			for( uint_fast8_t i = 0; i < numBots; i++ ) {
				player.at( bot.at( i ).getPlayer() ).draw( driver, cellWidth, cellHeight );
			}

			for( uint_fast8_t p = 0; p < numPlayers; p++ ) {
				if( player.at( p ).isHuman ) {
					player.at( p ).draw( driver, cellWidth, cellHeight );
				}
			}

			goal.draw( driver, cellWidth, cellHeight );

			mazeManager.draw( driver, cellWidth, cellHeight );

			if( showingMenu ) {
				newGame.draw( driver );
				loadMaze.draw( driver );
				saveMaze.draw( driver );
				exitGame.draw( driver );
				backToGame.draw( driver );
			}


			uint_least32_t spaceBetween = windowSize.Height / 30;
			uint_least32_t textY = spaceBetween;

			time_t currentTime = time( NULL );
			wchar_t clockTime[9];
			wcsftime( clockTime, 9, L"%H:%M:%S", localtime( &currentTime ) );
			core::dimension2d<uint_least32_t> tempDimensions = clockFont->getDimension( core::stringw( clockTime ).c_str() );
			core::rect<int_least32_t> tempRectangle( viewportSize.Width + 1, textY, tempDimensions.Width + ( viewportSize.Width + 1 ), tempDimensions.Height + textY );
			clockFont->draw( clockTime, tempRectangle, LIGHTMAGENTA, true, true, &tempRectangle );

			core::stringw timeLabel( L"Time:" );
			textY += tempDimensions.Height;
			tempDimensions = textFont->getDimension( timeLabel.c_str() );
			tempRectangle = core::rect<int_least32_t>( viewportSize.Width + 1, textY, tempDimensions.Width + ( viewportSize.Width + 1 ), tempDimensions.Height + textY );
			textFont->draw( L"Time:", tempRectangle, YELLOW, true, true, &tempRectangle );
			core::stringw timerStr( "" );
			timerStr += ( timer->getTime() / 1000 );
			timerStr += L" seconds";
			textY += tempDimensions.Height;
			tempDimensions = textFont->getDimension( timerStr.c_str() );
			tempRectangle = core::rect<int_least32_t>( viewportSize.Width + 1, textY, tempDimensions.Width + ( viewportSize.Width + 1 ), tempDimensions.Height + textY );
			textFont->draw( timerStr, tempRectangle, YELLOW, true, true, &tempRectangle );

			core::stringw keysFoundStr( L"Keys found:" );
			textY += tempDimensions.Height;
			tempDimensions = textFont->getDimension( keysFoundStr.c_str() );
			tempRectangle = core::rect<int_least32_t>( viewportSize.Width + 1, textY, tempDimensions.Width + ( viewportSize.Width + 1 ), tempDimensions.Height + textY );
			textFont->draw( keysFoundStr, tempRectangle, YELLOW, true, true, &tempRectangle );

			core::stringw keyStr;
			keyStr += numKeysFound;
			keyStr += L"/";
			keyStr += numLocks;
			textY += tempDimensions.Height;
			tempDimensions = textFont->getDimension( keyStr.c_str() );
			tempRectangle = core::rect<int_least32_t>( viewportSize.Width + 1, textY, tempDimensions.Width + ( viewportSize.Width + 1 ), tempDimensions.Height + textY );
			textFont->draw( keyStr, tempRectangle, YELLOW, true, true, &tempRectangle );

			core::stringw seedLabel( L"Random seed:" );
			textY += tempDimensions.Height;
			tempDimensions = textFont->getDimension( seedLabel.c_str() );
			tempRectangle = core::rect<int_least32_t>( viewportSize.Width + 1, textY, tempDimensions.Width + ( viewportSize.Width + 1 ), tempDimensions.Height + textY );
			textFont->draw( seedLabel, tempRectangle, YELLOW, true, true, &tempRectangle );

			core::stringw seedStr( randomSeed );
			textY += tempDimensions.Height;
			tempDimensions = textFont->getDimension( seedStr.c_str() );
			tempRectangle = core::rect<int_least32_t>( viewportSize.Width + 1, textY, tempDimensions.Width + ( viewportSize.Width + 1 ), tempDimensions.Height + textY );
			textFont->draw( seedStr, tempRectangle, YELLOW, true, true, &tempRectangle );

			core::stringw headfor( L"Head for" );
			textY += tempDimensions.Height;
			tempDimensions = textFont->getDimension( headfor.c_str() );

			if( textY < (( windowSize.Height / 2 ) - tempDimensions.Height ) ) {
				textY = (( windowSize.Height / 2 ) - tempDimensions.Height );
			}

			if( numKeysFound >= numLocks ) {
				tempRectangle = core::rect<int_least32_t>( viewportSize.Width + 1, textY, tempDimensions.Width + ( viewportSize.Width + 1 ), tempDimensions.Height + textY );
				textFont->draw( headfor, tempRectangle, LIGHTMAGENTA, true, true, &tempRectangle );
			}

			core::stringw theexit( L"the exit!" );
			textY += tempDimensions.Height;
			tempDimensions = textFont->getDimension( theexit.c_str() );

			if( numKeysFound >= numLocks ) {
				tempRectangle = core::rect<int_least32_t>( viewportSize.Width + 1, textY, tempDimensions.Width + ( viewportSize.Width + 1 ), tempDimensions.Height + textY );
				textFont->draw( theexit, tempRectangle, LIGHTCYAN, true, true, &tempRectangle );
			}

			if( playMusic ) {
				core::stringw nowplaying( L"Now playing:" );
				textY += tempDimensions.Height;
				tempDimensions = textFont->getDimension( nowplaying.c_str() );
				tempRectangle = core::rect<int_least32_t>( viewportSize.Width + 1, textY, tempDimensions.Width + ( viewportSize.Width + 1 ), tempDimensions.Height + textY );
				textFont->draw( nowplaying, tempRectangle, YELLOW, true, true, &tempRectangle );

				textY += tempDimensions.Height;
				tempDimensions = musicTagFont->getDimension( musicTitle.c_str() );
				tempRectangle = core::rect<int_least32_t>( viewportSize.Width + 1, textY, tempDimensions.Width + ( viewportSize.Width + 1 ), tempDimensions.Height + textY );
				musicTagFont->draw( musicTitle, tempRectangle, LIGHTGREEN, true, true, &tempRectangle );

				core::stringw by( L"by" );
				textY += tempDimensions.Height;
				tempDimensions = textFont->getDimension( by.c_str() );
				tempRectangle = core::rect<int_least32_t>( viewportSize.Width + 1, textY, tempDimensions.Width + ( viewportSize.Width + 1 ), tempDimensions.Height + textY );
				textFont->draw( by, tempRectangle, YELLOW, true, true, &tempRectangle );

				textY += tempDimensions.Height;
				tempDimensions = musicTagFont->getDimension( musicArtist.c_str() );
				tempRectangle = core::rect<int_least32_t>( viewportSize.Width + 1, textY, tempDimensions.Width + ( viewportSize.Width + 1 ), tempDimensions.Height + textY );
				musicTagFont->draw( musicArtist, tempRectangle, LIGHTGREEN, true, true, &tempRectangle );

				core::stringw fromalbum( L"from album" );
				textY += tempDimensions.Height;
				tempDimensions = textFont->getDimension( fromalbum.c_str() );
				tempRectangle = core::rect<int_least32_t>( viewportSize.Width + 1, textY, tempDimensions.Width + ( viewportSize.Width + 1 ), tempDimensions.Height + textY );
				textFont->draw( fromalbum, tempRectangle, YELLOW, true, true, &tempRectangle );

				textY += tempDimensions.Height;
				tempDimensions = musicTagFont->getDimension( musicAlbum.c_str() );
				tempRectangle = core::rect<int_least32_t>( viewportSize.Width + 1, textY, tempDimensions.Width + ( viewportSize.Width + 1 ), tempDimensions.Height + textY );
				musicTagFont->draw( musicAlbum, tempRectangle, LIGHTGRAY, true, true, &tempRectangle );

				core::stringw volume( L"Volume:" );
				textY += tempDimensions.Height;
				tempDimensions = textFont->getDimension( volume.c_str() );
				tempRectangle = core::rect<int_least32_t>( viewportSize.Width + 1, textY, tempDimensions.Width + ( viewportSize.Width + 1 ), tempDimensions.Height + textY );
				textFont->draw( volume, tempRectangle, YELLOW, true, true, &tempRectangle );

				core::stringw volumeNumber( musicVolume );
				volumeNumber.append( L"%" );
				textY += tempDimensions.Height;
				tempDimensions = textFont->getDimension( volumeNumber.c_str() );
				tempRectangle = core::rect<int_least32_t>( viewportSize.Width + 1, textY, tempDimensions.Width + ( viewportSize.Width + 1 ), tempDimensions.Height + textY );
				textFont->draw( volumeNumber, tempRectangle, LIGHTRED, true, true, &tempRectangle );
			}

			gui->drawAll();
		} else {
			drawLoadingScreen();
		}

		driver->endScene();
	} catch( std::exception e ) {
		std::wcerr << L"Error in GameManager::drawAll(): " << e.what() << std::endl;
	}
}

void GameManager::drawBackground() {
	try {
		switch( backgroundChosen ) {
			case 0: {
				bgscene->drawAll();
			}
			break;
			default:
				break;
		}
	} catch( std::exception e ) {
		std::wcerr << L"Error in GameManager::drawBackground(): " << e.what() << std::endl;
	}
}

void GameManager::drawLoadingScreen() {
	try {
		if( loadingFont == NULL ) {
			loadingFont = gui->getBuiltInFont();
		}

		core::dimension2d< uint_fast16_t > loadingDimensions = loadingFont->getDimension( loading.c_str() );
		int_fast16_t textY = 0;
		int_fast16_t textX = ( windowSize.Width / 2 ) - ( loadingDimensions.Width / 2 );
		core::rect< int_fast16_t > tempRectangle(textX, textY, ( windowSize.Width / 2 ) + ( loadingDimensions.Width / 2 ), loadingDimensions.Height + textY );
		loadingFont->draw( loading, tempRectangle, YELLOW, true, true, &tempRectangle );


		if( proTips.size() > 0 ) {
			if( tipFont == NULL ) {
				tipFont = gui->getBuiltInFont();
			}

			textY = loadingDimensions.Height + textY + 1;
			core::dimension2d< uint_fast16_t > proTipPrefixDimensions = tipFont->getDimension( proTipPrefix.c_str() );
			core::dimension2d< uint_fast16_t > proTipDimensions = tipFont->getDimension( proTips.at( currentProTip ).c_str() );
			textX = ( windowSize.Width / 2 ) - ( ( proTipPrefixDimensions.Width + proTipDimensions.Width ) / 2 );
			tempRectangle = core::rect< int_fast16_t >( textX, textY, proTipPrefixDimensions.Width + textX, proTipPrefixDimensions.Height + textY );
			tipFont->draw( proTipPrefix, tempRectangle, LIGHTCYAN, true, true, &tempRectangle );

			textX += proTipPrefixDimensions.Width;
			tempRectangle = core::rect< int_fast16_t >( textX, textY, proTipDimensions.Width + textX, proTipDimensions.Height + textY );
			tipFont->draw( proTips.at( currentProTip ), tempRectangle, WHITE, true, true, &tempRectangle );
		}
	} catch( std::exception e ) {
		std::wcerr << L"Error in GameManager::drawLoadingScreen(): " << e.what() << std::endl;
	}
}

//I... am... DESTRUCTOOOOOOORRRRR!!!!!!!!!
GameManager::~GameManager() {
	try {
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
	} catch( std::exception e ) {
		std::wcerr << L"Error in GameManager::~GameManager(): " << e.what() << std::endl;
	}
}

GameManager::GameManager() {
	try {
		//Just wanted to be totally sure that these point to NULL before being set otherwise
		clockFont = NULL;
		loadingFont = NULL;
		musicTagFont = NULL;
		statsFont = NULL;
		textFont = NULL;
		tipFont = NULL;

		loading = L"Loading...";
		proTipPrefix = L"Pro tip: ";
		stats = L"Player stats";
		steps = L"Steps";
		network.setGameManager( this );
		mazeManager.setGameManager( this );
		isServer = false;
		antiAliasFonts = true;
		currentProTip = 0;
		sideDisplaySizeDenominator = 6; //What fraction of the screen's width is set aside for displaying text, statistics, etc. during play.

		fontFile = "";
		boost::filesystem::recursive_directory_iterator end;
		for( boost::filesystem::recursive_directory_iterator i(L"./"); i != end; i++ ) {
			if( !is_directory( i->path() ) && boost::iequals( i->path().extension().generic_wstring(), L".ttf" ) ) { //Can we use formats other than TTF? What about identifying files without the use of extensions?
				fontFile = i->path().c_str();
				break;
			}
		}

		device = createDevice( video::EDT_NULL ); //Must create a device before calling readPrefs();

		if( !device ) {
			throw( std::wstring( L"Cannot create null device. Something is definitely wrong here!" ) );
		} else if ( debug ) {
			std::wcout << L"Got the null device" << std::endl;
		}

		readPrefs();

		if ( debug ) {
			std::wcout << L"Read prefs, now setting controls" << std::endl;
		}

		setControls();

		if( fullscreen && !allowSmallSize ) {
			windowSize = device->getVideoModeList()->getDesktopResolution();
		}

		viewportSize.set( windowSize.Width - ( windowSize.Width / sideDisplaySizeDenominator ), windowSize.Height - 1 );

		bool sbuffershadows = false; //Would not be visible anyway since this game is 2D
		IEventReceiver* receiver = this;

		device->closeDevice(); //Signals to the existing device that it needs to close itself on next run() so that we can create a new device
		device->run(); //This is next run()
		device->drop(); //Cleans up after the device

		device = createDevice( driverType, windowSize, bitsPerPixel, fullscreen, sbuffershadows, vsync, receiver ); //Most of these parameters were read from the preferences file

		if( !device ) {
			std::wcerr << L"Error: Cannot create device. Trying software renderer." << std::endl;
			device = createDevice( video::EDT_SOFTWARE, windowSize, bitsPerPixel, fullscreen, sbuffershadows, vsync, receiver );

			if( !device ) {
				throw( std::wstring( L"Even the software renderer didn't work." ) );
			}
		} else if ( debug ) {
			std::wcout << L"Got the new device" << std::endl;
		}

		gui = device->getGUIEnvironment(); //Put this before drawLoadingScreen() because drawLoadingScreen() needs gui
		if( !gui ) {
			throw( std::wstring( L"Cannot get GUI environment" ) );
		} else {
			if ( debug ) {
				std::wcout << L"Got the gui environment" << std::endl;
			}
			for( uint_fast16_t i = 0; i < gui::EGDC_COUNT ; i++ ) {
				video::SColor guiSkinColor = gui->getSkin()->getColor( static_cast<gui::EGUI_DEFAULT_COLOR>( i ) );
				guiSkinColor.setAlpha( 255 );
				gui->getSkin()->setColor( static_cast<gui::EGUI_DEFAULT_COLOR>( i ), guiSkinColor );
			}
		}

		device->getVideoDriver()->beginScene(true, false, BLACK);
		drawLoadingScreen();
		device->getVideoDriver()->endScene();

		loadProTips();

		device->setWindowCaption( core::stringw( PACKAGE_STRING ).c_str() );

		if( debug ) {
			device->getLogger()->setLogLevel( ELL_INFORMATION );
		} else {
			device->getLogger()->setLogLevel( ELL_ERROR );
		}

		driver = device->getVideoDriver(); //Not sure if this would be possible with a null device, which is why we don't exit
		if( !driver ) {
			throw( std::wstring( L"Cannot get video driver" ) );
		} else if ( debug ) {
			std::wcout << L"Got the video driver" << std::endl;
		}

		driver->setTextureCreationFlag( video::ETCF_NO_ALPHA_CHANNEL, false );
		driver->setTextureCreationFlag( video::ETCF_CREATE_MIP_MAPS, false );
		if( driverType == video::EDT_SOFTWARE || driverType == video:: EDT_BURNINGSVIDEO ) {
			driver->setTextureCreationFlag( video::ETCF_OPTIMIZED_FOR_SPEED, true );
		} else {
			driver->setTextureCreationFlag( video::ETCF_OPTIMIZED_FOR_QUALITY, true );
		}
		if( driverType = video::EDT_SOFTWARE ) {
			driver->setTextureCreationFlag( video::ETCF_ALLOW_NON_POWER_2, false );
		}

		bgscene = device->getSceneManager(); //Not sure if this would be possible with a null device, which is why we don't exit
		if( !bgscene ) {
			throw( std::wstring( L"Cannot get scene manager" ) );
		} else if ( debug ) {
			std::wcout << L"Got the scene manager" << std::endl;
		}


		if( playMusic ) {

			if( SDL_Init( SDL_INIT_AUDIO ) == -1 ) {
				std::wcerr << L"Cannot initialize SDL audio." << std::endl;
				playMusic = false;
			}

			//Set the audio properties we hope to get: sample rate, channels, etc.
			int audio_rate = 44100; //44.1 KHz is the standard sample rate for CDs, and so makes a good 'lowest common denominator' for anything related to audio.
			Uint16 audio_format = AUDIO_S16SYS; //CDs use signed 16-bit audio. SYS means use the system's native endianness.
			int audio_channels = 2; //Almost everything uses stereo. I wish surround sound were more common.
			int audio_buffers = 4096; //Magic number! Change it if you dare, and see what happeens.

			if( Mix_OpenAudio( audio_rate, audio_format, audio_channels, audio_buffers ) != 0 ) {
				std::wcerr << L"Unable to initialize audio: " << Mix_GetError() << std::endl;
				playMusic = false;
			} else if ( debug ) {
				std::wcout << L"Initilized audio" << std::endl;
			}

			if( debug ) {
				Mix_QuerySpec( &audio_rate, &audio_format, &audio_channels );//Don't assume we got everything we asked for above
				std::wcout << L"Audio sample rate: " << audio_rate << L" Hertz format: ";
				if( audio_format == AUDIO_U16SYS ) {
					std::wcout << L"AUDIO_U16SYS (equivalent to ";
					if( AUDIO_U16SYS == AUDIO_U16LSB ) {
						std::wcout << L"AUDIO_U16LSB)";
					} else if( AUDIO_U16SYS == AUDIO_U16MSB ) {
						std::wcout << L"AUDIO_U16MSB)";
					} else if( AUDIO_U16SYS == AUDIO_U16 ) {
						std::wcout << L"AUDIO_U16)";
					} else {
						std::wcout << L"unknown)";
					}
				} else if( audio_format == AUDIO_S16SYS ) {
					std::wcout << L"AUDIO_S16SYS (equivalent to ";
					if( AUDIO_S16SYS == AUDIO_S16LSB ) {
						std::wcout << L"AUDIO_S16LSB)";
					} else if( AUDIO_S16SYS == AUDIO_S16MSB ) {
						std::wcout << L"AUDIO_S16MSB)";
					} else if( AUDIO_S16SYS == AUDIO_S16 ) {
						std::wcout << L"AUDIO_S16)";
					} else {
						std::wcout << L"unknown)";
					}
				} else if( audio_format == AUDIO_U8 ) {
					std::wcout << L"AUDIO_U8";
				} else if( audio_format == AUDIO_S8 ) {
					std::wcout << L"AUDIO_S8";
				} else if( audio_format == AUDIO_U16LSB ) {
					std::wcout << L"AUDIO_U16LSB";
				} else if( audio_format == AUDIO_S16LSB ) {
					std::wcout << L"AUDIO_S16LSB";
				} else if( audio_format == AUDIO_U16MSB ) {
					std::wcout << L"AUDIO_U16MSB";
				} else if( audio_format == AUDIO_S16MSB ) {
					std::wcout << L"AUDIO_S16MSB";
				} else if( audio_format == AUDIO_U16 ) {
					std::wcout << L"AUDIO_U16";
				} else if( audio_format == AUDIO_S16 ) {
					std::wcout << L"AUDIO_S16";
				} else {
					std::wcout << L"unknown";
				}

				std::wcout << " channels: " << audio_channels  << L" buffers: " << audio_buffers << std::endl;
			}

			music = NULL;

			if( playMusic ) { //No sense in making the music list if we've been unable to initialize the audio.
				makeMusicList();
				if( playMusic ) {  //playMusic may be set to false if makeMusicList() can't find any songs to play
					loadNextSong();
				}
			}
		}

		if( debug ) {
			std::wcout << L"About to call loadFonts()" << std::endl;
		}
		loadFonts();
		if( debug ) {
			std::wcout << L"Returned from loadFonts()" << std::endl;
		}

		newGame.setText( L"New maze" );
		newGame.setFont( clockFont );
		loadMaze.setText( L"Load maze" );
		loadMaze.setFont( clockFont );
		saveMaze.setText( L"Save maze" );
		saveMaze.setFont( clockFont );
		exitGame.setText( L"Exit game" );
		exitGame.setFont( clockFont );
		backToGame.setText( L"Back to game" );
		backToGame.setFont( clockFont );

		newGame.setY( 0 );
		loadMaze.setY( 1 * windowSize.Height / 5 );
		saveMaze.setY( 2 * windowSize.Height / 5 );
		exitGame.setY( 3 * windowSize.Height / 5 );
		backToGame.setY( 4 * windowSize.Height / 5 );

		if ( debug ) {
			std::wcout << L"Resizing player and playerStart vectors to " << numPlayers << std::endl;
		}

		player.resize( numPlayers );
		playerStart.resize( numPlayers );

		if( numBots > numPlayers ) {
			numBots = numPlayers;
		}

		if( numBots <= numPlayers && numBots > 0 ) {
			if ( debug ) {
				std::wcout << L"Resizing numBots vector to " << numBots << std::endl;
			}

			bot.resize( numBots );

			for( uint_fast8_t i = 0; i < numBots; i++ ) {
				bot.at( i ).setPlayer( numPlayers - ( i + 1 ) ) ;
				player.at( bot.at( i ).getPlayer() ).isHuman = false;
			}
		}

		for( uint_fast8_t p = 0; p < numPlayers; p++ ) {
			player.at( p ).setColorBasedOnNum( p );
			player.at( p ).loadTexture( driver );
		}

		goal.loadTexture( driver );

		if( debug && enableJoystick && device->activateJoysticks( joystickInfo ) ) { //activateJoysticks fills joystickInfo with info about each joystick
			std::wcout << L"Joystick support is enabled and " << joystickInfo.size() << L" joystick(s) are present." << std::endl;

			for( uint_fast16_t joystick = 0; joystick < joystickInfo.size(); ++joystick ) {
				std::wcout << L"Joystick " << joystick << L":" << std::endl;
				std::wcout << L"\tName: '" << joystickInfo[ joystick ].Name.c_str() << L"'" << std::endl;
				std::wcout << L"\tAxes: " << joystickInfo[ joystick ].Axes << std::endl;
				std::wcout << L"\tButtons: " << joystickInfo[ joystick ].Buttons << std::endl;

				std::wcout << L"\tHat is: ";

				switch( joystickInfo[joystick].PovHat ) {
					case SJoystickInfo::POV_HAT_PRESENT:
						std::wcout << L"present" << std::endl;
						break;

					case SJoystickInfo::POV_HAT_ABSENT:
						std::wcout << L"absent" << std::endl;
						break;

					case SJoystickInfo::POV_HAT_UNKNOWN:
					default:
						std::wcout << L"unknown" << std::endl;
						break;
				}
			}
		} else if( debug ) {
			std::wcout << L"Joystick support is not enabled." << std::endl;
		}

		//Set up networking
		network.setup( isServer );

		timer = device->getTimer();

		if( debug ) {
			std::wcout << L"end of GameManager constructor" << std::endl;
		}
	} catch( std::exception e ) {
		std::wcerr << L"Error in GameManager::GameManager(): " << e.what() << std::endl;
	} catch( std::wstring e ) {
		std::wcerr << L"Error in GameManager::GameManager(): " << e << std::endl;
	}
}

bool GameManager::getDebugStatus() {
	try {
		return debug;
	} catch( std::exception e ) {
		std::wcerr << L"Error in GameManager::getDebugStatus(): " << e.what() << std::endl;
		return true;
	}
}

Goal GameManager::getGoal() {
	try {
		return goal;
	} catch( std::exception e ) {
		std::wcerr << L"Error in GameManager::getGoal(): " << e.what() << std::endl;
		Goal g;
		return g;
	}
}

MazeManager GameManager::getMazeManager() {
	try {
		return mazeManager;
	} catch( std::exception e ) {
		std::wcerr << L"Error in GameManager::getMazeManager(): " << e.what() << std::endl;
		MazeManager m;
		return m;
	}
}

Player* GameManager::getPlayer( uint_least8_t p ) {
	try {
		if( p < numPlayers ) {
			return &player.at( p );
		} else {
			std::wstring e = L"Request for player (" + std::to_wstring( p ) + L">= numPlayers (" + std::to_wstring( numPlayers ) + L")";
			throw( e );
		}
	} catch( std::exception e ) {
		std::wcerr << L"Error in GameManager::getPlayer(): " << e.what() << std::endl;
		return NULL;
	} catch( std::wstring e ) {
		std::wcerr << L"Error in GameManager::getPlayer(): " << e << std::endl;
		return NULL;
	}
}

void GameManager::loadFonts() {
	try {
		if( debug ) {
			std::wcout << L"loadFonts called" << std::endl;
		}
		//These were split off into separate functions because they are needed more often than loadFonts()
		loadTipFont();
		loadMusicFont();
		if( debug ) {
			std::wcout << L"Returned from loadMusicFont" << std::endl;
		}

		core::dimension2d< uint_least32_t > fontDimensions;
		uint_least32_t size = windowSize.Width / 30; //30 found through experimentation: much larger and it takes too long to load fonts, much smaller and the font doesn't get as big as it should. Feel free to change at will if your computer's faster than mine.

		do { //Repeatedly loading fonts like this seems like a waste of time. Is there a way we could load the font only once and still get this kind of size adjustment?
			loadingFont = fm.GetTtFont( driver, fontFile.c_str(), size, antiAliasFonts );
			if( loadingFont != NULL ) {
				fontDimensions = loadingFont->getDimension( loading.c_str() );
				size -= 2;
			}
		} while( loadingFont != NULL && ( fontDimensions.Width > ( windowSize.Width / sideDisplaySizeDenominator ) || fontDimensions.Height > ( windowSize.Height / 5 ) ) );

		size += 3;

		do {
			loadingFont = fm.GetTtFont( driver, fontFile.c_str(), size, antiAliasFonts );
			if( loadingFont != NULL ) {
				fontDimensions = loadingFont->getDimension( loading.c_str() );
				size -= 1;
			}
		} while( loadingFont != NULL && ( fontDimensions.Width > ( windowSize.Width / sideDisplaySizeDenominator ) || fontDimensions.Height > ( windowSize.Height / 5 ) ) );

		if( loadingFont == NULL ) {
			loadingFont = gui->getBuiltInFont();
		}

		if( debug ) {
			std::wcout << L"loadingFont is loaded" << std::endl;
		}

		size = ( windowSize.Width / sideDisplaySizeDenominator ) / 6; //found through experimentation, adjust it however you like and see how many times the font gets loaded

		do {
			if( debug ) {
				std::wcout << L"About to load textFont in loop (size " << size << L")" << std::endl;
			}
			textFont = fm.GetTtFont( driver, fontFile.c_str(), size, antiAliasFonts );
			if( debug ) {
				std::wcout << L"Loaded textFont in loop (size " << size << L")" << std::endl;
			}
			if( textFont != NULL ) {
				if( debug ) {
					std::wcout << L"textFont is not null" << std::endl;
				}
				fontDimensions = textFont->getDimension( L"Random seed: " );
				if( debug ) {
					std::wcout << L"fontDimensions set to " << fontDimensions.Width << L"x" << fontDimensions.Height << std::endl;
				}
				size -= 2;
			}
		} while( textFont != NULL && ( fontDimensions.Width + viewportSize.Width > windowSize.Width ) );

		size += 3;

		do {
			if( debug ) {
				std::wcout << L"About to load textFont in loop (size " << size << L")" << std::endl;
			}
			textFont = fm.GetTtFont( driver, fontFile.c_str(), size, antiAliasFonts );
			if( debug ) {
				std::wcout << L"Loaded textFont in loop (size " << size << L")" << std::endl;
			}
			if( textFont != NULL ) {
				if( debug ) {
					std::wcout << L"textFont is not null" << std::endl;
				}
				fontDimensions = textFont->getDimension( L"Random seed: " );
				if( debug ) {
					std::wcout << L"fontDimensions set to " << fontDimensions.Width << L"x" << fontDimensions.Height << std::endl;
				}
				size -= 1;
			}
		} while( textFont != NULL && ( fontDimensions.Width + viewportSize.Width > windowSize.Width ) );

		if( textFont == NULL ) {
			textFont = gui->getBuiltInFont();
		}

		if( debug ) {
			std::wcout << L"textFont is loaded" << std::endl;
		}

		size = ( windowSize.Width / sideDisplaySizeDenominator );

		do {
			clockFont = fm.GetTtFont( driver, fontFile.c_str(), size, antiAliasFonts );
			if( clockFont != NULL ) {
				fontDimensions = clockFont->getDimension( L"00:00:00" );
				size -= 2;
			}
		} while( clockFont != NULL && ( fontDimensions.Width + viewportSize.Width > windowSize.Width  || fontDimensions.Height > ( windowSize.Height / 5 ) ) );

		size += 3;

		do {
			clockFont = fm.GetTtFont( driver, fontFile.c_str(), size, antiAliasFonts );
			if( clockFont != NULL ) {
				fontDimensions = clockFont->getDimension( L"00:00:00" );
				size -= 1;
			}
		} while( clockFont != NULL && ( fontDimensions.Width + viewportSize.Width > windowSize.Width  || fontDimensions.Height > ( windowSize.Height / 5 ) ) );

		if( clockFont == NULL ) {
			clockFont = gui->getBuiltInFont();
		}

		if( debug ) {
			std::wcout << L"clockFont is loaded" << std::endl;
		}
	} catch( std::exception e ) {
		std::wcerr << L"Error in GameManager::loadFonts(): " << e.what() << std::endl;
	}
}

//Like loadTipFont() below, this guesses a good font size, then repeatedly adjusts the size and reloads the font until everything fits.
void GameManager::loadMusicFont() {
	try {
		if( debug ) {
			std::wcout << L"loadMusicFont called" << std::endl;
		}

		if( playMusic ) {
			uint_fast32_t maxWidth = ( windowSize.Width / sideDisplaySizeDenominator );
			uint_fast32_t size = 0;
			uint_fast32_t numerator = 2.5 * maxWidth; //2.5 is an arbitrarily chosen number, it has no special meaning. Change it to anything you want.

			//I felt it looked best if all three (artist, album, and title) had the same font size, so we're picking the longest of the three and basing the font size on its length.
			if( musicArtist.size() >= musicAlbum.size() && musicArtist.size() > 0 ) {
				if( musicArtist.size() >= musicTitle.size() ) {
					size = numerator / musicArtist.size();
				} else if( musicTitle.size() > 0 ) {
					size = numerator / musicTitle.size();
				}
			} else {
				if( musicAlbum.size() >= musicTitle.size() && musicAlbum.size() > 0 ) {
					size = numerator / musicAlbum.size();
				} else if( musicTitle.size() > 0 ) {
					size = numerator / musicTitle.size();
				}
			}

			if( size == 0 ) { //If none of album, artist, and title had lengths greater than zero: maybe they haven't been read yet, or maybe the file doesn't contain those tags.
				size = numerator;
			}

			core::dimension2d< uint_fast32_t > artistDimensions;
			core::dimension2d< uint_fast32_t > albumDimensions;
			core::dimension2d< uint_fast32_t > titleDimensions;

			do {
				musicTagFont = fm.GetTtFont( driver, fontFile.c_str(), size, antiAliasFonts );
				if( musicTagFont != NULL ) {
					artistDimensions = musicTagFont->getDimension( musicArtist.c_str() );
					albumDimensions = musicTagFont->getDimension( musicAlbum.c_str() );
					titleDimensions = musicTagFont->getDimension( musicTitle.c_str() );
					size -= 2;
				}
			} while( musicTagFont != NULL && ( artistDimensions.Width > maxWidth || albumDimensions.Width > maxWidth || titleDimensions.Width > maxWidth ) );

			size += 3;

			do {
				musicTagFont = fm.GetTtFont( driver, fontFile.c_str(), size, antiAliasFonts );
				if( musicTagFont != NULL ) {
					artistDimensions = musicTagFont->getDimension( musicArtist.c_str() );
					albumDimensions = musicTagFont->getDimension( musicAlbum.c_str() );
					titleDimensions = musicTagFont->getDimension( musicTitle.c_str() );
					size -= 1;
				}
			} while( musicTagFont != NULL && ( artistDimensions.Width > maxWidth || albumDimensions.Width > maxWidth || titleDimensions.Width > maxWidth ) );

			if( musicTagFont == NULL ) {
				musicTagFont = gui->getBuiltInFont();
			}
		}
	} catch( std::exception e ) {
		std::wcerr << L"Error in GameManager::loadMusicFont(): " << e.what() << std::endl;
	}
}

void GameManager::loadNextSong() {
	try {
		if( debug ) {
			std::wcout << L"loadNextSong() called" << std::endl;
		}

		//Figure out where we are in the music list
		std::vector<boost::filesystem::path>::size_type positionInList = 0;
		for( std::vector<boost::filesystem::path>::size_type i = 0; i < musicList.size(); i++ ) {
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

		currentMusic = musicList[positionInList];
		music = Mix_LoadMUS( currentMusic.c_str() ); //SDL Mixer does not support wstrings

		if( music == NULL ) {
			std::wstring e = L"Unable to load music file: ";
			e += stringConverter.toStdWString( std::string( Mix_GetError() ) );
			throw( e );
		} else {
			int musicStatus = Mix_PlayMusic( music, 0 ); //The second argument tells how many times to play the music. -1 means infinite.

			if( musicStatus == -1 ) {
				std::wstring e = L"Unable to play music file: ";
				e += stringConverter.toStdWString( Mix_GetError() );
				throw( e );
			} else {
				if( debug ) {
					switch( Mix_GetMusicType( NULL ) ) {
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
				TagLib::FileRef f( currentMusic.c_str() ); //TagLib doesn't accept wstrings as file names, but it apparently can read tags as wstrings

				if( !f.isNull() && f.tag() ) {
					musicTitle = stringConverter.toIrrlichtStringW( f.tag()->title().toWString() ); //toWString() alone doesn't work here even though these are wide character strings because Irrlicht doesn't like accepting TagLib's wstrings.
					musicArtist = stringConverter.toIrrlichtStringW( f.tag()->artist().toWString() );
					musicAlbum = stringConverter.toIrrlichtStringW( f.tag()->album().toWString() );

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
						std::wcout << L"Now playing: " << musicTitle.c_str() << L" by " << musicArtist.c_str() << L" from album " << musicAlbum.c_str() << std::endl;
					}
				}

				loadMusicFont();
			}

			Mix_VolumeMusic( musicVolume * MIX_MAX_VOLUME / 100 );
		}
	} catch( std::exception e ) {
		std::wcerr << L"Error in GameManager::loadNextSong(): " << e.what() << std::endl;
	} catch( std::wstring e ) {
		std::wcerr << L"Error in GameManager::loadNextSong(): " << e << std::endl;
	}
}

//Loads "pro tips" from file
void GameManager::loadProTips() {
	try {
		proTips.clear(); //This line is unnecessary because loadProTips() is only called once, but I just feel safer clearing this anyway.
		boost::filesystem::path proTipsPath( L"./protips.txt" );

		if( exists( proTipsPath ) ) {
			if( !is_directory( proTipsPath ) ) {
				if( debug ) {
					std::wcout << L"Loading pro tips from file " << proTipsPath.wstring() << std::endl;
				}
				boost::filesystem::wifstream proTipsFile;
				proTipsFile.open( proTipsPath );

				if( proTipsFile.is_open() ) {
					std::wstring line;
					uint_fast16_t lineNum = 0;

					while( proTipsFile.good() ) {
						lineNum++;
						getline( proTipsFile, line );

						if( !line.empty() ) {
							proTips.push_back( stringConverter.toIrrlichtStringW( line ) ); //StringConverter converts between wstring (which is what getLine needs) and core::stringw (which is what Irrlicht needs)

							if( debug ) {
								std::wcout << line << std::endl;
							}
						}
					}

					proTipsFile.close();

					srand( time( NULL ) );
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
	} catch( std::exception e ) {
		std::wcerr << L"Error in GameManager::loadProTips(): " << e.what() << std::endl;
	} catch( std::wstring e ) {
		std::wcerr << L"Error in GameManager::loadProTips(): " << e << std::endl;
	}
}

//Guesses a size that will work for showing pro tips. Keeps adjusting the size and reloading the font until everything fits.
void GameManager::loadTipFont() {
	try {
		if( debug ) {
			std::wcout << L"loadTipFont called" << std::endl;
		}

		uint_fast32_t maxWidth = windowSize.Width;
		uint_fast32_t size;
		core::stringw tipIncludingPrefix = proTipPrefix;

		if( proTips.size() > 0 ) { //If pro tips have been loaded, guess size based on tip length.
			tipIncludingPrefix.append( proTips.at( currentProTip ) );
			size = 2.5 * maxWidth / tipIncludingPrefix.size(); //2.5 is an arbitrarily chosen number, it has no special meaning. Change it to anything you want.
		} else {
			size = maxWidth / 10; //10 is also arbitrarily chosen.
		}

		core::dimension2d<uint_fast32_t> tipDimensions;

		do {
			tipFont = fm.GetTtFont( driver, fontFile.c_str(), size, antiAliasFonts );
			if( tipFont != NULL ) {
				tipDimensions = tipFont->getDimension( tipIncludingPrefix.c_str() );
				size -= 2;
			}
		} while( tipFont != NULL && ( tipDimensions.Width > maxWidth ) );

		size += 3;

		do {
			tipFont = fm.GetTtFont( driver, fontFile.c_str(), size, antiAliasFonts );
			if( tipFont != NULL ) {
				tipDimensions = tipFont->getDimension( tipIncludingPrefix.c_str() );
				size -= 1;
			}
		} while( tipFont != NULL && ( tipDimensions.Width > maxWidth ) );

		if( tipFont == NULL ) {
			tipFont = gui->getBuiltInFont();
		}
	} catch( std::exception e ) {
		std::wcerr << L"Error in GameManager::loadTipFont(): " << e.what() << std::endl;
	}
}

//Finds all playable music files in the ./music folder and compiles them into a list
void GameManager::makeMusicList() {
	try {
		musicList.clear(); //The music list should be empty anyway, since makeMusicList() only gets called once, but just in case...

		if( debug ) {
			std::wcout << L"makeMusicList() called" << std::endl;
			uint_least8_t numMusicDecoders = Mix_GetNumMusicDecoders();
			std::wcout << L"There are " << numMusicDecoders << L" music decoders available. They are:" << std::endl;

			for( uint_fast8_t decoder = 0; decoder < numMusicDecoders; decoder++ ) {
				std::wcout << decoder << L": " << Mix_GetMusicDecoder( decoder ) << std::endl;
			}
		}

		boost::filesystem::path musicPath( L"./music" );

		//Which is better: system_complete() or absolute()? On my computer they seem to do the same thing. Both are part of Boost Filesystem.
		musicPath = system_complete( musicPath );
		//musicPath = absolute( musicPath );

		if( debug ) {
			std::wcout << L"music path is absolute? " << musicPath.is_absolute() << std::endl;
		}

		while( ( !exists( musicPath ) || !is_directory( musicPath ) ) && musicPath.has_parent_path() ) {
			if( debug ) {
				std::wcout << L"Path " << musicPath.wstring() << L" does not exist or is not a directory. Checking parent path " << musicPath.parent_path().wstring() << std::endl;
			}

			musicPath = musicPath.parent_path();
		}

		if( exists( musicPath ) ) {
			boost::filesystem::recursive_directory_iterator end;

			for( boost::filesystem::recursive_directory_iterator i( musicPath ); i != end; i++ ) {
				if( !is_directory( i->path() ) ) { //We've found a file
					//Attempts to load a file as music. If successful, unload the file and add it to musicList.
					//This way the game is certain to accept any file formats the music library can use.
					Mix_Music* temp = Mix_LoadMUS( i->path().c_str() );

					if( temp != NULL ) {
						musicList.push_back( i->path() );
						Mix_FreeMusic( temp );
					}
				}
			}
		}

		if( musicList.size() > 0 ) {
			//Do we want music sorted or random?
			//sort( musicList.begin(), musicList.end() );
			srand( time( NULL ) );
			random_shuffle( musicList.begin(), musicList.end() );

			currentMusic = musicList.back();
		} else {
			std::wcout << L"Could not find any music to play. Turning off playback." << std::endl;
			playMusic = false;
		}
	} catch( std::exception e ) {
		std::wcerr << L"Error in GameManager::makeMusicList(): " << e.what() << std::endl;
	}
}

void GameManager::movePlayerOnX( uint_least8_t p, int_fast8_t direction ) {
	try {
		if( numPlayers > p ) {
			if( direction < 0 ) {
				if( player.at( p ).getX() > 0 && mazeManager.maze[ player.at( p ).getX() ][ player.at( p ).getY() ].getLeft() == 'n' ) {
					player.at( p ).moveX( -1 );
				}
			} else if( player.at( p ).getX() < ( mazeManager.cols - 1 ) && mazeManager.maze[ player.at( p ).getX() + 1 ][ player.at( p ).getY() ].getLeft() == 'n' ) {
				player.at( p ).moveX( 1 );
			}

			network.sendPlayerPos( p, player.at( p ).getX(), player.at( p ).getY() );
			mazeManager.maze[ player.at( p ).getX() ][ player.at( p ).getY() ].visited = true;

			if( player.at( p ).stepsTaken % 2 == 0 ) {
				mazeManager.maze[ player.at( p ).getX() ][ player.at( p ).getY() ].setVisitorColor( player.at( p ).getColorTwo() );
			} else {
				mazeManager.maze[ player.at( p ).getX() ][ player.at( p ).getY() ].setVisitorColor( player.at( p ).getColorOne() );
			}
		}
	} catch( std::exception e ) {
		std::wcerr << L"Error in GameManager::movePlayerOnX(): " << e.what() << std::endl;
	}
}

void GameManager::movePlayerOnY( uint_least8_t p, int_fast8_t direction ) {
	try {
		if( numPlayers > p ) {
			if( direction < 0 ) {
				if( player.at( p ).getY() > 0 && mazeManager.maze[ player.at( p ).getX() ][ player.at( p ).getY() ].getTop() == 'n' ) {
					player.at( p ).moveY( -1 );
				}
			} else if( player.at( p ).getY() < ( mazeManager.rows - 1 ) && mazeManager.maze[ player.at( p ).getX() ][ player.at( p ).getY() + 1 ].getTop() == 'n' ) {
				player.at( p ).moveY( 1 );
			}

			network.sendPlayerPos( p, player.at( p ).getX(), player.at( p ).getY() );
			mazeManager.maze[ player.at( p ).getX() ][ player.at( p ).getY() ].visited = true;

			if( player.at( p ).stepsTaken % 2 == 0 ) {
				mazeManager.maze[ player.at( p ).getX() ][ player.at( p ).getY() ].setVisitorColor( player.at( p ).getColorTwo() );
			} else {
				mazeManager.maze[ player.at( p ).getX() ][ player.at( p ).getY() ].setVisitorColor( player.at( p ).getColorOne() );
			}
		}
	} catch( std::exception e ) {
		std::wcerr << L"Error in GameManager::movePlayerOnY(): " << e.what() << std::endl;
	}
}

void GameManager::newMaze() {
	try {
		resetThings();
		mazeManager.makeRandomLevel();
		cellWidth = ( viewportSize.Width ) / mazeManager.cols;
		cellHeight = ( viewportSize.Height ) / mazeManager.rows;
	} catch( std::exception e ) {
		std::wcerr << L"Error in GameManager::newMaze(): " << e.what() << std::endl;
	}
}

void GameManager::newMaze( boost::filesystem::path src ) {
	try {
		resetThings();
		mazeManager.loadFromFile( src );
		cellWidth = ( viewportSize.Width ) / mazeManager.cols;
		cellHeight = ( viewportSize.Height ) / mazeManager.rows;
	} catch( std::exception e ) {
		std::wcerr << L"Error in GameManager::newMaze(): " << e.what() << std::endl;
	}
}

bool GameManager::OnEvent( const SEvent& event ) {
	try {
		switch( event.EventType ) {
			case EET_KEY_INPUT_EVENT: {
				if( event.KeyInput.PressedDown ) {
					if( !( showingMenu || showingLoadingScreen ) ) {
						for( std::vector< KeyMapping >::size_type k = 0; k < keyMap.size(); k++ ) {
							if( event.KeyInput.Key == keyMap.at( k ).getKey() ) {
								switch( keyMap.at( k ).getAction() ) {
									case 'm': {
										showingMenu = true;
										return true;
										break;
									}
									case 'd': {
										movePlayerOnY( keyMap.at( k ).getPlayer(), 1 );
										return true;
										break;
									}
									case 'u': {
										movePlayerOnY( keyMap.at( k ).getPlayer(), -1 );
										return true;
										break;
									}
									case 'r': {
										movePlayerOnX( keyMap.at( k ).getPlayer(), 1 );
										return true;
										break;
									}
									case 'l': {
										movePlayerOnX( keyMap.at( k ).getPlayer(), -1 );
										return true;
										break;
									}
									case 's': {
										takeScreenShot();
										return true;
										break;
									}
								}
								break;
							}
						}
					} else if( showingMenu ) {
						for( std::vector< KeyMapping >::size_type k = 0; k < keyMap.size(); k++ ) {
							if( event.KeyInput.Key == keyMap.at( k ).getKey() ) {
								switch( keyMap.at( k ).getAction() ) {
									case 'm': {
										showingMenu = false;
										return true;
										break;
									}
									case 's': {
										takeScreenShot();
										return true;
										break;
									}
								}
								break;
							}
						}
					}
				}
			}
			break;

			case EET_MOUSE_INPUT_EVENT: {
				switch( event.MouseInput.Event ) {
					case EMIE_LMOUSE_PRESSED_DOWN: {
						if( showingMenu ) {
							if( exitGame.contains( event.MouseInput.X, event.MouseInput.Y ) ) {
								device->closeDevice();
								donePlaying = true;
								return true;
							} else if( loadMaze.contains( event.MouseInput.X, event.MouseInput.Y ) ) {
								fileChooser = gui->addFileOpenDialog( L"Select a Maze", true, 0, -1 );
								//std::wcout << L"Directory: " << fileChooser->getDirectoryName().c_str() << std::endl;
								//loadFromFile();
								return true;
							} else if( saveMaze.contains( event.MouseInput.X, event.MouseInput.Y ) ) {
								mazeManager.saveToFile();
								return true;
							} else if( newGame.contains( event.MouseInput.X, event.MouseInput.Y ) ) {
								newMaze();
								return true;
							} else if( backToGame.contains( event.MouseInput.X, event.MouseInput.Y ) ) {
								showingMenu = false;
								return true;
							}
						}

						break;
					}
					case EMIE_MOUSE_WHEEL: {
						if( debug ) {
							std::wcout << L"Mouse wheel moved ";
						}

						if( event.MouseInput.Wheel > 0 ) {
							if( debug ) {
								std::wcout << "up" << std::endl;
							}

							musicVolume += 5;

							if( musicVolume > 100 ) {
								musicVolume = 100;
							}
						} else {
							if( debug ) {
								std::wcout << "down" << std::endl;
							}

							if( musicVolume >= 5 ) {
								musicVolume -= 5;
							} else {
								musicVolume = 0;
							}
						}

						Mix_VolumeMusic( musicVolume * MIX_MAX_VOLUME / 100 );

						break;
					}
					default:
						break;
				}
			}
			break;

			case EET_USER_EVENT: {
				switch( event.UserEvent.UserData1 ) {
					case USER_EVENT_WINDOW_RESIZE: {
						windowSize.set( driver->getScreenSize().Width, driver->getScreenSize().Height );

						///EXPERIMENTAL!
						//TODO: Remove or finish experimental code
						/*if (!allowSmallSize) {
							bool sizeChanged = false;

							if (windowSize.Height < 480) {
								windowSize.Height = 480;
								sizeChanged = true;
							}

							if (windowSize.Width < 640) {
								windowSize.Width = 640;
								sizeChanged = true;
							}

							if (sizeChanged) {
								driver->OnResize(windowSize);
							}
						}*/

						viewportSize.set( windowSize.Width - ( windowSize.Width / sideDisplaySizeDenominator ), windowSize.Height - 1 );
						cellWidth = ( viewportSize.Width ) / mazeManager.cols;
						cellHeight = ( viewportSize.Height ) / mazeManager.rows;
						loadFonts();
						return true;
					}
					break;
					case USER_EVENT_JOYSTICK_UP: {
						if( debug ) {
							std::wcout << L"Joystick moved up" << std::endl;
						}
						movePlayerOnY( 0, -1 );
						return true;
					}
					break;
					case USER_EVENT_JOYSTICK_DOWN: {
						if( debug ) {
							std::wcout << L"Joystick moved down" << std::endl;
						}
						movePlayerOnY( 0, 1 );
						return true;
					}
					break;
					default:
						break;
				}
			}
			break;
			case EET_JOYSTICK_INPUT_EVENT: {
				if( event.JoystickEvent.Joystick == joystickChosen ) {
					core::array<int_least16_t> verticalAxes;
					verticalAxes.push_back( SEvent::SJoystickEvent::AXIS_Y );

					bool joystickMovedUp = false;
					bool joystickMovedDown = false;
					bool joystickMovedRight = false;
					bool joystickMovedLeft = false;

					for( uint_fast16_t i = 0; i < verticalAxes.size(); i++ ) {
						if( event.JoystickEvent.Axis[i] >= ( SHRT_MAX / 2 ) ) { //See Irrlicht's <irrTypes.h>: Axes are represented by s16's, typedef'd in the current version (as of 2013-06-22) as signed short. SHRT_MAX comes from <climits>
							if( debug ) {
								std::wcout << L"Axis value: " << event.JoystickEvent.Axis[i] << std::endl;
							}
							joystickMovedUp = true;
						} else if( event.JoystickEvent.Axis[i] <= ( SHRT_MIN / 2 ) ) {
							if( debug ) {
								std::wcout << L"Axis value: " << event.JoystickEvent.Axis[i] << std::endl;
							}
							joystickMovedDown = true;
						}
					}

					if( joystickMovedUp ) {
						SEvent temp;
						temp.EventType = EET_USER_EVENT;
						temp.UserEvent.UserData1 = USER_EVENT_JOYSTICK_UP;
						device->postEventFromUser( temp );
					} else if( joystickMovedDown ) {
						SEvent temp;
						temp.EventType = EET_USER_EVENT;
						temp.UserEvent.UserData1 = USER_EVENT_JOYSTICK_DOWN;
						device->postEventFromUser( temp );
					}

					return true;
				}
			}
			break;
			case EET_GUI_EVENT: {
				switch( event.GUIEvent.EventType ) {
					case gui::EGET_FILE_SELECTED: {
						if( debug ) {
							std::wcout << L"File selected." << std::endl;
							std::wcout << L"Folder: " << core::stringw( fileChooser->getDirectoryName() ).c_str() << L"\tFile: " << core::stringw( fileChooser->getFileName() ).c_str() << std::endl;
						}

						newMaze( fileChooser->getFileName() );
						return true;
						break;
					}
					case gui::EGET_DIRECTORY_SELECTED: {
						if( debug ) {
							std::wcout << L"Folder selected." << std::endl;
						}

						break;
					}
					default:
						break;
				}
			}
			default:
				break;
		}

	} catch( std::exception e ) {
		std::wcerr << L"Error in GameManager::OnEvent(): " << e.what() << std::endl;
	}

	return false;
}

void GameManager::readPrefs() {
	try {
		boost::filesystem::path prefsPath( L"./prefs.cfg" );

		//Set default prefs, in case we can't get them from the file
		showBackgrounds = true;
		fullscreen = false;
		bitsPerPixel = 8;
		vsync = true;
		driverType = video::EDT_OPENGL;
		windowSize = core::dimension2d< uint_least16_t >( 640, 480 );
		allowSmallSize = false;
		playMusic = true;
		enableJoystick = false;
		joystickChosen = 1;
		numBots = 0;
		numPlayers = 1;
		markTrails = false;
		musicVolume = 100;
		network.setPort( 61187 );
		isServer = false;

	#ifdef DEBUG
		debug = true;
	#else
		debug = false;
	#endif

		if( exists( prefsPath ) ) {
			if( !is_directory( prefsPath ) ) {
				if( debug ) {
					std::wcout << L"Loading preferences from file " << prefsPath.wstring() << std::endl;
				}
				boost::filesystem::wifstream prefsFile;
				prefsFile.open( prefsPath );

				if( prefsFile.is_open() ) {
					std::wstring line;
					uint_fast8_t lineNum = 0;

					while( prefsFile.good() ) {
						lineNum++;
						getline( prefsFile, line );
						line = line.substr( 0, line.find( L"//" ) ); //Filters out comments
						boost::algorithm::trim( line ); //Removes trailing and leading spaces
						boost::algorithm::to_lower( line );
						if( debug ) {
							std::wcout << L"Line " << lineNum << L": \"" << line << "\"" << std::endl;
						}


						if( !line.empty() ) {
							try {
								std::wstring preference = boost::algorithm::trim_copy( line.substr( 0, line.find( '\t' ) ) );
								std::wstring choice = boost::algorithm::trim_copy( line.substr( line.find( '\t' ) ) );
								if( debug ) {
									std::wcout << L"Preference \"" << preference << L"\" choice \"" << choice << L"\""<< std::endl;
								}

								if( preference == L"volume:" ) {
									try {
										uint_fast16_t choiceAsInt = boost::lexical_cast< uint_fast16_t >( choice );

										if( choiceAsInt <= 100 && choiceAsInt >= 0 ) {
											musicVolume = choiceAsInt;
											Mix_VolumeMusic( musicVolume * MIX_MAX_VOLUME / 100 );
											if( debug ) {
												std::wcout << L"Volume should be " << choiceAsInt << "%" << std::endl;
												std::wcout << L"Volume is really " << 100 * Mix_VolumeMusic( -1 ) / MIX_MAX_VOLUME << "%" << std::endl;
											}
										} else if( choiceAsInt < 0 ) {
											std::wcerr << L"Warning: Volume less than zero: " << choiceAsInt << std::endl;
											Mix_VolumeMusic( 0 );
											musicVolume = 0;
										} else {
											std::wcerr << L"Warning: Volume greater than 100%: " << choiceAsInt << std::endl;
											Mix_VolumeMusic( MIX_MAX_VOLUME );
											musicVolume = 100;
										}
									} catch( boost::bad_lexical_cast e ) {
										std::wcerr << L"Error reading volume preference (is it not a number?) on line " << lineNum << L": " << e.what() << std::endl;
									}
								} else if( preference == L"number of bots:" ) {
									try {
										uint_least8_t choiceAsInt = boost::lexical_cast< short int >( choice ); //uint_least8_t is typedef'd as a kind of char apparently, at least on my raspberry pi, and Boost lexical_cast() won't convert from wchar_t to char.

										if( choiceAsInt <= numPlayers ) {
											numBots = choiceAsInt;
											if( debug ) {
												std::wcout << L"Number of bots is " << choiceAsInt << std::endl;
											}
										} else {
											std::wcerr << L"Warning: Number of bots not less than or equal to number of players (number of players may not have been read yet): " << choiceAsInt << std::endl;
											numBots = choiceAsInt;
										}
									} catch( boost::bad_lexical_cast e ) {
										std::wcerr << L"Error reading number of bots preference (is it not a number?) on line " << lineNum << L": " << e.what() << std::endl;
									}
								} else if( preference == L"show background animations:" ) {
									if( choice == L"true" ) {
										if( debug ) {
											std::wcout << L"Show backgrounds is ON" << std::endl;
										}
										showBackgrounds = true;
									} else if( choice == L"false" ) {
										if( debug ) {
											std::wcout << L"Show backgrounds is OFF" << std::endl;
										}
										showBackgrounds = false;
									} else {
										std::wcerr << L"Error reading show background animations preference on line " << lineNum  << L": \"" << choice  << std::endl;//<< L"\"" << std::endl;
									}
								} else if( preference == L"fullscreen:" ) {

									if( choice == L"true" ) {
										if( debug ) {
											std::wcout << L"Fullscreen is ON" << std::endl;
										}
										fullscreen = true;
									} else if( choice == L"false" ) {
										if( debug ) {
											std::wcout << L"Fullscreen is OFF" << std::endl;
										}
										fullscreen = false;
									} else {
										std::wcerr << L"Error reading fullscreen preference on line " << lineNum  << L": \"" << choice  << std::endl;//<< L"\"" << std::endl;
									}
								} else if( preference == L"mark player trails:" ) {

									if( choice == L"true" ) {
										if( debug ) {
											std::wcout << L"Mark trails is ON" << std::endl;
										}
										markTrails = true;
									} else if( choice == L"false" ) {
										if( debug ) {
											std::wcout << L"Mark trails is OFF" << std::endl;
										}
										markTrails = false;
									} else {
										std::wcerr << L"Error reading mark player trails preference on line " << lineNum  << L": \"" << choice  << std::endl;//<< L"\"" << std::endl;
									}
								} else if( preference == L"debug:" ) {

									#ifndef DEBUG
									if( choice == L"true" ) {
										debug = true;
									} else if( choice == L"false" ) {
										debug = false;
									} else {
										std::wcerr << L"Error reading debug preference on line " << lineNum  << L": \"" << choice  << std::endl;//<< L"\"" << std::endl;
									}
									#endif

									if( debug ) {
										std::wcout << L"Debug is ON" << std::endl;
									}
								} else if( preference == L"bits per pixel:" ) {
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
									} catch( boost::bad_lexical_cast e ) {
										std::wcerr << L"Error reading bitsPerPixel preference (is it not a number?) on line " << lineNum << L": " << e.what() << std::endl;
									}

								} else if( preference == L"wait for vertical sync:" ) {

									if( choice == L"true" ) {
										if( debug ) {
											std::wcout << L"Vertical sync is ON" << std::endl;
										}
										vsync = true;
									} else if( choice == L"false" ) {
										if( debug ) {
											std::wcout << L"Vertical sync is OFF" << std::endl;
										}
										vsync = false;
									} else {
										std::wcerr << L"Error reading vertical sync preference on line " << lineNum << L": \"" << choice << L"\"" << std::endl;
									}

								} else if( preference == L"driver type:" ) {

									if( choice == L"opengl" ) {
										driverType = video::EDT_OPENGL;
									} else if( choice == L"direct3d9" ) {
										driverType = video::EDT_DIRECT3D9;
									} else if( choice == L"direct3d8" ) {
										driverType = video::EDT_DIRECT3D8;
									} else if( choice == L"burning's video" ) {
										driverType = video::EDT_BURNINGSVIDEO;
									} else if( choice == L"software" ) {
										driverType = video::EDT_SOFTWARE;
									} else if( choice == L"null" ) {
										driverType = video::EDT_NULL;
									} else {
										std::wcerr << L"Warning: Selected driver type " << choice << L" not recognized. Trying OpenGL." << std::endl;
										choice = L"opengl";
									}

									if( !device->isDriverSupported( driverType ) ) {
										std::wcerr << L"Warning: Chosen driver type " << choice << L" is not supported on this system. ";

										if( device->isDriverSupported( video::EDT_OPENGL ) ) {
											std::wcerr << L"Trying OpenGL." << std::endl;
											driverType = video::EDT_OPENGL;
											choice = L"opengl";
										} else if( device->isDriverSupported( video::EDT_DIRECT3D9 ) ) {
											std::wcerr << L"Trying Direct3D 9." << std::endl;
											driverType = video::EDT_DIRECT3D9;
											choice = L"direct3d9";
										} else if( device->isDriverSupported( video::EDT_DIRECT3D8 ) ) {
											std::wcerr << L"Trying Direct3D 8." << std::endl;
											driverType = video::EDT_DIRECT3D8;
											choice = L"direct3d8";
										} else if( device->isDriverSupported( video::EDT_BURNINGSVIDEO ) ) {
											std::wcerr << L"Trying Burning's video." << std::endl;
											driverType = video::EDT_BURNINGSVIDEO;
											choice = L"burning's video";
										} else if( device->isDriverSupported( video::EDT_SOFTWARE ) ) {
											std::wcerr << L"Trying software renderer." << std::endl;
											driverType = video::EDT_SOFTWARE;
											choice = L"software";
										} else {
											std::wcerr << L"Error: No graphical output driver types are available. Using NULL type!! Also enabling debug." << std::endl;
											driverType = video::EDT_NULL;
											choice = L"NULL";
											debug = true;
										}
									}

									if( debug ) {
										std::wcout << L"Driver type is " << choice << std::endl;
									}

								} else if( preference == L"number of players:" ) {
									try {
										uint_least8_t choiceAsInt = boost::lexical_cast< short int >( choice ); //uint_least8_t is typedef'd as a kind of char apparently, at least on my raspberry pi, and Boost lexical_cast() won't convert from wchar_t to char.

										if( choiceAsInt > UINT_LEAST8_MAX ) { //The maximum number of players is whatever a uint_least8_t can hold, presumably 255 (the 8 is the minimum number of bits, it may not be the true number).
											choiceAsInt = UINT_LEAST8_MAX;
										}

										if( choiceAsInt <= 4 && choiceAsInt > 0 ) {
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
									} catch( boost::bad_lexical_cast e ) {
										std::wcerr << L"Error reading number of players preference (is it not a number?) on line " << lineNum << L": " << e.what() << std::endl;
									}

								} else if( preference == L"window size:" ) {
									size_t locationOfX = choice.find( L"x" );
									std::wstring width = choice.substr( 0, locationOfX );
									std::wstring height = choice.substr( locationOfX + 1 );
									if( debug ) {
										std::wcout << L"Window size: " << width << L"x" << height << std::endl;
									}

									uint_fast16_t widthAsInt = boost::lexical_cast< uint_fast16_t >( width );
									uint_fast16_t heightAsInt = boost::lexical_cast< uint_fast16_t >( height );

									if( widthAsInt < 160 || heightAsInt < 240 ) {
										std::wcerr << L"Error reading window size: Width and/or height are really really tiny. Sorry but you'll have to recompile the game yourself if you want a window that small." << std::endl;
									} else if( widthAsInt == 160 && heightAsInt == 240 ) {
										std::wcout << L"Rock on, CGA graphics. Rock on." << std::endl;
										windowSize = core::dimension2d<uint_least16_t>( widthAsInt, heightAsInt );
									} else {
										windowSize = core::dimension2d<uint_least16_t>( widthAsInt, heightAsInt );
									}

								} else if( preference == L"play music:" ) {
									if( choice == L"true" ) {
										if( debug ) {
											std::wcout << L"Play music is ON" << std::endl;
										}
										playMusic = true;
									} else if( choice == L"false" ) {
										if( debug ) {
											std::wcout << L"Play music is OFF" << std::endl;
										}
										playMusic = false;
									} else {
										std::wcerr << L"Error reading play music preference on line " << lineNum << L": \"" << choice << L"\"" << std::endl;
									}

								} else if( preference == L"network port:" ) {
									if( debug ) {
										std::wcout << L"Network port: " << choice << std::endl;
									}

									try {
										uint_fast16_t choiceAsInt = boost::lexical_cast< uint_fast16_t >( choice );
										network.setPort( choiceAsInt );
									} catch( boost::bad_lexical_cast e ) {
										std::wcerr << L"Error reading network port (is it not a number?) on line " << lineNum << L": " << e.what() << std::endl;
									}
								} else if( preference == L"enable joystick:" ) {
									if( choice == L"true" ) {
										if( debug ) {
											std::wcout << L"Joystick is ENABLED" << std::endl;
										}
										enableJoystick = true;
									} else if( choice == L"false" ) {
										if( debug ) {
											std::wcout << L"Joystick is DISABLED" << std::endl;
										}
										enableJoystick = false;
									} else {
										std::wcerr << L"Error reading enable joystick preference on line " << lineNum << L": \"" << choice << L"\"" << std::endl;
									}
								} else if( preference == L"joystick number:" ) {
									if( debug ) {
										std::wcout << L"Joystick number: " << choice << std::endl;
									}
									try {
										uint_fast16_t choiceAsInt = boost::lexical_cast< uint_fast16_t >( choice );
										joystickChosen = choiceAsInt;
									} catch (boost::bad_lexical_cast e ) {
										std::wcerr << L"Error reading joystick number (is it not a number?) on line " << lineNum << L": " << e.what() << std::endl;
									}
								} else if( preference == L"always server:" ) {
									if( choice == L"true" ) {
										if( debug ) {
											std::wcout << L"This is always a server" << std::endl;
										}
										isServer = true;
									} else if( choice == L"false" ) {
										if( debug ) {
											std::wcout << L"This is not always a server" << std::endl;
										}
										isServer = false;
									} else {
										std::wcerr << L"Error reading \"always server\" preference on line " << lineNum << L": \"" << choice << L"\"" << std::endl;
									}
								} else {
									std::wcerr << L"Unrecognized preference on line " << lineNum << L": \"" << line << L"\"" << std::endl;
								}
							} catch ( std::exception e ) {
								std::wcout << L"Error: " << e.what() << L". Does line " << lineNum << L" not have a tab character separating preference and value? The line says " << line << std::endl;
							}
						}
					}

					prefsFile.close();
				} else {
					throw( std::wstring( L"Cannot open prefs file." ) );
				}
			} else {
				throw( std::wstring( L"Prefs file is a directory. Cannot load prefs." ) );
			}
		} else {
			if( debug ) {
				std::wcout << L"Creating preferences file " << prefsPath.wstring() << std::endl;
			}
			boost::filesystem::wofstream prefsFile;
			prefsFile.open( prefsPath );

			if( prefsFile.is_open() ) {
				prefsFile << L"volume:\t" << musicVolume << std::endl;
				prefsFile << L"number of bots:\t" << numBots << std::endl;

				prefsFile << L"show background animations:\t";
				if( showBackgrounds ) {
					prefsFile << L"true";
				} else {
					prefsFile << L"false";
				}
				prefsFile << std::endl;

				prefsFile << L"fullscreen:\t";
				if( fullscreen ) {
					prefsFile << L"true";
				} else {
					prefsFile << L"false";
				}
				prefsFile << std::endl;

				prefsFile << L"mark player trails:\t";
				if( markTrails ) {
					prefsFile << L"true";
				} else {
					prefsFile << L"false";
				}
				prefsFile << std::endl;

				prefsFile << L"debug:\t";
				if( debug ) {
					prefsFile << L"true";
				} else {
					prefsFile << L"false";
				}
				prefsFile << std::endl;

				prefsFile << L"bits per pixel:\t" << bitsPerPixel << std::endl;

				prefsFile << L"wait for vertical sync:\t";
				if( vsync ) {
					prefsFile << L"true";
				} else {
					prefsFile << L"false";
				}
				prefsFile << std::endl;

				prefsFile << L"driver type:\t";
				if( driverType == video::EDT_OPENGL ) {
					prefsFile << L"opengl";
				} else if( driverType == video::EDT_DIRECT3D9 ) {
					prefsFile << L"direct3d9";
				} else if( driverType == video::EDT_DIRECT3D8 ) {
					prefsFile << L"direct3d8";
				} else if( driverType == video::EDT_BURNINGSVIDEO ) {
					prefsFile << L"burning's video";
				} else if( driverType == video::EDT_SOFTWARE ) {
					prefsFile << L"software";
				} else if( driverType == video::EDT_NULL ) {
					prefsFile << L"null";
				} else {
					std::wcerr << L"Warning: Creating preferences file, selected driver type " << driverType << L" not recognized. Saving OpenGL." << std::endl;
					prefsFile << L"opengl";
				}
				prefsFile << std::endl;

				prefsFile << L"number of players:\t" << numPlayers << std::endl;

				prefsFile << L"window size:\t" << windowSize.Width << L"x" << windowSize.Height << std::endl;

				prefsFile << L"play music:\t";
				if( playMusic ) {
					prefsFile << L"true";
				} else {
					prefsFile << L"false";
				}
				prefsFile << std::endl;

				prefsFile << L"network port:\t" << network.getPort() << std::endl;

				prefsFile << L"enable joystick:\t";
				if( enableJoystick ) {
					prefsFile << L"true";
				} else {
					prefsFile << L"false";
				}
				prefsFile << std::endl;

				prefsFile << L"joystick number:\t" << joystickChosen << std::endl;

				prefsFile << L"always server:\t";
				if( isServer ) {
					prefsFile << L"true";
				} else {
					prefsFile << L"false";
				}
				prefsFile << std::endl;

			}
		}

		if( !isServer ) {
			std::wcout << L"(S)erver or (c)lient? " << std::endl;
			wchar_t a;
			std::wcin >> a;
			isServer = ( a == L's' || a == L'S' );
			myPlayer = 0;
		}
	} catch( std::exception e ) {
		std::wcerr << L"Error in GameManager::readPrefs(): " << e.what() << std::endl;
	} catch( std::wstring e ) {
		std::wcerr << L"Error in GameManager::readPrefs(): " << e << std::endl;
	}
}

//Resets miscellaneous stuff between mazes
void GameManager::resetThings() {
	try {
		randomSeed = time( NULL );
		loadingDelay = 1000 + ( rand() % 5000 );

		winners.clear();
		stuff.clear();
		showingMenu = false;
		numKeysFound = 0;
		numLocks = 0;
		donePlaying = false;

		for( uint_fast8_t p = 0; p < numPlayers; p++ ) {
			playerStart.at( p ).reset();
			player.at( p ).stepsTaken = 0;
		}

		for( uint_fast16_t i = 0; i < stuff.size(); i++ ) {
			stuff.at( i ).loadTexture( driver );
		}

		for( uint_fast8_t x = 0; x < mazeManager.cols; x++ ) {
			for( uint_fast8_t y = 0; y < mazeManager.rows; y++ ) {
				mazeManager.maze[ x ][ y ].visited = false;
			}
		}

		won = false;
		bgscene->clear();

		if( showBackgrounds ) {
			setupBackground();
		}
		timer->setTime( 0 );
		timer->start();
		startLoadingScreen();

		currentProTip = ( currentProTip + 1 ) % proTips.size();

		loadTipFont();
	} catch( std::exception e ) {
		std::wcerr << L"Error in GameManager::resetThings(): " << e.what() << std::endl;
	}
}

//Should only be called by main() in main.cpp
int GameManager::run() {
	try {
		while( device->run() && !donePlaying ) {
			newMaze();

			while( device->run() && !won && !donePlaying ) {

				if( showingLoadingScreen && ( timer->getRealTime() > timeStartedLoading + loadingDelay ) ) {
					showingLoadingScreen = false;
				}

				if( driver->getFPS() > 60 ) {
					/*These next 3 lines are for limiting processor usage. I really
					 *doubt they're all needed, so comment one or another if you
					 *experience slowness. Two are provided by Irrlicht and the other by SDL.*/
					//device->yield();
					device->sleep( 17 ); //17 = 1/60 of a second, rounded up, in milliseconds. My monitor refreshes at 60 Hz.
					//SDL_Delay( 17 );
				}

				if( driver->getScreenSize() != windowSize ) { //If the window has been resized. Only here until Irrlicht implements proper window resize events.
					SEvent temp;
					temp.EventType = EET_USER_EVENT;
					temp.UserEvent.UserData1 = USER_EVENT_WINDOW_RESIZE;
					device->postEventFromUser( temp );
				}

				if( playMusic && !Mix_PlayingMusic() ) {
					loadNextSong();
				}

				if(( !showingLoadingScreen && device->isWindowActive() ) || debug ) {
					//It's the bots' turn to move now.
					if( !( showingMenu || showingLoadingScreen ) && numBots > 0 ) {
						for( uint_fast8_t i = 0; i < numBots; i++ ) {
							if( debug || allHumansAtGoal() || bot.at( i ).lastTimeMoved < timer->getRealTime() - bot.at( i ).movementDelay ) {
								bot.at( i ).move();
							}
						}
					}

					device->getCursorControl()->setVisible( showingMenu || showingLoadingScreen || debug );
					drawAll();

					//Check if any of the players have landed on a collectable item
					for( uint_fast8_t p = 0; p < numPlayers; p++ ) {
						for( uint_fast16_t s = 0; s < stuff.size(); s++ ) {
							if( player.at( p ).getX() == stuff.at( s ).getX() && player.at( p ).getY() == stuff.at( s ).getY() ) {
								switch( stuff.at( s ).getType() ) {
									case COLLECTABLE_KEY: {
										numKeysFound++;
										stuff.erase( stuff.begin() + s );

										if( numKeysFound >= numLocks ) {
											for( uint_fast8_t c = 0; c < mazeManager.cols; c++ ) {
												for( uint_fast8_t r = 0; r < mazeManager.rows; r++ ) {
													mazeManager.maze[c][r].removeLocks();
												}
											}

											for( uint_fast8_t b = 0; b < numBots; b++ ) {
												bot.at( b ).allKeysFound();
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


					for( uint_fast8_t p = 0; p < numPlayers; p++ ) {
						if( ( player.at( p ).getX() == goal.getX() ) && player.at( p ).getY() == goal.getY() ) { //Make a list of who finished in what order
							bool alreadyFinished = false; //Indicates whether the player is already on the winners list

							for( uint_fast8_t i = 0; i < winners.size() && !alreadyFinished; i++ ) {
								if( p == winners.at( i ) ) {
									alreadyFinished = true;
								}
							}

							if( !alreadyFinished ) {
								winners.push_back( p );
							}
						}
					}

					won = ( winners.size() >= numPlayers ); //If all the players are on the winners list, we've won.

				} else {
					showingMenu = true;
					device->yield();
				}

				if( showingMenu && !timer->isStopped() ) {
					timer->stop();
				} else if( !showingMenu && timer->isStopped() ) {
					timer->start();
				}

				if( isServer ) {
					if( network.checkForConnections() != 0 ) {
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

				//TODO: add networking stuff here
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
				}
			}

			timer->stop();

			if( !donePlaying ) {
				if( debug ) {
					std::wcout << L"On to the next level!" << std::endl;
					std::wcout << L"Winners:";

					for( uint_fast8_t i = 0; i < winners.size(); i++ ) {
						std::wcout << L" " << winners.at( i );
					}

					std::wcout << std::endl;
				}
				startLoadingScreen();
			}
		}
	} catch( std::exception e ) {
		std::wcerr << L"Error in GameManager::run(): " << e.what() << std::endl;
		return -1;
	}
	return 0;
}

void GameManager::setControls() {
	try {
		boost::filesystem::path controlsPath( L"./controls.cfg" );
		/*uint_least8_t nonPlayerActions = 3; //The number of keys assigned to things other than controlling the player objects: screenshots, opening & closing the menu, etc.
		keyMap.resize( 4 * ( numPlayers - numBots ) + nonPlayerActions );

		//set defaults
		for( uint_fast8_t i = 0; i < keyMap.size() - nonPlayerActions; i += 4 ) {
			if ( debug ) {
				std::wcout << L"keyMap.size(): " << keyMap.size() << "\ti: " << i << std::endl;
			}
			keyMap.at( i ).setAction( 'u' );
			keyMap.at( i + 1 ).setAction( 'd' );
			keyMap.at( i + 2 ).setAction( 'l' );
			keyMap.at( i + 3 ).setAction( 'r' );
			keyMap.at( i ).setPlayer( i / 4 );
			keyMap.at( i + 1 ).setPlayer( i / 4 );
			keyMap.at( i + 2 ).setPlayer( i / 4 );
			keyMap.at( i + 3 ).setPlayer( i / 4 );
			if( debug ) {
				std::wcout << L"Set keyMap " << i << L" player to " << i / 4 << std::endl;
			}
		}

		if( numPlayers - numBots > 0 ) {
			keyMap.at( 0 ).setKey( KEY_UP );
			keyMap.at( 0 + 1 ).setKey( KEY_DOWN );
			keyMap.at( 0 + 2 ).setKey( KEY_LEFT );
			keyMap.at( 0 + 3 ).setKey( KEY_RIGHT );
		}

		if ( debug ) {
			std::wcout << L"Set arrow keys for player 0, now setting snapshot, menu, and escape keys" << std::endl;
		}

		keyMap.at( 4 * ( numPlayers - numBots ) ).setAction( 's' );
		keyMap.at( 4 * ( numPlayers - numBots ) ).setKey( KEY_SNAPSHOT );

		keyMap.at( 4 * ( numPlayers - numBots ) + 1 ).setAction( 'm' );
		keyMap.at( 4 * ( numPlayers - numBots ) + 1 ).setKey( KEY_MENU );
		keyMap.at( 4 * ( numPlayers - numBots ) + 2 ).setAction( 'm' );
		keyMap.at( 4 * ( numPlayers - numBots ) + 2 ).setKey( KEY_ESCAPE );*/

		if( exists( controlsPath ) ) {
			if( !is_directory( controlsPath ) ) {
				if( debug ) {
					std::wcout << L"Loading controls from file " << controlsPath.wstring() << std::endl;
				}
				boost::filesystem::wifstream controlsFile;
				controlsFile.open( controlsPath );

				if( controlsFile.is_open() ) {
					std::wstring line;
					uint_fast8_t lineNum = 0;

					while( controlsFile.good() ) {
						lineNum++;
						getline( controlsFile, line );
						line = line.substr( 0, line.find( L"//" ) ); //Filters out comments
						boost::algorithm::trim( line ); //Removes trailing and leading spaces
						boost::algorithm::to_lower( line );
						if( debug ) {
							std::wcout << L"Line " << lineNum << L": \"" << line << "\"" << std::endl;
						}


						if( !line.empty() ) {
							try {
								std::wstring preference = boost::algorithm::trim_copy( line.substr( 0, line.find( L'\t' ) ) );
								std::wstring choice = boost::algorithm::trim_copy( line.substr( line.find( L'\t' ) ) );
								if( debug ) {
									std::wcout << L"Preference \"" << preference << L"\" choice \"" << choice << L"\""<< std::endl;
								}
								if( preference == L"menu" ) {
								} else if( preference == L"screenshot" ) {
								} else {
									std::wstring playerNumStr = boost::algorithm::trim_copy( preference.substr( 0, preference.find( L' ' ) ) );
									std::wstring action = boost::algorithm::trim_copy( preference.substr( preference.find( L' ' ) ) );
								}
							} catch( std::exception e ) {
								std::wcerr << L"Error in GameManager::setControls(): " << e.what() << std::endl;
							} catch( std::wstring e ) {
								std::wcerr << L"Error in GameManager::setControls(): " << e << std::endl;
							}
						}
					}

					controlsFile.close();
				} else {
					throw( std::wstring( L"controls.cfg cannot be opened." ) );
				}
			} else {
				throw( std::wstring( L"controls.cfg is a directory, should be a file." ) );
			}
		} else {
			throw( std::wstring( L"controls.cfg does not exist." ) );
		}
	} catch( std::exception e ) {
		std::wcerr << L"Error in GameManager::setControls(): " << e.what() << std::endl;
	} catch( std::wstring e ) {
		std::wcerr << L"Error in GameManager::setControls(): " << e << std::endl;
	}
}

void GameManager::setupBackground() {
	try {
		uint_least8_t availableBackgrounds = 1; //The number of different background animations to choose from

		backgroundChosen = rand() % availableBackgrounds;
		if( debug ) {
			std::wcout << L"Background chosen: " << backgroundChosen << std::endl;
		}

		switch( backgroundChosen ) {
			case 0: {
				// create a particle system
				scene::ICameraSceneNode* camera = bgscene->addCameraSceneNode();
				camera->setPosition( core::vector3df( 0, 0, -150 ) );
				scene::IParticleSystemSceneNode* ps = bgscene->addParticleSystemSceneNode( false );

				scene::IParticleEmitter* em = ps->createBoxEmitter(
												  camera->getViewFrustum()->getBoundingBox(), //core::aabbox3d< float >(-7,-7,-7,7,7,7), // emitter size
												  core::vector3df( 0.0f, 0.0f, -0.1f ), // initial direction
												  100, 500,                            // Min & max emit rate
												  CYAN,       // darkest color
												  WHITE,       // brightest color
												  2000, 20000, 0,                       // min and max age, angle
												  core::dimension2df( 1.f, 1.f ),      // min size
												  core::dimension2df( 20.f, 20.f ) );    // max size

				ps->setEmitter( em ); // this grabs the emitter
				em->drop(); // so we can drop it here without deleting it

				//scene::IParticleAffector* paf = ps->createFadeOutParticleAffector();

				//ps->addAffector(paf); // same goes for the affector
				//paf->drop();

				ps->setPosition( core::vector3df( 0, 0, 40 ) );
				ps->setScale( core::vector3df( 1, 1, 1 ) );
				ps->setMaterialFlag( video::EMF_LIGHTING, false );
				ps->setMaterialFlag( video::EMF_ZWRITE_ENABLE, false );
				//ps->setMaterialTexture( 0, driver->getTexture( "star.png" ) );
				ps->setMaterialType( video::EMT_TRANSPARENT_ALPHA_CHANNEL );

				video::IImage* pixelImage = driver->createImage( video::ECF_A8R8G8B8, core::dimension2d< u32 >( 1, 1 ) );
				//pixelImage->fill( WHITE );
				pixelImage->setPixel( 0, 0, WHITE, false ); //Which is faster on a 1x1 pixel image: setPixel() or fill()?
				video::ITexture* pixelTexture = driver->addTexture( "pixel", pixelImage );
				ps->setMaterialTexture( 0, pixelTexture );
			}
			break;
			default:
				break;
		}
	} catch( std::exception e ) {
		std::wcerr << L"Error in GameManager::setupBackground(): " << e.what() << std::endl;
	}
}

void GameManager::startLoadingScreen() {
	try {
		showingLoadingScreen = true;
		timeStartedLoading = timer->getRealTime();
		drawLoadingScreen();
	} catch( std::exception e ) {
		std::wcerr << L"Error in GameManager::startLoadingScreen(): " << e.what() << std::endl;
	}
}

void GameManager::takeScreenShot() {
	try {
		irr::video::IImage* image = driver->createScreenShot();

		if( image ) {
			irr::core::stringw filename;
			filename.append( L"screenshot_" );

			time_t currentTime = time( NULL );
			wchar_t clockTime[ 20 ];
			wcsftime( clockTime, 20, L"%Y-%m-%d %H:%M:%S", localtime( &currentTime ) );
			filename.append( clockTime );
			filename.append( L".png" );

			if( !driver->writeImageToFile( image, filename ) ) {
				throw( std::wstring( L"Failed to save screen shot to file " + stringConverter.toStdWString( filename ) ) );
			} else if( debug ) {
				std::wcout << L"Screen shot saved as " << filename.c_str() << std::endl;
			}

			image->drop();
		} else {
			throw( std::wstring( L"takeScreenShot(): Failed to take screen shot" ) );
		}
	} catch( std::exception e ) {
		std::wcerr << L"Error in GameManager::takeScreenShot(): " << e.what() << std::endl;
	}
}
