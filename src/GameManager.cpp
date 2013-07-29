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
#include <taglib/fileref.h>
#include <SDL/SDL.h>

#if defined WINDOWS //Networking stuff
#include <winsock.h>
#elif defined LINUX
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
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

using namespace std;
using namespace irr;
//using boost::asio::ip::tcp;


bool GameManager::allHumansAtGoal() {
	vector< uint8_t > humanPlayers; //Get a list of players

	for( uint8_t p = 0; p < numPlayers; p++ ) {
		humanPlayers.push_back( p );
	}

	bool result = false;

	for( uint8_t b = 0; b < numBots; b++ ) { //Remove bots from the list
		uint8_t botPlayer = bot[ b ].getPlayer();

		for( uint32_t p = 0; p < humanPlayers.size(); p++ ) {
			if( humanPlayers[ p ] == botPlayer ) {
				humanPlayers.erase( humanPlayers.begin() + p );
			}
		}
	}

	if( humanPlayers.size() > 0 ) {
		result = true;

		for( uint32_t p = 0; ( p < humanPlayers.size() && result == true ); p++ ) {
			if( !( player[ humanPlayers[ p ] ].getX() == goal.getX() && player[ humanPlayers[ p ] ].getY() == goal.getY() ) ) {
				result = false;
			}
		}

	}

	return result;
}

void GameManager::drawAll() {
	driver->beginScene();

	if( !showingLoadingScreen ) {
		if( showBackgrounds ) {
			drawBackground();
		}

		//Draws player trails ("footprints")
		if( markTrails ) {
			for( uint8_t x = 0; x < mazeManager.cols; x++ ) { //It's inefficient to do this here and have similar nested loops below drawing the walls, but I want these drawn before the players, and the players drawn before the walls.
				for( uint8_t y = 0; y < mazeManager.rows; y++ ) {
					if( mazeManager.maze[ x ][ y ].visited ) {
						int32_t dotSize = cellWidth / 5;

						if( dotSize < 2 ) {
							dotSize = 2;
						}

						driver->draw2DRectangle( mazeManager.maze[ x ][ y ].getVisitorColor() , core::rect<int32_t>( core::position2d<int32_t>(( x * cellWidth ) + ( 0.5 * cellWidth ) - ( 0.5 * dotSize ), ( y * cellHeight ) + ( 0.5 * cellHeight ) - ( 0.5 * dotSize ) ), core::dimension2d<int32_t>( dotSize, dotSize ) ) );
					}
				}
			}
		}

		for( uint32_t ps = 0; ps < playerStart.size(); ps++ ) { //Put this in a separate loop from the players (below) so that the players would all be drawn after the playerStarts.
			playerStart[ ps ].draw( driver, cellWidth, cellHeight );
		}

		//Because of a texture resizing bug in Irrlicht's software renderers, we draw the items (currently all keys, which use a large png as a texture) before the players (which generate their own texture at the correct size) and the maze walls.
		for( uint32_t i = 0; i < stuff.size(); i++ ) {
			stuff[i].draw( driver, cellWidth, cellHeight );
		}

		//Drawing bots before human players makes it easier to play against large numbers of bots
		for( uint8_t i = 0; i < numBots; i++ ) {
			player[ bot[ i ].getPlayer() ].draw( driver, cellWidth, cellHeight );
		}

		for( uint8_t p = 0; p < numPlayers; p++ ) {
			if( player[ p ].isHuman ) {
				player[ p ].draw( driver, cellWidth, cellHeight );
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


		uint32_t spaceBetween = windowSize.Height / 30;
		uint32_t textY = spaceBetween;

		time_t currentTime = time( NULL );
		wchar_t clockTime[9];
		wcsftime( clockTime, 9, L"%H:%M:%S", localtime( &currentTime ) );
		core::dimension2d<uint32_t> tempDimensions = clockFont->getDimension( core::stringw( clockTime ).c_str() );
		core::rect<int32_t> tempRectangle( viewportSize.Width + 1, textY, tempDimensions.Width + ( viewportSize.Width + 1 ), tempDimensions.Height + textY );
		clockFont->draw( clockTime, tempRectangle, LIGHTMAGENTA, true, true, &tempRectangle );

		core::stringw timeLabel( L"Time:" );
		textY += tempDimensions.Height;
		tempDimensions = textFont->getDimension( timeLabel.c_str() );
		tempRectangle = core::rect<int32_t>( viewportSize.Width + 1, textY, tempDimensions.Width + ( viewportSize.Width + 1 ), tempDimensions.Height + textY );
		textFont->draw( L"Time:", tempRectangle, YELLOW, true, true, &tempRectangle );
		core::stringw timerStr( "" );
		timerStr += ( timer->getTime() / 1000 );
		timerStr += L" seconds";
		textY += tempDimensions.Height;
		tempDimensions = textFont->getDimension( timerStr.c_str() );
		tempRectangle = core::rect<int32_t>( viewportSize.Width + 1, textY, tempDimensions.Width + ( viewportSize.Width + 1 ), tempDimensions.Height + textY );
		textFont->draw( timerStr, tempRectangle, YELLOW, true, true, &tempRectangle );

		core::stringw keysFoundStr( L"Keys found:" );
		textY += tempDimensions.Height;
		tempDimensions = textFont->getDimension( keysFoundStr.c_str() );
		tempRectangle = core::rect<int32_t>( viewportSize.Width + 1, textY, tempDimensions.Width + ( viewportSize.Width + 1 ), tempDimensions.Height + textY );
		textFont->draw( keysFoundStr, tempRectangle, YELLOW, true, true, &tempRectangle );

		core::stringw keyStr;
		keyStr += numKeysFound;
		keyStr += L"/";
		keyStr += numLocks;
		textY += tempDimensions.Height;
		tempDimensions = textFont->getDimension( keyStr.c_str() );
		tempRectangle = core::rect<int32_t>( viewportSize.Width + 1, textY, tempDimensions.Width + ( viewportSize.Width + 1 ), tempDimensions.Height + textY );
		textFont->draw( keyStr, tempRectangle, YELLOW, true, true, &tempRectangle );

		core::stringw seedLabel( L"Random seed:" );
		textY += tempDimensions.Height;
		tempDimensions = textFont->getDimension( seedLabel.c_str() );
		tempRectangle = core::rect<int32_t>( viewportSize.Width + 1, textY, tempDimensions.Width + ( viewportSize.Width + 1 ), tempDimensions.Height + textY );
		textFont->draw( seedLabel, tempRectangle, YELLOW, true, true, &tempRectangle );

		core::stringw seedStr( randomSeed );
		textY += tempDimensions.Height;
		tempDimensions = textFont->getDimension( seedStr.c_str() );
		tempRectangle = core::rect<int32_t>( viewportSize.Width + 1, textY, tempDimensions.Width + ( viewportSize.Width + 1 ), tempDimensions.Height + textY );
		textFont->draw( seedStr, tempRectangle, YELLOW, true, true, &tempRectangle );

		core::stringw headfor( L"Head for" );
		textY += tempDimensions.Height;
		tempDimensions = textFont->getDimension( headfor.c_str() );

		if( textY < (( windowSize.Height / 2 ) - tempDimensions.Height ) ) {
			textY = (( windowSize.Height / 2 ) - tempDimensions.Height );
		}

		if( numKeysFound >= numLocks ) {
			tempRectangle = core::rect<int32_t>( viewportSize.Width + 1, textY, tempDimensions.Width + ( viewportSize.Width + 1 ), tempDimensions.Height + textY );
			textFont->draw( headfor, tempRectangle, LIGHTMAGENTA, true, true, &tempRectangle );
		}

		core::stringw theexit( L"the exit!" );
		textY += tempDimensions.Height;
		tempDimensions = textFont->getDimension( theexit.c_str() );

		if( numKeysFound >= numLocks ) {
			tempRectangle = core::rect<int32_t>( viewportSize.Width + 1, textY, tempDimensions.Width + ( viewportSize.Width + 1 ), tempDimensions.Height + textY );
			textFont->draw( theexit, tempRectangle, LIGHTCYAN, true, true, &tempRectangle );
		}

		if( playMusic ) {
			core::stringw nowplaying( L"Now playing:" );
			textY += tempDimensions.Height;
			tempDimensions = textFont->getDimension( nowplaying.c_str() );
			tempRectangle = core::rect<int32_t>( viewportSize.Width + 1, textY, tempDimensions.Width + ( viewportSize.Width + 1 ), tempDimensions.Height + textY );
			textFont->draw( nowplaying, tempRectangle, YELLOW, true, true, &tempRectangle );

			textY += tempDimensions.Height;
			tempDimensions = musicTagFont->getDimension( musicTitle.c_str() );
			tempRectangle = core::rect<int32_t>( viewportSize.Width + 1, textY, tempDimensions.Width + ( viewportSize.Width + 1 ), tempDimensions.Height + textY );
			musicTagFont->draw( musicTitle, tempRectangle, LIGHTGREEN, true, true, &tempRectangle );

			core::stringw by( L"by" );
			textY += tempDimensions.Height;
			tempDimensions = textFont->getDimension( by.c_str() );
			tempRectangle = core::rect<int32_t>( viewportSize.Width + 1, textY, tempDimensions.Width + ( viewportSize.Width + 1 ), tempDimensions.Height + textY );
			textFont->draw( by, tempRectangle, YELLOW, true, true, &tempRectangle );

			textY += tempDimensions.Height;
			tempDimensions = musicTagFont->getDimension( musicArtist.c_str() );
			tempRectangle = core::rect<int32_t>( viewportSize.Width + 1, textY, tempDimensions.Width + ( viewportSize.Width + 1 ), tempDimensions.Height + textY );
			musicTagFont->draw( musicArtist, tempRectangle, LIGHTGREEN, true, true, &tempRectangle );

			core::stringw fromalbum( L"from album" );
			textY += tempDimensions.Height;
			tempDimensions = textFont->getDimension( fromalbum.c_str() );
			tempRectangle = core::rect<int32_t>( viewportSize.Width + 1, textY, tempDimensions.Width + ( viewportSize.Width + 1 ), tempDimensions.Height + textY );
			textFont->draw( fromalbum, tempRectangle, YELLOW, true, true, &tempRectangle );

			textY += tempDimensions.Height;
			tempDimensions = musicTagFont->getDimension( musicAlbum.c_str() );
			tempRectangle = core::rect<int32_t>( viewportSize.Width + 1, textY, tempDimensions.Width + ( viewportSize.Width + 1 ), tempDimensions.Height + textY );
			musicTagFont->draw( musicAlbum, tempRectangle, LIGHTGRAY, true, true, &tempRectangle );

			core::stringw volume( L"Volume:" );
			textY += tempDimensions.Height;
			tempDimensions = textFont->getDimension( volume.c_str() );
			tempRectangle = core::rect<int32_t>( viewportSize.Width + 1, textY, tempDimensions.Width + ( viewportSize.Width + 1 ), tempDimensions.Height + textY );
			textFont->draw( volume, tempRectangle, YELLOW, true, true, &tempRectangle );

			core::stringw volumeNumber( musicVolume );
			volumeNumber.append( L"%" );
			textY += tempDimensions.Height;
			tempDimensions = textFont->getDimension( volumeNumber.c_str() );
			tempRectangle = core::rect<int32_t>( viewportSize.Width + 1, textY, tempDimensions.Width + ( viewportSize.Width + 1 ), tempDimensions.Height + textY );
			textFont->draw( volumeNumber, tempRectangle, LIGHTRED, true, true, &tempRectangle );
		}

		gui->drawAll();
	} else {
		core::dimension2d<uint32_t> tempDimensions = loadingFont->getDimension( loading.c_str() );
		uint32_t textY = 0;
		core::rect< int32_t > tempRectangle(( windowSize.Width / 2 ) - ( tempDimensions.Width / 2 ), textY, ( windowSize.Width / 2 ) + ( tempDimensions.Width / 2 ), tempDimensions.Height + textY );
		loadingFont->draw( loading, tempRectangle, YELLOW, true, true, &tempRectangle );

		textY = tempDimensions.Height + textY + 1;
		tempDimensions = tipFont->getDimension( proTipPrefix.c_str() );
		tempRectangle = core::rect< int32_t >( 0, textY, tempDimensions.Width, tempDimensions.Height + textY );
		tipFont->draw( proTipPrefix, tempRectangle, LIGHTCYAN, true, true, &tempRectangle );

		uint32_t textX = tempDimensions.Width + 1;
		tempDimensions = tipFont->getDimension( proTips.at( currentProTip ).c_str() );
		tempRectangle = core::rect< int32_t >( textX, textY, tempDimensions.Width + textX, tempDimensions.Height + textY );
		tipFont->draw( proTips.at( currentProTip ), tempRectangle, WHITE, true, true, &tempRectangle );
	}

	driver->endScene();
}

void GameManager::drawBackground() {
	switch( backgroundChosen ) {
		case 0: {
			bgscene->drawAll();
		}
		break;
		default:
			break;
	}
}

//I... am... DESTRUCTOOOOOOORRRRR!!!!!!!!!
GameManager::~GameManager() {
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
}

GameManager::GameManager() {
	network.setGameManager( this );
	mazeManager.setGameManager( this );
	isServer = false;
	antiAliasFonts = true;
	currentProTip = 0;
	sideDisplaySizeDenominator = 6; //What fraction of the screen's width is set aside for displaying text, statistics, etc. during play.

	fontFile = "";
	boost::filesystem::recursive_directory_iterator end;
	for( boost::filesystem::recursive_directory_iterator i(L"./"); i != end; i++ ) {
		if( !is_directory( i->path() ) && boost::iequals( i->path().extension().generic_wstring(), L".ttf" ) ) {
			fontFile = i->path().c_str();
			break;
		}
	}

	device = createDevice( video::EDT_NULL ); //Must create a null device before calling readPrefs();

	if( !device ) {
		wcerr << L"Error: Cannot create null device. Something is definitely wrong here!" << endl;
		exit( -1 );
	}

	readPrefs();

	uint8_t nonPlayerActions = 0;
	keyMap.resize( 4 * ( numPlayers - numBots ) + nonPlayerActions );
	for( uint8_t i = 0; i < 4 * ( numPlayers - numBots ); i += 4 ) {
		keyMap.at( i ).setAction( 'u' );
		keyMap.at( i + 1 ).setAction( 'd' );
		keyMap.at( i + 2 ).setAction( 'l' );
		keyMap.at( i + 3 ).setAction( 'r' );
		keyMap.at( i ).setPlayer( i / 4 );
		keyMap.at( i + 1 ).setPlayer( i / 4 );
		keyMap.at( i + 2 ).setPlayer( i / 4 );
		keyMap.at( i + 3 ).setPlayer( i / 4 );
		if( debug ) {
			wcout << L"Set keyMap " << i << L" player to " << i / 4 << endl;
		}
	}

	if( fullscreen && !allowSmallSize ) {
		windowSize = device->getVideoModeList()->getDesktopResolution();
	}

	viewportSize.set( windowSize.Width - ( windowSize.Width / sideDisplaySizeDenominator ), windowSize.Height - 1 );

	bool sbuffershadows = false; //Would not be visible anyway since this game is 2D
	IEventReceiver* receiver = this;

	device->closeDevice(); //Signals to the null device that it needs to close itself on next run() so that we can create a non-null device
	device->run(); //This is next run()
	device->drop(); //Cleans up after the device

	device = createDevice( driverType, windowSize, bitsPerPixel, fullscreen, sbuffershadows, vsync, receiver ); //Most of these parameters were read from the preferences file

	if( !device ) {
		wcerr << L"Error: Cannot create device. Trying software renderer." << endl;
		device = createDevice( video::EDT_SOFTWARE, windowSize, bitsPerPixel, fullscreen, sbuffershadows, vsync, receiver );

		if( !device ) {
			wcerr << L"Error: Even the software renderer didn't work. Exiting." << endl;
			exit( -2 );
		}
	}
	device->setWindowCaption( L"Cybrinth" );

	if( debug ) {
		device->getLogger()->setLogLevel( ELL_INFORMATION );
	} else {
		device->getLogger()->setLogLevel( ELL_ERROR );
	}

	driver = device->getVideoDriver(); //Not sure if this would be possible with a null device, which is why we don't exit
	if( !driver ) {
		wcerr << L"Error: Cannot get video driver" << endl;
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
		wcerr << L"Error: Cannot get scene manager" << endl;
	}

	gui = device->getGUIEnvironment(); //Not sure if this would be possible with a null device, which is why we don't exit
	if( !gui ) {
		wcerr << L"Error: Cannot get GUI environment" << endl;
	} else {
		for( uint32_t i = 0; i < gui::EGDC_COUNT ; i++ ) {
			video::SColor guiSkinColor = gui->getSkin()->getColor( static_cast<gui::EGUI_DEFAULT_COLOR>( i ) );
			guiSkinColor.setAlpha( 255 );
			gui->getSkin()->setColor( static_cast<gui::EGUI_DEFAULT_COLOR>( i ), guiSkinColor );
		}
	}

	if( playMusic ) {

		if( SDL_Init( SDL_INIT_AUDIO ) == -1 ) {
			wcerr << L"Cannot initialize SDL audio." << endl;
			playMusic = false;
		}

		//Set the audio properties we hope to get: sample rate, channels, etc.
		int audio_rate = 44100; //44.1 KHz is the standard sample rate for CDs, and so makes a good 'lowest common denominator' for anything related to audio.
		Uint16 audio_format = AUDIO_S16SYS;
		int audio_channels = 2;
		int audio_buffers = 4096;

		if( Mix_OpenAudio( audio_rate, audio_format, audio_channels, audio_buffers ) != 0 ) {
			wcerr << L"Unable to initialize audio: " << Mix_GetError() << endl;
			playMusic = false;
		}

		if( debug ) {
			Mix_QuerySpec( &audio_rate, &audio_format, &audio_channels );//Don't assume we got everything we asked for above
			wcout << L"Audio sample rate: " << audio_rate << L" Hertz format: ";
			if( audio_format == AUDIO_U16SYS ) {
				wcout << L"AUDIO_U16SYS (equivalent to ";
				if( AUDIO_U16SYS == AUDIO_U16LSB ) {
					wcout << L"AUDIO_U16LSB)";
				} else if( AUDIO_U16SYS == AUDIO_U16MSB ) {
					wcout << L"AUDIO_U16MSB)";
				} else if( AUDIO_U16SYS == AUDIO_U16 ) {
					wcout << L"AUDIO_U16)";
				} else {
					wcout << L"unknown)";
				}
			} else if( audio_format == AUDIO_S16SYS ) {
				wcout << L"AUDIO_S16SYS (equivalent to ";
				if( AUDIO_S16SYS == AUDIO_S16LSB ) {
					wcout << L"AUDIO_S16LSB)";
				} else if( AUDIO_S16SYS == AUDIO_S16MSB ) {
					wcout << L"AUDIO_S16MSB)";
				} else if( AUDIO_S16SYS == AUDIO_S16 ) {
					wcout << L"AUDIO_S16)";
				} else {
					wcout << L"unknown)";
				}
			} else if( audio_format == AUDIO_U8 ) {
				wcout << L"AUDIO_U8";
			} else if( audio_format == AUDIO_S8 ) {
				wcout << L"AUDIO_S8";
			} else if( audio_format == AUDIO_U16LSB ) {
				wcout << L"AUDIO_U16LSB";
			} else if( audio_format == AUDIO_S16LSB ) {
				wcout << L"AUDIO_S16LSB";
			} else if( audio_format == AUDIO_U16MSB ) {
				wcout << L"AUDIO_U16MSB";
			} else if( audio_format == AUDIO_S16MSB ) {
				wcout << L"AUDIO_S16MSB";
			} else if( audio_format == AUDIO_U16 ) {
				wcout << L"AUDIO_U16";
			} else if( audio_format == AUDIO_S16 ) {
				wcout << L"AUDIO_S16";
			} else {
				wcout << L"unknown";
			}

			wcout << " channels: " << audio_channels  << L" buffers: " << audio_buffers << endl;
		}

		music = NULL;

		if( playMusic ) { //No sense in making the music list if we've been unable to initialize the audio.
			makeMusicList();
		}

		if( playMusic ) {  //playMusic may be set to false if makeMusicList() can't find any songs to play
			loadNextSong();
		}
	}

	/*cols = 0;
	rows = 0;
	resizeMaze( 1, 1 ); //This gets called here... why?*/

	loadFonts();
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

	loading = L"Loading...";
	proTipPrefix = L"Pro tip: ";
	stats = L"Player stats";
	steps = L"Steps";

	newGame.setY( 0 );
	loadMaze.setY( 1 * windowSize.Height / 5 );
	saveMaze.setY( 2 * windowSize.Height / 5 );
	exitGame.setY( 3 * windowSize.Height / 5 );
	backToGame.setY( 4 * windowSize.Height / 5 );

	//timer = device->getTimer();

	player.resize( numPlayers );
	playerStart.resize( numPlayers );

	if( numBots > numPlayers ) {
		numBots = numPlayers;
	}

	if( numBots <= numPlayers && numBots > 0 ) {
		bot.resize( numBots );

		for( uint8_t i = 0; i < numBots; i++ ) {
			bot[ i ].setPlayer( numPlayers - ( i + 1 ) ) ;
			player[ bot[ i ].getPlayer() ].isHuman = false;
		}
	}

	for( uint8_t p = 0; p < numPlayers; p++ ) {
		player[p].setColorBasedOnNum( p );
		player[p].loadTexture( driver );
	}

	goal.loadTexture( driver );

	if( debug && enableJoystick && device->activateJoysticks( joystickInfo ) ) { //activateJoysticks fills joystickInfo with info about each joystick
		wcout << L"Joystick support is enabled and " << joystickInfo.size() << L" joystick(s) are present." << std::endl;

		for( uint32_t joystick = 0; joystick < joystickInfo.size(); ++joystick ) {
			wcout << L"Joystick " << joystick << L":" << endl;
			wcout << L"\tName: '" << joystickInfo[joystick].Name.c_str() << L"'" << endl;
			wcout << L"\tAxes: " << joystickInfo[joystick].Axes << endl;
			wcout << L"\tButtons: " << joystickInfo[joystick].Buttons << endl;

			wcout << L"\tHat is: ";

			switch( joystickInfo[joystick].PovHat ) {
				case SJoystickInfo::POV_HAT_PRESENT:
					wcout << L"present" << endl;
					break;

				case SJoystickInfo::POV_HAT_ABSENT:
					wcout << L"absent" << endl;
					break;

				case SJoystickInfo::POV_HAT_UNKNOWN:
				default:
					wcout << L"unknown" << endl;
					break;
			}
		}
	} else if( debug ) {
		wcout << L"Joystick support is not enabled." << endl;
	}

	//Set up networking
	network.setup( isServer );


	timer = device->getTimer(); //timer must be set before loadProTips() is called only because loadProTips() calls resetThings(), which uses the timer.
	loadProTips();
}

bool GameManager::getDebugStatus() {
	return debug;
}

Goal GameManager::getGoal() {
	return goal;
}

MazeManager GameManager::getMazeManager() {
	return mazeManager;
}

Player* GameManager::getPlayer( uint8_t p ) {
	if( p < numPlayers ) {
		return &player.at( p );
	} else {
		wcerr << "getPlayer() error: Request for player (" << p << ">= numPlayers (" << numPlayers << ")" << endl;
		return NULL;
	}
}

void GameManager::loadFonts() {
	//These were split off into separate functions because they are needed more often than loadFonts()
	loadTipFont();
	loadMusicFont();

	core::dimension2d< uint32_t > fontDimensions;
	uint32_t size = windowSize.Width / 30; //30 found through experimentation: much larger and it takes too long to load fonts, much smaller and the font doesn't get as big as it should. Feel free to change at will if your computer's faster than mine.

	do { //Repeatedly loading fonts like this seems like a waste of time. Is there a way we could load the font only once and still get this kind of size adjustment?
		loadingFont = fm.GetTtFont( driver, fontFile.c_str(), size, antiAliasFonts );
		if( loadingFont != NULL ) {
			fontDimensions = loadingFont->getDimension( loading.c_str() );
			size -= 1;
		}
	} while( loadingFont != NULL && ( fontDimensions.Width > ( windowSize.Width / sideDisplaySizeDenominator ) || fontDimensions.Height > ( windowSize.Height / 5 ) ) );

	if( loadingFont == NULL ) {
		loadingFont = gui->getBuiltInFont();
	}

	size = ( windowSize.Width / sideDisplaySizeDenominator );

	do {
		textFont = fm.GetTtFont( driver, fontFile.c_str(), size, antiAliasFonts );
		if( textFont != NULL ) {
			fontDimensions = textFont->getDimension( L"Random seed: " );
			size -= 1;
		}
	} while( textFont != NULL && ( fontDimensions.Width + viewportSize.Width > windowSize.Width ) );

	if( textFont == NULL ) {
		textFont = gui->getBuiltInFont();
	}

	size = ( windowSize.Width / sideDisplaySizeDenominator );

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
}

//Like loadTipFont() below, this guesses a good font size, then repeatedly adjusts the size and reloads the font until everything fits.
void GameManager::loadMusicFont() {
	uint32_t maxWidth = ( windowSize.Width / sideDisplaySizeDenominator );
	uint32_t size = 0;
	uint32_t numerator = 2.5 * maxWidth; //2.5 is an arbitrarily chosen number, it has no special meaning. Change it to anything you want.

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

	core::dimension2d< uint32_t > artistDimensions;
	core::dimension2d< uint32_t > albumDimensions;
	core::dimension2d< uint32_t > titleDimensions;

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

void GameManager::loadNextSong() {
	if( debug ) {
		wcout << L"loadNextSong() called" << endl;
	}

	//Figure out where we are in the music list
	uint32_t positionInList = 0;
	for( uint32_t i = 0; i < musicList.size(); i++ ) {
		if( musicList[i] == currentMusic ) {
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
		wcerr << L"Unable to load music file: " << Mix_GetError() << endl;
	} else {
		int musicStatus = Mix_PlayMusic( music, 0 ); //The second argument tells how many times to play the music. -1 means infinite.

		if( musicStatus == -1 ) {
			wcerr << L"Unable to play music file: " << Mix_GetError() << endl;
		} else {
			if( debug ) {
				switch( Mix_GetMusicType( NULL ) ) {
					case MUS_CMD:
						wcout << L"Command based";
						break;
					case MUS_WAV:
						wcout << L"WAVE";
						break;
					case MUS_MOD:
						wcout << L"MOD";
						break;
					case MUS_MID:
						wcout << L"MIDI";
						break;
					case MUS_OGG:
						wcout << L"OGG";
						break;
					case MUS_MP3:
					case MUS_MP3_MAD:
						wcout << L"MP3";
						break;
					case MUS_NONE:
						wcout << L"No";
						break;
					case MUS_FLAC:
						wcout << L"FLAC";
						break;
					default:
						wcout << L"An unknown type of";
						break;
				}

				wcout << " music is playing." << endl;
			}

			//Now playing
			TagLib::FileRef f( currentMusic.c_str() ); //TagLib doesn't accept wstrings as file names, but it apparently can read tags as wstrings

			if( !f.isNull() && f.tag() ) {
				musicTitle = stringConverter.convert( f.tag()->title().toWString() ); //toWString() alone doesn't work here even though these are wide character strings because Irrlicht doesn't like accepting TagLib's wstrings.
				musicArtist = stringConverter.convert( f.tag()->artist().toWString() );
				musicAlbum = stringConverter.convert( f.tag()->album().toWString() );

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
					wcout << L"Now playing: " << musicTitle.c_str() << L" by " << musicArtist.c_str() << L" from album " << musicAlbum.c_str() << endl;
				}
			}

			loadMusicFont();
		}

		Mix_VolumeMusic( musicVolume * MIX_MAX_VOLUME / 100 );
	}
}

//Loads "pro tips" from file
void GameManager::loadProTips() {
	proTips.clear(); //This line is unnecessary because loadProTips() is only called once, but I just feel safer clearing this anyway.
	boost::filesystem::path proTipsPath( L"./protips.txt" );

	if( exists( proTipsPath ) ) {
		if( !is_directory( proTipsPath ) ) {
			if( debug ) {
				wcout << L"Loading pro tips from file " << proTipsPath.wstring() << endl;
			}
			boost::filesystem::wifstream proTipsFile;
			proTipsFile.open( proTipsPath, ios::in );

			if( proTipsFile.is_open() ) {
				wstring line;
				uint16_t lineNum = 0;

				while( proTipsFile.good() ) {
					lineNum++;
					getline( proTipsFile, line );

					if( !line.empty() ) {
						proTips.push_back( stringConverter.convert( line ) ); //StringConverter converts between wstring (which is what getLine needs) and core::stringw (which is what Irrlicht needs)

						if( debug ) {
							wcout << line << endl;
						}
					}
				}

				proTipsFile.close();

				resetThings(); //Only calling this here because it sets randomSeed;
				srand( randomSeed );
				random_shuffle( proTips.begin(), proTips.end() );
			} else {
				wcerr << L"Error: Unable to open pro tips file even though it exists. Check its access permissions." << endl;
			}
		} else {
			wcerr << L"Error: Pro tips file is a directory. Cannot load pro tips." << endl;
		}
	} else {
		wcerr << L"Error: Pro tips file does not exist. Cannot load pro tips." << endl;
	}
}

//Guesses a size that will work for showing pro tips. Keeps adjusting the size and reloading the font until everything fits.
void GameManager::loadTipFont() {
	uint32_t maxWidth = windowSize.Width;
	unsigned int size;
	core::stringw tipIncludingPrefix = proTipPrefix;

	if( proTips.size() > 0 ) { //If pro tips have been loaded, guess size based on tip length.
		tipIncludingPrefix.append( proTips.at( currentProTip ) );
		size = 2.5 * maxWidth / tipIncludingPrefix.size(); //2.5 is an arbitrarily chosen number, it has no special meaning. Change it to anything you want.
	} else {
		size = maxWidth / 10; //10 is also arbitrarily chosen.
	}

	core::dimension2d<uint32_t> tipDimensions;

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
}

//Finds all playable music files in the ./music folder and compiles them into a list
void GameManager::makeMusicList() {
	musicList.clear(); //The music list should be empty anyway, since makeMusicList() only gets called once, but just in case...

	uint8_t numMusicDecoders = Mix_GetNumMusicDecoders();

	if( debug ) {
		wcout << L"makeMusicList() called" << endl;
		wcout << L"There are " << numMusicDecoders << L" music decoders available. They are:" << endl;

		for( uint8_t decoder = 0; decoder < numMusicDecoders; decoder++ ) {
			wcout << decoder << L": " << Mix_GetMusicDecoder( decoder ) << endl;
		}
	}

	boost::filesystem::path musicPath( L"./music" );

	//Which is better: system_complete() or absolute()? On my computer they seem to do the same thing. Both are part of Boost Filesystem.
	musicPath = system_complete( musicPath );
	//musicPath = absolute( musicPath );

	if( debug ) {
		wcout << L"music path is absolute? " << musicPath.is_absolute() << endl;
	}

	while( ( !exists( musicPath ) || !is_directory( musicPath ) ) && musicPath.has_parent_path() ) {
		if( debug ) {
			wcout << L"Path " << musicPath.wstring() << L" does not exist or is not a directory. Checking parent path " << musicPath.parent_path().wstring() << endl;
		}

		musicPath = musicPath.parent_path();
	}

	if( exists( musicPath ) ) {
		boost::filesystem::recursive_directory_iterator end;

		for( boost::filesystem::recursive_directory_iterator i( musicPath ); i != end; i++ ) {
			if( !is_directory( i->path() ) ) {
				//Attempts to load a file as music. If successful, unload the file and add it to musicList.
				//This way the game is certain to accept any file formats the music library can use.
				Mix_Music* temp;
				temp = Mix_LoadMUS( i->path().c_str() );

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
		wcout << L"Could not find any music to play. Turning off playback." << endl;
		playMusic = false;
	}
}

void GameManager::movePlayerOnX( uint8_t p, int8_t direction ) {
	if( numPlayers > p ) {
		if( direction < 0 ) {
			if( player[ p ].getX() > 0 && mazeManager.maze[ player[ p ].getX()][ player[ p ].getY()].getLeft() == 'n' ) {
				player[ p ].moveX( -1 );
			}
		} else {
			if( player[ p ].getX() < ( mazeManager.cols - 1 ) && mazeManager.maze[ player[ p ].getX() + 1 ][ player[ p ].getY()].getLeft() == 'n' ) {
				player[ p ].moveX( 1 );
			}
		}

		network.sendPlayerPos( p, player[p].getX(), player[p].getY() );
		mazeManager.maze[ player[ p ].getX()][ player[ p ].getY()].visited = true;

		if( player[ p ].stepsTaken % 2 == 0 ) {
			mazeManager.maze[ player[ p ].getX()][ player[ p ].getY()].setVisitorColor( player[ p ].getColorTwo() );
		} else {
			mazeManager.maze[ player[ p ].getX()][ player[ p ].getY()].setVisitorColor( player[ p ].getColorOne() );
		}
	}
}

void GameManager::movePlayerOnY( uint8_t p, int8_t direction ) {
	if( numPlayers > p ) {
		if( direction < 0 ) {
			if( player[p].getY() > 0 && mazeManager.maze[player[p].getX()][player[p].getY()].getTop() == 'n' ) {
				player[p].moveY( -1 );
			}
		} else {
			if( player[p].getY() < ( mazeManager.rows - 1 ) && mazeManager.maze[player[p].getX()][player[p].getY() + 1].getTop() == 'n' ) {
				player[p].moveY( 1 );
			}
		}

		network.sendPlayerPos( p, player[p].getX(), player[p].getY() );
		mazeManager.maze[ player[ p ].getX()][ player[ p ].getY()].visited = true;

		if( player[ p ].stepsTaken % 2 == 0 ) {
			mazeManager.maze[ player[ p ].getX()][ player[ p ].getY()].setVisitorColor( player[ p ].getColorTwo() );
		} else {
			mazeManager.maze[ player[ p ].getX()][ player[ p ].getY()].setVisitorColor( player[ p ].getColorOne() );
		}
	}
}

bool GameManager::OnEvent( const SEvent& event ) {
	switch( event.EventType ) {
		case EET_KEY_INPUT_EVENT: {
			if( event.KeyInput.PressedDown && !( showingMenu || showingLoadingScreen ) ) {
				switch( event.KeyInput.Key ) { //TODO: Allow customizable controls
					case KEY_SNAPSHOT: //I think KEY_SNAPSHOT corresponds to print screen
					case KEY_KEY_P: { //My computer has no print screen key, so I use the letter P instead
						takeScreenShot();
						break;
					}

					//left movement keys
					case KEY_LEFT: {
						if( numBots < ( numPlayers - 0 ) && ( isServer || myPlayer == 0 ) ) {
							movePlayerOnX( 0, -1 );
							return true;
						}

						break;
					}
					case KEY_KEY_A: {
						if( numBots < ( numPlayers - 1 ) && ( isServer || myPlayer == 1 ) ) {
							movePlayerOnX( 1, -1 );
							return true;
						}

						break;
					}
					case KEY_KEY_F: {
						if( numBots < ( numPlayers - 2 ) && ( isServer || myPlayer == 2 ) ) {
							movePlayerOnX( 2, -1 );
							return true;
						}

						break;
					}
					case KEY_KEY_J: {
						if( numBots < ( numPlayers - 3 ) && ( isServer || myPlayer == 3 ) ) {
							movePlayerOnX( 3, -1 );
							return true;
						}

						break;
					}

					//right movement keys
					case KEY_RIGHT: {
						if( numBots < ( numPlayers - 0 ) && ( isServer || myPlayer == 0 ) ) {
							movePlayerOnX( 0, 1 );
							return true;
						}

						break;
					}
					case KEY_KEY_D: {
						if( numBots < ( numPlayers - 1 ) && ( isServer || myPlayer == 1 ) ) {
							movePlayerOnX( 1, 1 );
							return true;
						}

						break;
					}
					case KEY_KEY_H: {
						if( numBots < ( numPlayers - 2 ) && ( isServer || myPlayer == 2 ) ) {
							movePlayerOnX( 2, 1 );
							return true;
						}

						break;
					}
					case KEY_KEY_L: {
						if( numBots < ( numPlayers - 3 ) && ( isServer || myPlayer == 3 ) ) {
							movePlayerOnX( 3, 1 );
							return true;
						}

						break;
					}

					//up movement keys
					case KEY_UP: {
						if( numBots < ( numPlayers - 0 ) && ( isServer || myPlayer == 0 ) ) {
							movePlayerOnY( 0, -1 );
							return true;
						}

						break;
					}
					case KEY_KEY_W: {
						if( numBots < ( numPlayers - 1 ) && ( isServer || myPlayer == 1 ) ) {
							movePlayerOnY( 1, -1 );
							return true;
						}

						break;
					}
					case KEY_KEY_T: {
						if( numBots < ( numPlayers - 2 ) && ( isServer || myPlayer == 2 ) ) {
							movePlayerOnY( 2, -1 );
							return true;
						}

						break;
					}
					case KEY_KEY_I: {
						if( numBots < ( numPlayers - 3 ) && ( isServer || myPlayer == 3 ) ) {
							movePlayerOnY( 3, -1 );
							return true;
						}

						break;
					}

					//down movement keys
					case KEY_DOWN: {
						if( numBots < ( numPlayers - 0 ) && ( isServer || myPlayer == 0 ) ) {
							movePlayerOnY( 0, 1 );
							return true;
						}

						break;
					}
					case KEY_KEY_S: {
						if( numBots < ( numPlayers - 1 ) && ( isServer || myPlayer == 1 ) ) {
							movePlayerOnY( 1, 1 );
							return true;
						}

						break;
					}
					case KEY_KEY_G: {
						if( numBots < ( numPlayers - 2 ) && ( isServer || myPlayer == 2 ) ) {
							movePlayerOnY( 2, 1 );
							return true;
						}

						break;
					}
					case KEY_KEY_K: {
						if( numBots < ( numPlayers - 3 ) && ( isServer || myPlayer == 3 ) ) {
							movePlayerOnY( 3, 1 );
							return true;
							\
						}

						break;
					}

					case KEY_MENU:
					case KEY_ESCAPE: {
						showingMenu = true;
						return true;
					}
					break;
					default:
						break;
				}
			} else if( event.KeyInput.PressedDown && showingMenu ) {
				switch( event.KeyInput.Key ) {
					case KEY_MENU:
					case KEY_ESCAPE: { //Get out of the menu if we're showing it and escape is pressed
						showingMenu = false;
						return true;
					}
					break;
					default:
						break;
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
							//wcout << L"Directory: " << fileChooser->getDirectoryName().c_str() << endl;
							//loadFromFile();
							return true;
						} else if( saveMaze.contains( event.MouseInput.X, event.MouseInput.Y ) ) {
							mazeManager.saveToFile();
							return true;
						} else if( newGame.contains( event.MouseInput.X, event.MouseInput.Y ) ) {
							//randomSeed = time( NULL );
							resetThings();
							mazeManager.makeRandomLevel();

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
						wcout << L"Mouse wheel moved ";
					}

					if( event.MouseInput.Wheel > 0 ) {
						if( debug ) {
							wcout << "up" << endl;
						}

						musicVolume += 5;

						if( musicVolume > 100 ) {
							musicVolume = 100;
						}
					} else {
						if( debug ) {
							wcout << "down" << endl;
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
					loadFonts();
					return true;
				}
				break;
				case USER_EVENT_JOYSTICK_UP: {
					if( debug ) {
						wcout << L"Joystick moved up" << endl;
					}
					movePlayerOnY( 0, -1 );
					return true;
				}
				break;
				case USER_EVENT_JOYSTICK_DOWN: {
					if( debug ) {
						wcout << L"Joystick moved down" << endl;
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
				core::array<int16_t> verticalAxes;
				verticalAxes.push_back( SEvent::SJoystickEvent::AXIS_Y );

				bool joystickMovedUp = false;
				bool joystickMovedDown = false;
				bool joystickMovedRight = false;
				bool joystickMovedLeft = false;

				for( uint32_t i = 0; i < verticalAxes.size(); i++ ) {
					if( event.JoystickEvent.Axis[i] >= ( SHRT_MAX / 2 ) ) { //See Irrlicht's <irrTypes.h>: Axes are represented by int16_t's, typedef'd in the current version (as of 2013-06-22) as signed short. SHRT_MAX comes from <climits>
						if( debug ) {
							wcout << L"Axis value: " << event.JoystickEvent.Axis[i] << endl;
						}
						joystickMovedUp = true;
					} else if( event.JoystickEvent.Axis[i] <= ( SHRT_MIN / 2 ) ) {
						if( debug ) {
							wcout << L"Axis value: " << event.JoystickEvent.Axis[i] << endl;
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
						wcout << L"File selected." << endl;
						wcout << L"Folder: " << core::stringw( fileChooser->getDirectoryName() ).c_str() << L"\tFile: " << core::stringw( fileChooser->getFileName() ).c_str() << endl;
					}

					mazeManager.loadFromFile( fileChooser->getFileName() );
					return true;
					break;
				}
				case gui::EGET_DIRECTORY_SELECTED: {
					if( debug ) {
						wcout << L"Folder selected." << endl;
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

	return false;
}

void GameManager::readPrefs() {
	boost::filesystem::path prefsPath( L"./prefs.cfg" );

	//Set default prefs, in case we can't get them from the file
	showBackgrounds = true;
	fullscreen = false;
	bitsPerPixel = 16;
	vsync = true;
	driverType = video::EDT_OPENGL;
	windowSize = core::dimension2d< uint32_t >( 640, 480 );
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

	if( !is_directory( prefsPath ) ) {
		if( debug ) {
			wcout << L"Loading preferences from file " << prefsPath.wstring() << endl;
		}
		boost::filesystem::wifstream prefsFile;
		prefsFile.open( prefsPath, ios::in );

		if( prefsFile.is_open() ) {
			wstring line;
			uint16_t lineNum = 0;

			while( prefsFile.good() ) {
				lineNum++;
				getline( prefsFile, line );
				line = line.substr( 0, line.find( L"//" ) ); //Filters out comments
				boost::algorithm::trim( line ); //Removes trailing and leading spaces
				boost::algorithm::to_lower( line );
				if( debug ) {
				    wcout << L"Line " << lineNum << L": \"" << line << "\"" << endl;
				}


				if( !line.empty() ) {
					try {
						wstring preference = boost::algorithm::trim_copy( line.substr( 0, line.find( '\t' ) ) );
						wstring choice = boost::algorithm::trim_copy( line.substr( line.find( '\t' ) ) );
						if( debug ) {
							wcout << L"Preference \"" << preference << L"\" choice \"" << choice << L"\""<< endl;
						}

						if( preference == L"volume:" ) {
							try {
								uint16_t choiceAsInt = boost::lexical_cast< uint16_t >( choice ); //Used uint16_t here because boost::lexical_cast refuses to convert from a wstring to a uint8_t. uint8_t is technically an unsigned char, but we're using that type as an 8-bit unsigned integer

								if( choiceAsInt <= 100 && choiceAsInt >= 0 ) {
									musicVolume = choiceAsInt;
									Mix_VolumeMusic( musicVolume * MIX_MAX_VOLUME / 100 );
									if( debug ) {
										wcout << L"Volume should be " << choiceAsInt << "%" << endl;
										wcout << L"Volume is really " << 100 * Mix_VolumeMusic( -1 ) / MIX_MAX_VOLUME << "%" << endl;
									}
								} else if( choiceAsInt < 0 ) {
									wcerr << L"Warning: Volume less than zero: " << choiceAsInt << endl;
									Mix_VolumeMusic( 0 );
									musicVolume = 0;
								} else {
									wcerr << L"Warning: Volume greater than 100%: " << choiceAsInt << endl;
									Mix_VolumeMusic( MIX_MAX_VOLUME );
									musicVolume = 100;
								}
							} catch( boost::bad_lexical_cast error ) {
								wcerr << L"Error reading volume preference (is it not a number?) on line " << lineNum << L": " << error.what() << endl;
							}
						} else if( preference == L"number of bots:" ) {
							try {
								uint16_t choiceAsInt = boost::lexical_cast< uint16_t >( choice ); //Used uint16_t here because boost::lexical_cast refuses to convert from a wstring to a uint8_t. uint8_t is technically an unsigned char, but we're using that type as an 8-bit unsigned integer

								if( choiceAsInt <= numPlayers ) {
									numBots = choiceAsInt;
									if( debug ) {
										wcout << L"Number of bots is " << choiceAsInt << endl;
									}
								} else {
									wcerr << L"Warning: Number of bots not less than or equal to number of players (number of players may not have been read yet): " << choiceAsInt << endl;
									numBots = choiceAsInt;
								}
							} catch( boost::bad_lexical_cast error ) {
								wcerr << L"Error reading number of bots preference (is it not a number?) on line " << lineNum << L": " << error.what() << endl;
							}
						} else if( preference == L"show background animations:" ) {
							if( choice == L"true" ) {
								if( debug ) {
									wcout << L"Show backgrounds is ON" << endl;
								}
								showBackgrounds = true;
							} else if( choice == L"false" ) {
								if( debug ) {
									wcout << L"Show backgrounds is OFF" << endl;
								}
								showBackgrounds = false;
							} else {
								wcerr << L"Error reading show background animations preference on line " << lineNum  << L": \"" << choice  << endl;//<< L"\"" << endl;
							}
						} else if( preference == L"fullscreen:" ) {

							if( choice == L"true" ) {
								if( debug ) {
									wcout << L"Fullscreen is ON" << endl;
								}
								fullscreen = true;
							} else if( choice == L"false" ) {
								if( debug ) {
									wcout << L"Fullscreen is OFF" << endl;
								}
								fullscreen = false;
							} else {
								wcerr << L"Error reading fullscreen preference on line " << lineNum  << L": \"" << choice  << endl;//<< L"\"" << endl;
							}
						} else if( preference == L"mark player trails:" ) {

							if( choice == L"true" ) {
								if( debug ) {
									wcout << L"Mark trails is ON" << endl;
								}
								markTrails = true;
							} else if( choice == L"false" ) {
								if( debug ) {
									wcout << L"Mark trails is OFF" << endl;
								}
								markTrails = false;
							} else {
								wcerr << L"Error reading mark player trails preference on line " << lineNum  << L": \"" << choice  << endl;//<< L"\"" << endl;
							}
						} else if( preference == L"debug:" ) {

	#ifndef DEBUG

							if( choice == L"true" ) {
								debug = true;
							} else if( choice == L"false" ) {
								debug = false;
							} else {
								wcerr << L"Error reading debug preference on line " << lineNum  << L": \"" << choice  << endl;//<< L"\"" << endl;
							}

	#endif

							if( debug ) {
								wcout << L"Debug is ON" << endl;
							}
						} else if( preference == L"bits per pixel:" ) {
							try {
								uint32_t choiceAsInt = boost::lexical_cast<uint32_t>( choice );

								if( choiceAsInt <= 32 ) {
									bitsPerPixel = choiceAsInt;
									if( debug ) {
										wcout << L"Bits per pixel is " << choiceAsInt << endl;
									}
								} else {
									wcerr << L"Warning: Bits per pixel not less than or equal to 32: " << choiceAsInt << endl;
									bitsPerPixel = choiceAsInt;
								}
							} catch( boost::bad_lexical_cast error ) {
								wcerr << L"Error reading bitsPerPixel preference (is it not a number?) on line " << lineNum << L": " << error.what() << endl;
							}

						} else if( preference == L"wait for vertical sync:" ) {

							if( choice == L"true" ) {
								if( debug ) {
									wcout << L"Vertical sync is ON" << endl;
								}
								vsync = true;
							} else if( choice == L"false" ) {
								if( debug ) {
									wcout << L"Vertical sync is OFF" << endl;
								}
								vsync = false;
							} else {
								wcerr << L"Error reading vertical sync preference on line " << lineNum << L": \"" << choice << L"\"" << endl;
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
								wcerr << L"Warning: Selected driver type " << choice << L" not recognized. Trying OpenGL." << endl;
								choice = L"opengl";
							}

							if( !device->isDriverSupported( driverType ) ) {
								wcerr << L"Warning: Chosen driver type " << choice << L" is not supported on this system. ";

								if( device->isDriverSupported( video::EDT_OPENGL ) ) {
									wcerr << L"Trying OpenGL." << endl;
									driverType = video::EDT_OPENGL;
									choice = L"opengl";
								} else if( device->isDriverSupported( video::EDT_DIRECT3D9 ) ) {
									wcerr << L"Trying Direct3D 9." << endl;
									driverType = video::EDT_DIRECT3D9;
									choice = L"direct3d9";
								} else if( device->isDriverSupported( video::EDT_DIRECT3D8 ) ) {
									wcerr << L"Trying Direct3D 8." << endl;
									driverType = video::EDT_DIRECT3D8;
									choice = L"direct3d8";
								} else if( device->isDriverSupported( video::EDT_BURNINGSVIDEO ) ) {
									wcerr << L"Trying Burning's video." << endl;
									driverType = video::EDT_BURNINGSVIDEO;
									choice = L"burning's video";
								} else if( device->isDriverSupported( video::EDT_SOFTWARE ) ) {
									wcerr << L"Trying software renderer." << endl;
									driverType = video::EDT_SOFTWARE;
									choice = L"software";
								} else {
									wcerr << L"Error: No graphical output driver types are available. Using NULL type!! Also enabling debug." << endl;
									driverType = video::EDT_NULL;
									choice = L"NULL";
									debug = true;
								}
							}

							if( debug ) {
								wcout << L"Driver type is " << choice << endl;
							}

						} else if( preference == L"number of players:" ) {
							try {
								uint16_t choiceAsInt = boost::lexical_cast<uint16_t>( choice ); //Used uint16_t here because boost::lexical_cast refuses to convert from a wstring to a uint8_t. uint8_t is technically an unsigned char, but we're using that type as an 8-bit unsigned integer

								if( choiceAsInt > UCHAR_MAX ) { //The maximum number of players is whatever a uint8_t can hold, presumably 255 (the 8 is the minimum number of bits, it may not be the true number).
									choiceAsInt = UCHAR_MAX;
								}

								if( choiceAsInt <= 4 && choiceAsInt > 0 ) {
									numPlayers = choiceAsInt;
									if( debug ) {
										wcout << L"Number of players is " << choiceAsInt << endl;
									}
								} else if( choiceAsInt > 4 ) {
									wcerr << L"Warning: Number of players not less than or equal to 4: " << choiceAsInt << endl;
									numPlayers = choiceAsInt;
								} else {
									wcerr << L"Warning: Number of players is zero or not a number: " << choiceAsInt << L". Setting number of players to default." << endl;
								}
							} catch( boost::bad_lexical_cast error ) {
								wcerr << L"Error reading number of players preference (is it not a number?) on line " << lineNum << L": " << error.what() << endl;
							}

						} else if( preference == L"window size:" ) {
							size_t locationOfX = choice.find( L"x" );
							wstring width = choice.substr( 0, locationOfX );
							wstring height = choice.substr( locationOfX + 1 );
							if( debug ) {
								wcout << L"Window size: " << width << L"x" << height << endl;
							}

							uint32_t widthAsInt = boost::lexical_cast<uint32_t>( width );
							uint32_t heightAsInt = boost::lexical_cast<uint32_t>( height );

							if( widthAsInt < 320 || heightAsInt < 240 ) {
								wcerr << L"Error reading window size: Width and/or height are really really tiny. Sorry but you'll have to recompile the game yourself if you want a window that small." << endl;
							} else if( widthAsInt == 320 && heightAsInt == 240 ) {
								wcout << L"Rock on, CGA graphics. Rock on." << endl;
								windowSize = core::dimension2d<uint32_t>( widthAsInt, heightAsInt );
							} else {
								windowSize = core::dimension2d<uint32_t>( widthAsInt, heightAsInt );
							}

						} else if( preference == L"play music:" ) {
							if( choice == L"true" ) {
								if( debug ) {
									wcout << L"Play music is ON" << endl;
								}
								playMusic = true;
							} else if( choice == L"false" ) {
								if( debug ) {
									wcout << L"Play music is OFF" << endl;
								}
								playMusic = false;
							} else {
								wcerr << L"Error reading play music preference on line " << lineNum << L": \"" << choice << L"\"" << endl;
							}

						} else if( preference == L"network port:" ) {
							if( debug ) {
								wcout << L"Network port: " << choice << endl;
							}

							try {
								uint16_t choiceAsInt = boost::lexical_cast<uint16_t>( choice );
								network.setPort( choiceAsInt );
							} catch( boost::bad_lexical_cast error ) {
								wcerr << L"Error reading network port (is it not a number?) on line " << lineNum << L": " << error.what() << endl;
							}
						} else if( preference == L"enable joystick:" ) {
							if( choice == L"true" ) {
								if( debug ) {
									wcout << L"Joystick is ENABLED" << endl;
								}
								enableJoystick = true;
							} else if( choice == L"false" ) {
								if( debug ) {
									wcout << L"Joystick is DISABLED" << endl;
								}
								enableJoystick = false;
							} else {
								wcerr << L"Error reading enable joystick preference on line " << lineNum << L": \"" << choice << L"\"" << endl;
							}
						} else if( preference == L"joystick number:" ) {
							if( debug ) {
								wcout << L"Joystick number: " << choice << endl;
							}
							try {
								uint32_t choiceAsInt = boost::lexical_cast<uint32_t>( choice );
								joystickChosen = choiceAsInt;
							} catch (boost::bad_lexical_cast error ) {
								wcerr << L"Error reading joystick number (is it not a number?) on line " << lineNum << L": " << error.what() << endl;
							}
						} else if( preference == L"always server:" ) {
							if( choice == L"true" ) {
								if( debug ) {
									wcout << L"This is always a server" << endl;
								}
								isServer = true;
							} else if( choice == L"false" ) {
								if( debug ) {
									wcout << L"This is not always a server" << endl;
								}
								isServer = false;
							} else {
								wcerr << L"Error reading \"always server\" preference on line " << lineNum << L": \"" << choice << L"\"" << endl;
							}
						} else {
							wcerr << L"Unrecognized preference on line " << lineNum << L": \"" << line << L"\"" << endl;
						}
					} catch ( std::exception e ) {
						wcout << L"Error: " << e.what() << L". Does line " << lineNum << L" not have a tab character separating preference and value? The line says " << line << endl;
					}
				} else { //This line in the preferences file is empty
				}
			}

			prefsFile.close();
		}
	} else {
		wcerr << L"Error: Prefs file is a directory. Cannot load prefs." << endl;
	}

	if( !isServer ) {
		wcout << L"(S)erver or (c)lient? " << endl;
		wchar_t a;
		wcin >> a;
		isServer = ( a == L's' || a == L'S' );
		myPlayer = 0;
	}
}

//Resets miscellaneous stuff between mazes
void GameManager::resetThings() {
	randomSeed = time( NULL );
	loadingDelay = 1000 + ( rand() % 5000 );

	winners.clear();
	stuff.clear();
	showingMenu = false;
	numKeysFound = 0;
	numLocks = 0;
	donePlaying = false;

	for( uint8_t p = 0; p < numPlayers; p++ ) {
		playerStart[ p ].reset();
		player[ p ].stepsTaken = 0;
	}

	for( uint32_t i = 0; i < stuff.size(); i++ ) {
		stuff[ i ].loadTexture( driver );
	}

	for( uint8_t x = 0; x < mazeManager.cols; x++ ) {
		for( uint8_t y = 0; y < mazeManager.rows; y++ ) {
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
}

//Should only be called by main() in main.cpp
int GameManager::run() {

	while( device->run() && !donePlaying ) {
		resetThings();
		mazeManager.makeRandomLevel();

		showingMenu = false; //Temporary workaround until I figure out where in makeRandomLevel() this is being overwritten

		while( device->run() && !won && !donePlaying ) {

			if( showingLoadingScreen && ( timer->getRealTime() > timeStartedLoading + loadingDelay ) ) {
				showingLoadingScreen = false;
			}

			uint32_t fps = driver->getFPS();
			if( debug ) {
				//wcout << L"FPS: " << fps << endl;
			}
			/*These next 3 lines are for limiting processor usage. I really
			 *doubt they're all needed, so comment one or another if you
			 *experience slowness. Two are provided by Irrlicht and the other by SDL.*/
			//device->yield();
			device->sleep( 17 ); //17 = 1/60 of a second, rounded up, in milliseconds. My monitor refreshes at 60 Hz.
			//SDL_Delay( 17 );

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
				cellWidth = ( viewportSize.Width ) / mazeManager.cols;
				cellHeight = ( viewportSize.Height ) / mazeManager.rows;

				//It's the bots' turn to move now.
				if( !( showingMenu || showingLoadingScreen ) && numBots > 0 ) {
					for( uint8_t i = 0; i < numBots; i++ ) {
						if( debug || allHumansAtGoal() || bot[ i ].lastTimeMoved < timer->getRealTime() - bot[ i ].movementDelay ) {
							bot[ i ].move();
						}
					}
				}

				device->getCursorControl()->setVisible( showingMenu || showingLoadingScreen || debug );
				drawAll();

				//Check if any of the players have landed on a collectable item
				for( uint8_t p = 0; p < numPlayers; p++ ) {
					for( uint32_t s = 0; s < stuff.size(); s++ ) {
						if( player[p].getX() == stuff[s].getX() && player[p].getY() == stuff[s].getY() ) {
							switch( stuff[s].getType() ) {
								case COLLECTABLE_KEY: {
									numKeysFound++;
									stuff.erase( stuff.begin() + s );

									if( numKeysFound >= numLocks ) {
										for( uint8_t c = 0; c < mazeManager.cols; c++ ) {
											for( uint8_t r = 0; r < mazeManager.rows; r++ ) {
												mazeManager.maze[c][r].removeLocks();
											}
										}

										for( uint8_t b = 0; b < numBots; b++ ) {
											bot[ b ].allKeysFound();
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


				for( uint8_t p = 0; p < numPlayers; p++ ) {
					if(( player[p].getX() == goal.getX() ) && player[p].getY() == goal.getY() ) { //Make a list of who finished in what order
						bool alreadyFinished = false; //Indicates whether the player is already on the winners list

						for( uint32_t i = 0; i < winners.size() && !alreadyFinished; i++ ) {
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
			if( network.checkForConnections() != 0 ) {
				std::wcerr << L"Networking error." << std::endl;
			} else {
				if( network.receiveData() ) {
					if( debug ) {
						std::wcout << L"Received data" << std::endl;
					}
				} else {
					/*if( debug ) {
						std::wcout << L"Did not receive data" << std::endl;
					}*/
				}
			}
		}

		timer->stop();

		if( !donePlaying ) {
			if( debug ) {
				wcout << L"On to the next level!" << endl;
				wcout << L"Winners:";

				for( uint32_t i = 0; i < winners.size(); i++ ) {
					wcout << L" " << winners.at( i );
				}

				wcout << endl;
			}
			startLoadingScreen();
		}
	}

	return 0;
}

void GameManager::setupBackground() {
	uint8_t availableBackgrounds = 1; //The number of different background animations to choose from

	backgroundChosen = rand() % availableBackgrounds;
	if( debug ) {
		wcout << L"Background chosen: " << backgroundChosen << endl;
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

			video::IImage* pixelImage = driver->createImage( video::ECF_A8R8G8B8, core::dimension2d<uint32_t>( 1, 1 ) );
			//pixelImage->fill( WHITE );
			pixelImage->setPixel( 0, 0, WHITE, false ); //Which is faster on a 1x1 pixel image: setPixel() or fill()?
			video::ITexture* pixelTexture = driver->addTexture( "pixel", pixelImage );
			ps->setMaterialTexture( 0, pixelTexture );
		}
		break;
		default:
			break;
	}
}

void GameManager::startLoadingScreen() {
	showingLoadingScreen = true;
	timeStartedLoading = timer->getRealTime();
}

void GameManager::takeScreenShot() {
	irr::video::IImage* image = driver->createScreenShot();

	if( image ) {
		irr::core::stringw filename;
		filename.append( L"screenshot_" );

		time_t currentTime = time( NULL );
		wchar_t clockTime[20];
		wcsftime( clockTime, 20, L"%Y-%m-%d %H:%M:%S", localtime( &currentTime ) );
		filename.append( clockTime );
		filename.append( L".png" );

		if( !driver->writeImageToFile( image, filename ) ) {
			std::wcerr << L"takeScreenShot(): Failed to save screen shot to file " << filename.c_str() << std::endl;
		} else if( debug ) {
			std::wcout << L"Screen shot saved as " << filename.c_str() << std::endl;
		}

		image->drop();
	} else {
		std::wcerr << L"takeScreenShot(): Failed to take screen shot" << std::endl;
	}
}



