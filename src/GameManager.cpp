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
#include "font_manager.h"
#include <iostream>
#include <irrlicht.h>
#include <vector>
#include <filesystem.hpp>
#include <filesystem/fstream.hpp>
#include <algorithm/string.hpp>
#include <lexical_cast.hpp>
//#include <asio.hpp>
#include <bind.hpp>
#include <SDL/SDL.h>
#include <SDL_mixer.h>
#include <stdlib.h>
#include <time.h>
#include <queue>
#include <date_time/posix_time/posix_time.hpp>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <iostream>
#include <errno.h>
#include <sstream>
#include <taglib/fileref.h>
#include <taglib/tag.h>

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
//TODO: Allow volume adjustment (Mix_Volume && Mix_VolumeMusic)
//TODO: Consider adding stats (estimated difficulty of maze, number of steps taken, number of cells backtracked, etc)
//TODO: Add a loading screen between maps. Display pro tips on loading screen. Possibly display player stats too.

using namespace std;
using namespace irr;
using boost::asio::ip::tcp;

void GameManager::loadProTips() {
	proTips.clear();
	boost::filesystem::path proTipsPath( L"./protips.txt" );

	if( exists( proTipsPath ) ) {
		if( !is_directory( proTipsPath ) ) {
			wcout << L"Loading pro tips from file " << proTipsPath.wstring() << endl;
			boost::filesystem::wifstream proTipsFile;
			proTipsFile.open( proTipsPath, ios::in );

			if( proTipsFile.is_open() ) {
				wstring line;
				u16 lineNum = 0;

				while( proTipsFile.good() ) {
					lineNum++;
					getline( proTipsFile, line );

					if( !line.empty() ) {
						proTips.push_back( stringConverter.convert( line ) );
						if( debug ) {
							wcout << line << endl;
						}
					}
				}

				proTipsFile.close();
				srand( time( NULL) );
				random_shuffle( proTips.begin(), proTips.end() );
			}
		} else {
			wcerr << L"Error: Pro tips file is a directory. Cannot load pro tips." << endl;
		}
	} else {
		wcerr << L"Error: Pro tips file does not exist. Cannot load pro tips." << endl;
	}
}

void GameManager::loadTipFont() {
	u32 maxWidth = windowSize.Width;
	unsigned int size;
	core::stringw tipTotal = proTipStart;
	if( proTips.size() > 0 ) { //If pro tips have been loaded, guess size based on tip length.
		tipTotal.append( proTips.at( currentProTip ) );
		size = 2.5 * maxWidth / tipTotal.size();
	} else {
		size = maxWidth / 10;
	}

	core::dimension2d<u32> tipDimensions;
	do {
		tipFont = fm.GetTtFont(driver, fontFile.c_str(), size, antiAliasFonts);
		tipDimensions = tipFont->getDimension( tipTotal.c_str() );
		size -= 1;
	} while ( tipDimensions.Width > maxWidth );
}

void GameManager::loadMusicFont() {
	u32 maxWidth = (windowSize.Width / 6);
	u32 size = 0;// = maxWidth / 6;
	u32 numerator = 2.5 * maxWidth;

	if( musicArtist.size() >= musicAlbum.size() && musicArtist.size() > 0 ) {
		if( musicArtist.size() >= musicTitle.size() ){
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

	if( size == 0 ) {
		size = numerator;
	}

	core::dimension2d< u32 > artistDimensions;
	core::dimension2d< u32 > albumDimensions;
	core::dimension2d< u32 > titleDimensions;
	do {
		musicTagFont = fm.GetTtFont(driver, fontFile.c_str(), size, antiAliasFonts);
		artistDimensions = musicTagFont->getDimension( musicArtist.c_str() );
		albumDimensions = musicTagFont->getDimension( musicAlbum.c_str() );
		titleDimensions = musicTagFont->getDimension( musicTitle.c_str() );
		size -= 1;
	} while ( artistDimensions.Width > maxWidth || albumDimensions.Width > maxWidth || titleDimensions.Width > maxWidth );
}

void GameManager::loadFonts() {
	loadTipFont();
	loadMusicFont();

	core::dimension2d< u32 > fontDimensions;
	u32 size = windowSize.Width / 10;
	do {
		loadingFont = fm.GetTtFont(driver, fontFile.c_str(), size, antiAliasFonts);
		fontDimensions = loadingFont->getDimension( loading.c_str() );
		size -= 1;
	} while( fontDimensions.Width > ( windowSize.Width / 5 ) || fontDimensions.Height > ( windowSize.Height / 5 ) );

	size = ( windowSize.Width / 6 ) / 7;
	do {
		textFont = fm.GetTtFont(driver, fontFile.c_str(), size, antiAliasFonts);
		fontDimensions = textFont->getDimension( L"Random seed: " );
		size -= 1;
	} while( fontDimensions.Width + viewportSize.Width > windowSize.Width );

	size = ( windowSize.Width / 6 ) / 4;
	do {
		clockFont = fm.GetTtFont(driver, fontFile.c_str(), size, antiAliasFonts);
		fontDimensions = textFont->getDimension( L"00:00:00" );
		size -= 1;
	} while( fontDimensions.Width + viewportSize.Width > windowSize.Width  || fontDimensions.Height > ( windowSize.Height / 5 ) );
}

bool GameManager::getDebugStatus() {
	return debug;
}

void GameManager::takeScreenShot() {
	irr::video::IImage* image = driver->createScreenShot();

	if( image ) {
		irr::core::stringw filename;
		filename.append( L"screenshot_" );

		time_t currentTime = time( NULL );
		wchar_t clockTime[20];
		//strftime( clockTime, 20, "%Y-%m-%d %H:%M:%S", localtime( &currentTime ) );
		wcsftime( clockTime, 20, L"%Y-%m-%d %H:%M:%S", localtime( &currentTime ) );
		filename.append( clockTime );
		filename.append( L".png" );

		if( !driver->writeImageToFile( image, filename ) ) {
			std::wcerr << L"takeScreenShot(): Failed to save screen shot to file " << filename.c_str() << std::endl;
		} else {
			if( debug ) {
				std::wcout << L"Screen shot saved as " << filename.c_str() << std::endl;
			}
		}

		image->drop();
	} else {
		std::wcerr << L"takeScreenShot(): Failed to take screen shot" << std::endl;
	}
}

void GameManager::makeMusicList() {
	u8 numMusicDecoders = Mix_GetNumMusicDecoders();

	if( debug ) {
		wcout << L"makeMusicList() called" << endl;
		wcout << L"There are " << static_cast<unsigned int>( numMusicDecoders ) << L" music decoders available. They are:" << endl;

		for( u8 decoder = 0; decoder < numMusicDecoders; decoder++ ) {
			wcout << static_cast<unsigned int>( decoder ) << L": " << Mix_GetMusicDecoder( decoder ) << endl;
		}
	}
	musicList.clear();

	boost::filesystem::path musicPath( L"./music/whatever/fake/folder/really/long" );

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
				Mix_Music * temp;
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
		srand( time( NULL) );
		random_shuffle( musicList.begin(), musicList.end() );

		currentMusic = musicList.back();
	} else {
		wcout << L"Could not find any music to play. Turning off playback." << endl;
		playMusic = false;
	}
}

void GameManager::loadNextSong() {
	wcout << L"loadNextSong() called" << endl;

	u32 positionInList = 0;

	for( u32 i = 0; i < musicList.size(); i++ ) {
		if( musicList[i] == currentMusic ) {
			positionInList = i;
			break;
		}
	}

	positionInList += 1;

	if( positionInList >= musicList.size() ) {
		positionInList -= musicList.size();
	}

	currentMusic = musicList[positionInList];
	music = Mix_LoadMUS( currentMusic.c_str() ); //SDL Mixer does not support wstrings

	if( music == NULL ) {
		wcerr << L"Unable to load music file: " << Mix_GetError() << endl;
	} else {
		channel = Mix_PlayMusic( music, 0 ); //The second argument tells how many times to play the music. -1 means infinite.

		if( channel == -1 ) {
			wcerr << L"Unable to play music file: " << Mix_GetError() << endl;
		} else {
			//wcout << Mix_GetMusicDecoder(Mix_GetMusicType(NULL)) << endl;
			switch( Mix_GetMusicType( NULL ) ) {
				case MUS_CMD:
					wcout << L"Command based music is playing." << endl;
					break;
				case MUS_WAV:
					wcout << L"WAVE music is playing." << endl;
					break;
				case MUS_MOD:
					wcout << L"MOD music is playing." << endl;
					break;
				case MUS_MID:
					wcout << L"MIDI music is playing." << endl;
					break;
				case MUS_OGG:
					wcout << L"OGG music is playing." << endl;
					break;
				case MUS_MP3:
				case MUS_MP3_MAD:
					wcout << L"MP3 music is playing." << endl;
					break;
				case MUS_NONE:
					wcout << L"No music is playing." << endl;
					break;
				case MUS_FLAC:
					wcout << L"FLAC music is playing." << endl;
					break;
				default:
					wcout << L"Unknown music is playing." << endl;
					break;
			}

			//Now playing
			TagLib::FileRef f( currentMusic.c_str() ); //TagLib doesn't accept wstrings as file names, but it apparently can read tags as wstrings

			if( !f.isNull() && f.tag() ) {
				musicTitle = stringConverter.convert( f.tag()->title().toWString() ); //toWString() alone doesn't work here even though these are wide character strings. Irrlicht doesn't like accepting TagLib's wstrings.
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

				wcout << L"Now playing: " << musicTitle.c_str() << L" by " << musicArtist.c_str() << L" from album " << musicAlbum.c_str() << endl;
			}

			loadMusicFont();
		}
	}
}

void GameManager::readPrefs() {
	boost::filesystem::path prefsPath( L"./prefs.cfg" );

	//Set default prefs, in case we can't get them from the file
	fullscreen = false;
	bitsPerPixel = 16;
	vsync = true;
	driverType = video::EDT_OPENGL;
	windowSize = core::dimension2d< u32 >( 640, 480 );
	allowSmallSize = false;
	playMusic = true;
	enableJoystick = false;
	joystickChosen = 1;
	numBots = 0;
	numPlayers = 1;
	markTrails = false;

	#ifdef DEBUG
		debug = true;
	#else
		debug = false;
	#endif

	if( !is_directory( prefsPath ) ) {
		wcout << L"Loading preferences from file " << prefsPath.wstring() << endl;
		boost::filesystem::wifstream prefsFile;
		prefsFile.open( prefsPath, ios::in );

		if( prefsFile.is_open() ) {
			wstring line;
			u16 lineNum = 0;

			while( prefsFile.good() ) {
				lineNum++;
				getline( prefsFile, line );
				line = line.substr( 0, line.find( L"//" ) ); //Filters out comments
				boost::algorithm::trim( line ); //Removes trailing and leading spaces
				boost::algorithm::to_lower( line );


				if( !line.empty() ) {
					wstring preference = boost::algorithm::trim_copy( line.substr( 0, line.find( '\t' ) ) );
					wstring choice = boost::algorithm::trim_copy( line.substr( line.find( '\t' ) ) );

					if( preference == L"number of bots:" ) {
						try {
							u16 choiceAsInt = boost::lexical_cast< u16 >( choice ); //Used u16 here because boost::lexical_cast refuses to convert from a wstring to a u8. u8 is technically an unsigned char, but we're using that type as an 8-bit unsigned integer
							//choiceAsInt -= '0'; //Convert from a character to its equivalent number

							if( choiceAsInt < numPlayers ) {
								numBots = choiceAsInt;
								wcout << L"Number of bots is " << static_cast<int>( choiceAsInt ) << endl;
							} else {
								wcerr << L"Warning: Number of bots not less than number of players (number of players may not have been read yet): " << static_cast<unsigned int>( choiceAsInt ) << endl;
								numBots = choiceAsInt;
							}
						} catch( boost::bad_lexical_cast error ) {
							wcerr << L"Error reading number of players preference (is it not a number?) on line " << lineNum << L": " << error.what() << endl;
						}
					} else if( preference == L"fullscreen:" ) {

						if( choice == L"true" ) {
							wcout << L"Fullscreen is ON" << endl;
							fullscreen = true;
						} else if( choice == L"false" ) {
							wcout << L"Fullscreen is OFF" << endl;
							fullscreen = false;
						} else {
							wcerr << L"Error reading fullscreen preference on line " << lineNum  << L": \"" << choice  << endl;//<< L"\"" << endl;
						}
					} else if( preference == L"mark player trails:" ) {

						if( choice == L"true" ) {
							wcout << L"Mark trails is ON" << endl;
							markTrails = true;
						} else if( choice == L"false" ) {
							wcout << L"Mark trails is OFF" << endl;
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

						if( debug == true ) {
							wcout << L"Debug is ON" << endl;

						} else {
							wcout << L"Debug is OFF" << endl;
						}
					} else if( preference == L"bits per pixel:" ) {
						try {
							u32 choiceAsInt = boost::lexical_cast<u32>( choice );

							if( choiceAsInt <= 32 ) {
								bitsPerPixel = choiceAsInt;
								wcout << L"Bits per pixel is " << choiceAsInt << endl;
							} else {
								wcerr << L"Warning: Bits per pixel not less than or equal to 32: " << choiceAsInt << endl;
								bitsPerPixel = choiceAsInt;
							}
						} catch( boost::bad_lexical_cast error ) {
							wcerr << L"Error reading bitsPerPixel preference (is it not a number?) on line " << lineNum << L": " << error.what() << endl;
						}

					} else if( preference == L"wait for vertical sync:" ) {

						if( choice == L"true" ) {
							wcout << L"Vertical sync is ON" << endl;
							vsync = true;
						} else if( choice == L"false" ) {
							wcout << L"Vertical sync is OFF" << endl;
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
							wcerr << L"Warning: Chosen type " << choice << L" is not supported on this system. ";

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
								wcerr << L"Error: No graphical output driver types are available. Using NULL type!!" << endl;
								driverType = video::EDT_NULL;
								choice = L"NULL";
							}
						}

						wcout << L"Driver type is " << choice << endl;

					} else if( preference == L"number of players:" ) {
						try {
							u16 choiceAsInt = boost::lexical_cast<u16>( choice ); //Used u16 here because boost::lexical_cast refuses to convert from a wstring to a u8. u8 is technically an unsigned char, but we're using that type as an 8-bit unsigned integer
							//choiceAsInt -= '0'; //Convert from a character to its equivalent number

							if( choiceAsInt > 255 ) { //The maximum number of players is whatever a u8 can hold, presumably 255 (the 8 is the minimum number of bits, it may not be the true number).
								choiceAsInt = choiceAsInt % 255;
							}

							if( choiceAsInt <= 4 && choiceAsInt > 0) {
								numPlayers = choiceAsInt;
								wcout << L"Number of players is " << static_cast<int>( choiceAsInt ) << endl;
							} else if( choiceAsInt > 4 ) {
								wcerr << L"Warning: Number of players not less than or equal to 4: " << static_cast<unsigned int>( choiceAsInt ) << endl;
								numPlayers = choiceAsInt;
							} else {
								wcerr << L"Warning: Number of players is zero or not a number: " << static_cast<unsigned int>( choiceAsInt ) << L". Setting number of players to default." << endl;
							}
						} catch( boost::bad_lexical_cast error ) {
							wcerr << L"Error reading number of players preference (is it not a number?) on line " << lineNum << L": " << error.what() << endl;
						}

					} else if( preference == L"window size:" ) {
						size_t locationOfX = choice.find( L"x" );
						wstring width = choice.substr( 0, locationOfX );
						wstring height = choice.substr( locationOfX + 1 );
						wcout << L"Window size: " << width << L"x" << height << endl;

						u32 widthAsInt = boost::lexical_cast<u32>( width );
						u32 heightAsInt = boost::lexical_cast<u32>( height );

						if( widthAsInt < 320 || heightAsInt < 240 ) {
							wcerr << L"Error reading window size: Width and/or height are really really tiny. Sorry but you'll have to recompile the game yourself if you want a window that small." << endl;
						} else if( widthAsInt == 320 && heightAsInt == 240 ) {
							wcout << L"Rock on, CGA graphics. Rock on." << endl;
							windowSize = core::dimension2d<u32>( widthAsInt, heightAsInt );
						} else {
							windowSize = core::dimension2d<u32>( widthAsInt, heightAsInt );
						}

					} else if( preference == L"play music:" ) {
						if( choice == L"true" ) {
							wcout << L"Play music is ON" << endl;
							playMusic = true;
						} else if( choice == L"false" ) {
							wcout << L"Play music is OFF" << endl;
							playMusic = false;
						} else {
							wcerr << L"Error reading play music preference on line " << lineNum << L": \"" << choice << L"\"" << endl;
						}

					} else if( preference == L"network port:" ) {
						wcout << L"Network port: " << choice << endl;
						u16 choiceAsInt = boost::lexical_cast<u16>( choice );
						network.setPort( choiceAsInt );
					} else if( preference == L"enable joystick:" ) {
						if( choice == L"true" ) {
							wcout << L"Joystick is ENABLED" << endl;
							enableJoystick = true;
						} else if( choice == L"false" ) {
							wcout << L"Joystick is DISABLED" << endl;
							enableJoystick = false;
						} else {
							wcerr << L"Error reading enable joystick preference on line " << lineNum << L": \"" << choice << L"\"" << endl;
						}
					} else if( preference == L"joystick chosen:" ) {
						wcout << L"Joystick chosen: " << choice << endl;
						u32 choiceAsInt = boost::lexical_cast<u32>( choice );
						joystickChosen = choiceAsInt;
					} else if( preference == L"always server:" ) {
						if( choice == L"true" ) {
							wcout << L"This is always a server" << endl;
							isServer = true;
						} else if( choice == L"false" ) {
							wcout << L"This is not always a server" << endl;
							isServer = false;
						} else {
							wcerr << L"Error reading \"always server\" preference on line " << lineNum << L": \"" << choice << L"\"" << endl;
						}
					} else {
						wcerr << L"Unrecognized preference on line " << lineNum << L": \"" << line << L"\"" << endl;
					}
				} else {
					//wcout << L"Line is empty." << endl;
				}
			}

			prefsFile.close();
		}
	} else {
		wcerr << L"Error: Prefs file is a directory. Cannot save or load prefs." << endl;
	}

	if( !isServer ) {
		wcout << L"(S)erver or (c)lient? " << endl;
		wchar_t a;
		wcin >> a;
		isServer = ( a == L's' || a == L'S' );
		myPlayer = 0;
	}
}

void GameManager::startLoadingScreen() {
	showingLoadingScreen = true;
	timeStartedLoading = timer->getRealTime();
}

GameManager::GameManager() {
	fontFile = "Ubuntu-R.ttf";
	isServer = false;
	antiAliasFonts = true;

	device = createDevice( video::EDT_NULL ); //Must create a null device before calling readPrefs();

	if( !device ) {
		wcerr << L"Error: Cannot create null device. Something is definitely wrong here!" << endl;
		exit( -1 );
	}

	readPrefs();

	if( fullscreen && !allowSmallSize ) {
		windowSize = device->getVideoModeList()->getDesktopResolution();
	}

	viewportSize.set( windowSize.Width - ( windowSize.Width / 6 ), windowSize.Height - 1 );

	bool sbuffershadows = false; //Would not be visible anyway since this game is 2D
	IEventReceiver* receiver = this;

	device->closeDevice(); //Signals to the device that it needs to close itself on next run()
	device->run(); //This is next run()
	device->drop(); //Cleans up after the device

	device = createDevice( driverType, windowSize, bitsPerPixel, fullscreen, sbuffershadows, vsync, receiver );

	if( !device ) {
		wcerr << L"Error: Cannot create device" << endl;
	}

	if( debug ) {
		device->getLogger()->setLogLevel( ELL_INFORMATION );
	} else {
		device->getLogger()->setLogLevel( ELL_ERROR );
	}

	driver = device->getVideoDriver();

	if( !driver ) {
		wcerr << L"Error: Cannot get video driver" << endl;
	}

	bgscene = device->getSceneManager();

	if( !bgscene ) {
		wcerr << L"Error: Cannot get scene manager" << endl;
	}

	gui = device->getGUIEnvironment();

	if( !gui ) {
		wcerr << L"Error: Cannot get GUI environment" << endl;
	} else {
		for( u32 i = 0; i < gui::EGDC_COUNT ; i++ ) {
			video::SColor guiSkinColor = gui->getSkin()->getColor( static_cast<gui::EGUI_DEFAULT_COLOR>( i ) );
			guiSkinColor.setAlpha( 255 );
			gui->getSkin()->setColor( static_cast<gui::EGUI_DEFAULT_COLOR>( i ), guiSkinColor );
		}
	}

	if( playMusic ) {

		if( SDL_Init( SDL_INIT_AUDIO ) == -1 ) {
			wcerr << L"Cannot initialize SDL audio." << endl;
		}

		int audio_rate = 44100; //44.1 KHz is the standard sample rate for CDs, and so makes a good 'lowest common denominator' for anything related to audio.
		Uint16 audio_format = AUDIO_S16SYS;
		int audio_channels = 2;
		int audio_buffers = 4096;

		if( Mix_OpenAudio( audio_rate, audio_format, audio_channels, audio_buffers ) != 0 ) {
			wcerr << L"Unable to initialize audio: " << Mix_GetError() << endl;
		}

		Mix_QuerySpec( &audio_rate, &audio_format, &audio_channels );//Don't assume we got everything we asked for above

		music = NULL;
		makeMusicList();

		if( playMusic ) {  //playMusic may be set to false if makeMusicList() can't find any songs to play
			loadNextSong();
		}

		//Mix_HookMusicFinished(musicFinished);
	}

	cols = 1;
	rows = 1;
	maze = new MazeCell *[cols];

	for( int i = 0 ; i < cols ; i++ )
		maze[i] = new MazeCell[rows];

	resizeMaze( 2, 2 );

	//currentFile = boost::filesystem::directory_iterator(".");
	currentProTip = 0;

	device->setWindowCaption( L"Cybrinth" );

	loadFonts();

	driver->setTextureCreationFlag( video::ETCF_NO_ALPHA_CHANNEL, false );
	driver->setTextureCreationFlag( video::ETCF_OPTIMIZED_FOR_QUALITY, true );

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
	proTipStart = L"Pro tip: ";
	stats = L"Player stats";
	steps = L"Steps";

	loadMaze.setY( 100 );
	saveMaze.setY( 200 );
	exitGame.setY( 300 );
	backToGame.setY( 400 );

	timer = device->getTimer();

	player.resize( numPlayers );
	playerStart.resize( numPlayers );

	if( numBots > numPlayers ) {
		numBots = numPlayers;
	}

	if( numBots <= numPlayers && numBots > 0 ) {
		bot.resize( numBots );
		for( u8 i = 0; i < numBots; i++ ) {
			bot[ i ].setPlayer( numPlayers - ( i + 1 ) ) ;
		}
	}

	for( u8 p = 0; p < numPlayers; p++ ) {
		player[p].setColorBasedOnNum( p );
		player[p].loadImage( driver );
	}

	goal.loadImage( driver );

	network.setup( isServer );

	if( debug && enableJoystick && device->activateJoysticks( joystickInfo ) ) { //Fills joystickInfo with info about each joystick
		wcout << L"Joystick support is enabled and " << joystickInfo.size() << L" joystick(s) are present." << std::endl;

		for( u32 joystick = 0; joystick < joystickInfo.size(); ++joystick ) {
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

	loadProTips();
}

void GameManager::resizeMaze( u8 newCols, u8 newRows ) {
	u8 oldCols = cols;
	u8 oldRows = rows;

	if( debug && ( newCols < oldCols || newRows < oldRows ) ) {
		wcerr << L"Warning: New maze size smaller than old in some dimension. newCols: " << static_cast<unsigned int>( newCols ) << L" oldCols: " << static_cast<unsigned int>( oldCols ) << L" newRows: " << static_cast<unsigned int>( newRows ) << L" oldRows: " << static_cast<unsigned int>( oldRows ) << endl;
	}

	MazeCell** temp = new MazeCell *[newCols];

	for( int i = 0 ; i < newCols ; i++ )
		temp[i] = new MazeCell[newRows];

	unsigned int colsToCopy;

	if( newCols > oldCols ) {
		colsToCopy = oldCols;
	} else {
		colsToCopy = newCols;
	}

	unsigned int rowsToCopy;

	if( newRows > oldRows ) {
		rowsToCopy = oldRows;
	} else {
		rowsToCopy = newRows;
	}

	for( unsigned int x = 0; x < colsToCopy; x++ ) {
		for( unsigned int y = 0; y < rowsToCopy; y++ ) {
			temp[x][y] = maze[x][y];
		}
	}

	for( int i = 0 ; i < oldCols ; i++ )
		delete [] maze[i] ;

	delete [] maze ;

	cols = newCols;
	rows = newRows;

	maze = temp;
}

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

bool GameManager::loadFromFile( boost::filesystem::path src ) {
	try {
		if( !exists( src ) ) {
			wcerr << L"Error: File not found: " << src << endl;
			return false;
		} else if( is_directory( src ) ) {
			wcerr << L"Error: Directory specified, file needed. " << src << endl;
			return false;
		}

		boost::filesystem::wifstream file; //Identical to a standard C++ fstream, except it takes Boost paths
		file.open( src, ios::in );

		if( file.is_open() ) {
			resetThings();
			file >> randomSeed;
			file.close();
			srand(randomSeed);
			makeRandomLevel();
		} else {
			wcerr << L"Cannot open file " << src << endl;
		}

		return true;
	} catch( const boost::filesystem::filesystem_error& e ) {
		wcerr << e.what() << endl;
		return false;
	} catch( exception& e ) {
		wcerr << e.what() << endl;
		return false;
	}

	return false;
}

void GameManager::loadFromFile() {
	loadFromFile( L"default.maz" );
}

bool GameManager::saveToFile( boost::filesystem::path dest ) {
	try {
		if( is_directory( dest ) ) {
			wcerr << L"Error: Directory specified, file needed. " << dest << endl;
			return false;
		}

		boost::filesystem::wofstream file; //Identical to a standard C++ wofstream, except it takes Boost paths
		file.open( dest, ios::out );

		if( file.is_open() ) {
			file << randomSeed;
			irr::core::stringw message( L"This maze has been saved to the file " );
			message += stringConverter.convert( dest.wstring() );
			gui->addMessageBox( L"Maze saved", message.c_str() );
			file.close();
		} else {
			irr::core::stringw message( L"Cannot save to file " );
			message += stringConverter.convert( dest.wstring() );
			wcerr << message.c_str() << endl;
			gui->addMessageBox( L"Maze NOT saved", message.c_str() );
		}

		return true;
	} catch( const boost::filesystem::filesystem_error& e ) {
		wcerr << e.what() << endl;
		return false;
	} catch( exception& e ) {
		wcerr << e.what() << endl;
		return false;
	}

	return false;
}

void GameManager::saveToFile() {
	saveToFile( L"default.maz" );
}

void GameManager::setupBackground() {
	u8 availableBackgrounds = 1;

	backgroundChosen = rand() % availableBackgrounds;
	wcout << L"Background chosen: " << backgroundChosen << endl;

	switch( backgroundChosen ) {
		case 0: {
				// create a particle system
				scene::ICameraSceneNode* camera = bgscene->addCameraSceneNode();
				camera->setPosition( core::vector3df( 0, 0, -150 ) );
				scene::IParticleSystemSceneNode* ps = bgscene->addParticleSystemSceneNode( false );

				scene::IParticleEmitter* em = ps->createBoxEmitter(
												  camera->getViewFrustum()->getBoundingBox(), //core::aabbox3d<f32>(-7,-7,-7,7,7,7), // emitter size
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

				video::IImage* pixelImage = driver->createImage( video::ECF_A8R8G8B8, core::dimension2d<u32>( 1, 1 ) );
				pixelImage->fill( WHITE );
				pixelImage->setPixel( 0, 0, WHITE, false );
				video::ITexture* pixelTexture = driver->addTexture( "pixel", pixelImage );
				ps->setMaterialTexture( 0, pixelTexture );
			}
			break;
		default:
			break;
	}
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

void GameManager::recurseRandom( u8 x, u8 y, u16 depth, u16 numSoFar ) {
	device->run();
	drawAll();

	maze[x][y].visited = true;
	maze[x][y].id = numSoFar;

	for( u8 p = 0; p < numPlayers; p++ ) {
		if( depth >= playerStart[p].distanceFromExit ) {
			playerStart[p].setPos( x, y );
			playerStart[p].distanceFromExit = depth;
		}
	}

	bool keepGoing = true;

	while( keepGoing ) {
		numSoFar += 1;

		switch( rand() % 4 ) {
			case 0:

				if( x > 0 && maze[x-1][y].visited == false ) {
					/*if( numLocksPlaced < numLocks ) {
						maze[x][y].setLeft( 'l' );
						numLocksPlaced++;
					} else {
						maze[x][y].setLeft( '0' );
					}*/
					maze[x][y].setLeft( '0' );

					recurseRandom( x - 1, y, depth + 1, numSoFar );
				}

				break;
			case 1:

				if( x < cols - 1 && maze[x+1][y].visited == false ) {
					/*if( numLocksPlaced < numLocks ) {
						maze[x+1][y].setLeft( 'l' );
						numLocksPlaced++;
					} else {
						maze[x+1][y].setLeft( '0' );
					}*/
					maze[x+1][y].setLeft( '0' );

					recurseRandom( x + 1, y, depth + 1, numSoFar );
				}

				break;
			case 2:

				if( y > 0 && maze[x][y-1].visited == false ) {
					/*if( numLocksPlaced < numLocks ) {
						maze[x][y].setTop( 'l' );
						numLocksPlaced++;
					} else {
						maze[x][y].setTop( '0' );
					}*/
					maze[x][y].setTop( '0' );

					recurseRandom( x, y - 1, depth + 1, numSoFar );
				}

				break;
			case 3:
			default:

				if( y < rows - 1 && maze[x][y+1].visited == false ) {
					/*if( numLocksPlaced < numLocks ) {
						maze[x][y+1].setTop( 'l' );
						numLocksPlaced++;
					} else {
						maze[x][y+1].setTop( '0' );
					}*/
					maze[x][y+1].setTop( '0' );

					recurseRandom( x, y + 1, depth + 1, numSoFar );
				}

				break;
		}

		keepGoing = false;

		if(( x > 0 && maze[x-1][y].visited == false )
				|| ( x < cols - 1 && maze[x+1][y].visited == false )
				|| ( y > 0 && maze[x][y-1].visited == false )
				|| ( y < rows - 1 && maze[x][y+1].visited == false )
		  ) {
			keepGoing = true;
		}
	}
}

void GameManager::makeRandomLevel() {
	drawAll();

	srand( randomSeed ); //randomSeed is set either by resetThings() or by loadFromFile()
	u8 tempCols = rand() % 28 + 2;
	//wcout << L"cols: " << static_cast<int>(tempCols) << endl;
	u8 tempRows = tempCols + ( rand() % 5 );
	//wcout << L"rows: " << static_cast<int>(tempRows) << endl;
	resizeMaze( tempCols, tempRows );

	for( u8 x = 0; x < cols; x++ ) {
		for( u8 y = 0; y < rows; y++ ) {
			//wcout << L"y: " << static_cast<int>(y) << endl;
			maze[x][y].setTop( '1' );
			maze[x][y].setLeft( '1' );
			maze[x][y].setRight( '0' );
			maze[x][y].setBottom( '0' );
			maze[x][y].visited = false;
		}
	}

	for( u8 p = 0; p < numPlayers; p++ ) {
		playerStart[p].reset();
	}

	//do {
	//player.setX(rand() % cols);
	//player.setY(rand() % rows);
	goal.setX( rand() % cols );
	goal.setY( rand() % rows );
	//} while (player.getX() == goal.getX() && player.getY() == goal.getY());

	numLocks = ( rand() % 10 ) % cols;
	//numLocks = rand() % ( cols * rows ); //Uncomment this for a crazy number of keys!

	recurseRandom( goal.getX(), goal.getY(), 0, 0 );

	//Add walls at maze borders
	for( u8 x = 0; x < cols; x++ ) {
		maze[x][0].setTop( '1' );
		maze[x][rows-1].setBottom( '1' );
	}

	for( u8 y = 0; y < rows; y++ ) {
		maze[0][y].setLeft( '1' );
		maze[cols-1][y].setRight( '1' );
	}

	for( u8 x = 1; x < cols; x++ ) {
		for( u8 y = 0; y < rows; y++ ) {
			maze[x-1][y].setRight( maze[x][y].getLeft() );
		}
	}

	for( u8 x = 0; x < cols; x++ ) {
		for( u8 y = 1; y < rows; y++ ) {
			maze[x][y-1].setBottom( maze[x][y].getTop() );
		}
	}

	//Find all dead ends. I'm sure it would be more efficient to do this during maze generation rather than going back through afterward, but I can't be bothered with that now.
	vector<u8> deadEndsX;
	vector<u8> deadEndsY;

	for( u8 x = 0; x < cols; x++ ) {
		for( u8 y = 0; y < rows; y++ ) {
			//wcout << L"x: " << static_cast<unsigned int>(x) << L"\ty: " << static_cast<unsigned int>(y) << endl;
			if( maze[x][y].isDeadEnd() ) {
				deadEndsX.push_back( x );
				deadEndsY.push_back( y );
			}
		}
	}

	//Remove player starts from list of dead ends
	for( u8 p = 0; p < numPlayers; p++ ) {
		for( u32 i = 0; i < deadEndsX.size(); i++ ) {
			if( playerStart[p].getX() == deadEndsX[i] && playerStart[p].getY() == deadEndsY[i] ) {
				deadEndsX.erase( deadEndsX.begin() + i );
				deadEndsY.erase( deadEndsY.begin() + i );
			}
		}
	}

	//Remove goal from list of dead ends
	for( u32 i = 0; i < deadEndsX.size(); i++ ) {
		if( goal.getX() == deadEndsX[i] && goal.getY() == deadEndsY[i] ) {
			deadEndsX.erase( deadEndsX.begin() + i );
			deadEndsY.erase( deadEndsY.begin() + i );
		}
	}

	if( numLocks > deadEndsX.size() ) {
		numLocks = deadEndsX.size();
	}

	u8 numKeys = numLocks;

	//Place keys in dead ends
	vector<u8> keyPlaceX;
	vector<u8> keyPlaceY;

	for( u8 k = 0; k < numKeys; k++ ) {
		vector<u8> chosenPlaces;

		//Pick one of the dead ends randomly.
		if( deadEndsX.size() == 0 ) {
			deadEndsX.push_back( playerStart[0].getX() );
			deadEndsY.push_back( playerStart[0].getY() );
		}

		u8 chosen = rand() % deadEndsX.size();

		//Finally, create a key and put it there.
		Collectable temp;
		temp.setX( deadEndsX.at( chosen ) );
		temp.setY( deadEndsY.at( chosen ) );
		temp.setType( COLLECTABLE_KEY );
		temp.loadImage( driver );
		stuff.push_back( temp );

		//Remove chosen from the list of dead ends so no other keys go there
		deadEndsX.erase( deadEndsX.begin() + chosen );
		deadEndsY.erase( deadEndsY.begin() + chosen );
	}

	for( u8 p = 0; p < numPlayers; p++ ) {
		player[p].setPos( playerStart[p].getX(), playerStart[p].getY() );
	}

	if( numLocks > 0 ) {
		//Place locks
		if( maze[goal.getX()][goal.getY()].getTop() == '0' ) {
			maze[goal.getX()][goal.getY()].setTop( 'l' );
		} else if( maze[goal.getX()][goal.getY()].getLeft() == '0' ) {
			maze[goal.getX()][goal.getY()].setLeft( 'l' );
		} else if( maze[goal.getX()][goal.getY() + 1].getTop() == '0' ) {
			maze[goal.getX()][goal.getY() + 1].setTop( 'l' );
		} else if( maze[goal.getX() + 1][goal.getY()].getLeft() == '0' ) {
			maze[goal.getX() + 1][goal.getY()].setLeft( 'l' );
		}

		u8 numLocksPlaced = 1;

		while( device->run() != false && numLocksPlaced < numLocks && timer->getTime() < timeStartedLoading + loadingDelay ) {
			u8 tempX = rand() % cols;
			u8 tempY = rand() % rows;

			if( maze[tempX][tempY].getTop() == '0' ) {
				maze[tempX][tempY].setTop( 'l' );

				if( canGetToAllCollectables( playerStart[0].getX(), playerStart[0].getY() ) ) {
					numLocksPlaced += 1;
					wcout << L"Placed lock " << static_cast<unsigned int>( numLocksPlaced ) << L" at " << static_cast<unsigned int>( tempX ) << L"x" << static_cast<unsigned int>( tempY ) << endl;
				} else {
					maze[tempX][tempY].setTop( '0' );
				}
			} else if( maze[tempX][tempY].getLeft() == '0' ) {
				maze[tempX][tempY].setLeft( 'l' );

				if( canGetToAllCollectables( playerStart[0].getX(), playerStart[0].getY() ) ) {
					numLocksPlaced += 1;
					wcout << L"Placed lock " << static_cast<unsigned int>( numLocksPlaced ) << L" at " << static_cast<unsigned int>( tempX ) << L"x" << static_cast<unsigned int>( tempY ) << endl;
				} else {
					maze[tempX][tempY].setLeft( '0' );
				}
			}
		}

		timer->stop();
		timer->setTime( 0 );
		wcout << L"numLocksPlaced: " << static_cast<unsigned int>( numLocksPlaced ) << L"\tnumLocks: " << static_cast<unsigned int>( numLocks ) << endl;

		if( numLocksPlaced < numLocks ) {
			wcout << L"test" << endl;
			int keysToRemove = numLocks - numLocksPlaced;

			for( u32 i = 0; ( i < stuff.size() && keysToRemove > 0 ); i++ ) {
				wcout << L"keysToRemove: " << static_cast<unsigned int>( keysToRemove ) << endl;

				if( stuff[i].getType() == COLLECTABLE_KEY ) {
					stuff.erase( stuff.begin() + i );
					i = 0;
					keysToRemove -= 1;
				}
			}
		}

		numKeys = numLocks = numLocksPlaced;
	}

	for( u8 x = 0; x < cols; x++ ) {
		for( u8 y = 0; y < rows; y++ ) {
			maze[x][y].visited = false;
		}
	}

	for( u8 p = 0; p < numPlayers; p++ ) {
		maze[ playerStart[ p ].getX()][ playerStart[ p ].getY()].visited = true;
		maze[ playerStart[ p ].getX()][ playerStart[ p ].getY()].setVisitorColor( player[ p ].getColorTwo() );
	}

	//Set up bots;
	if( numBots > 0 ) {
		for( u8 i = 0; i < numBots; i++ ) {
			bot[ i ].setup( maze, cols, rows, this );
		}
	}
}

bool GameManager::existsAnythingAt( u8 x, u8 y ) {
	bool result = false;

	if( goal.getX() == x && goal.getY() == y ) {
		result = true;
	}

	for( u32 i = 0; ( result == false && i < stuff.size() ); i++ ) {
		if( stuff[i].getX() == x && stuff[i].getY() == y ) {
			result = true;
		}
	}

	return result;
}

bool GameManager::canGetToAllCollectables( u8 startX, u8 startY ) {
	bool result = true;

	for( u32 i = 0; ( i < stuff.size() && result == true ); i++ ) {
		for( u8 x = 0; x < cols; x++ ) {
			for( u8 y = 0; y < rows; y++ ) {
				maze[x][y].visited = false;
			}
		}

		bool otherResult = canGetTo( startX, startY, stuff[i].getX(), stuff[i].getY() );

		if( otherResult == false ) {
			result = false;
		}
	}

	return result;
}

bool GameManager::canGetTo( u8 startX, u8 startY, u8 goalX, u8 goalY ) {
	bool found = false;
	//wcout << L"Searching for a way from " << static_cast<unsigned int>( startX ) << L"x" << static_cast<unsigned int>( startY ) << L" to " << static_cast<unsigned int>( goalX ) << L"x" << static_cast<unsigned int>( goalY ) << endl;
	maze[startX][startY].visited = true;

	if( startX == goalX && startY == goalY ) {
		found = true;
	} else {
		if( startY > 0 && maze[startX][startY].getTop() == '0' && maze[startX][startY - 1].visited == false ) {
			found = canGetTo( startX, startY - 1, goalX, goalY );
		}

		if( found == false && startY < ( rows - 1 ) && maze[startX][startY + 1].getTop() == '0' && maze[startX][startY + 1].visited == false ) {
			found = canGetTo( startX, startY + 1, goalX, goalY );
		}

		if( found == false && startX < ( cols - 1 ) && maze[startX + 1][startY].getLeft() == '0' && maze[startX + 1][startY].visited == false ) {
			found = canGetTo( startX + 1, startY, goalX, goalY );
		}

		if( found == false && startX > 0 && maze[startX][startY].getLeft() == '0' && maze[startX - 1][startY].visited == false ) {
			found = canGetTo( startX - 1, startY, goalX, goalY );
		}
	}

	return found;
}

void GameManager::resetThings() {
	randomSeed = time( NULL );
	loadingDelay = 1000 + ( rand() % 5000 );

	winners.clear();
	stuff.clear();
	showingMenu = false;
	numKeysFound = 0;
	numLocks = 0;
	donePlaying = false;

	for( u8 p = 0; p < numPlayers; p++ ) {
		playerStart[p].reset();
		player[ p ].stepsTaken = 0;
	}

	for( u32 i = 0; i < stuff.size(); i++ ) {
		stuff[i].loadImage( driver );
	}

	for( u8 x = 0; x < cols; x++ ) {
		for( u8 y = 0; y < rows; y++ ) {
			maze[x][y].visited = false;
		}
	}

	won = false;
	bgscene->clear();
	setupBackground();
	timer->setTime( 0 );
	timer->start();
	startLoadingScreen();

	currentProTip = ( currentProTip + 1 ) % proTips.size();

	loadTipFont();
}

s32 GameManager::run() {

	while( device->run() && !donePlaying ) {
		resetThings();
		makeRandomLevel();

		while( device->run() && !won && !donePlaying ) {

			if( showingLoadingScreen && ( timer->getRealTime() > timeStartedLoading + loadingDelay ) ) {
				showingLoadingScreen = false;
			}

			/*These next 3 lines are for limiting processor usage. I really
			 *doubt they're all needed, so comment one or another if you
			 *experience slowness. Two are provided by Irrlicht and the other by SDL.*/
			//device->yield();
			device->sleep( 10 );
			//SDL_Delay( 10 );

			if( driver->getScreenSize() != windowSize ) { //If the window has been resized. Only here until Irrlicht implements proper window resize events.
				SEvent temp;
				temp.EventType = EET_USER_EVENT;
				temp.UserEvent.UserData1 = USER_EVENT_WINDOW_RESIZE;
				device->postEventFromUser( temp );
			}

			if( playMusic && !Mix_PlayingMusic() ) {
				loadNextSong();
			}

			if( ( !showingLoadingScreen && device->isWindowActive() ) || debug ) {
				cellWidth = ( viewportSize.Width ) / cols;
				cellHeight = ( viewportSize.Height ) / rows;

				if( showingMenu && !timer->isStopped() ) {
					timer->stop();
				} else if( !showingMenu && timer->isStopped() ) {
					timer->start();
				}

				//It's the bots' turn to move now.
				if( !( showingMenu || showingLoadingScreen ) && numBots > 0 ) {
					for( u8 i = 0; i < numBots; i++ ) {
						if( debug || allHumansAtGoal() || bot[ i ].lastTimeMoved < timer->getRealTime() - bot[ i ].movementDelay ) {
							bot[ i ].move();
						}
					}
				}

				//device->getCursorControl()->setVisible( showingMenu || showingLoadingScreen );
				device->getCursorControl()->setVisible( true );
				drawAll();

				//Check if any of the players have landed on a collectable item
				for( u8 p = 0; p < numPlayers; p++ ) {
					for( u32 s = 0; s < stuff.size(); s++ ) {
						if( player[p].getX() == stuff[s].getX() && player[p].getY() == stuff[s].getY() ) {
							switch( stuff[s].getType() ) {
								case COLLECTABLE_KEY: {
									numKeysFound++;
									stuff.erase( stuff.begin() + s );

									if( numKeysFound >= numLocks ) {
										for( u8 c = 0; c < cols; c++ ) {
											for( u8 r = 0; r < rows; r++ ) {
												maze[c][r].removeLocks();
											}
										}

										for( u8 b = 0; b < numBots; b++ ) {
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


				for( u8 p = 0; p < numPlayers; p++ ) {
					if(( player[p].getX() != goal.getX() ) || player[p].getY() != goal.getY() ) { //Require that all players be at the end
						//won = false;
					} else { //Make a list of who finished in what order
						bool alreadyFinished = false;
						for( u32 i = 0; i < winners.size() && !alreadyFinished; i++) {
							if( p == winners.at( i ) ) {
								alreadyFinished = true;
							}
						}

						if( !alreadyFinished ) {
							winners.push_back( p );
						}
					}
				}

				won = ( winners.size() >= numPlayers );

			} else {
				showingMenu = true;
				device->yield();
			}

			if( isServer ) {
				if( network.checkForConnections() != 0 ) {
					std::wcerr << L"Networking error." << std::endl;
				} else {
					//std::wcout << L"Checked for connections." << std::endl;
					if( network.hasNewPlayerConnected() ) {
						std::wcout << L"New connections exist." << std::endl;
						network.sendMaze( maze, cols, rows );
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
				//std::wcout << L"Checked for connections." << std::endl;
				if( network.receiveData() ) {
					std::wcout << L"Received data" << std::endl;
				} else {
					//std::wcout << L"Did not receive data" << std::endl;
				}
			}
		}

		timer->stop();

		if( !donePlaying ) {
			wcout << L"On to the next level!" << endl;
			wcout << L"Winners:";
			for(u32 i = 0; i < winners.size(); i++) {
				wcout << L" " << winners.at( i );
			}
			wcout << endl;
			startLoadingScreen();
		}
	}

	return 0;
}

bool GameManager::allHumansAtGoal() {
	vector< u8 > humanPlayers; //Get a list of players
	for( u8 p = 0; p < numPlayers; p++) {
		humanPlayers.push_back( p );
	}

	bool result = false;
	for( u8 b = 0; b < numBots; b++ ) { //Remove bots from the list
		u8 botPlayer = bot[ b ].getPlayer();
		for( u32 p = 0; p < humanPlayers.size(); p++ ) {
			if( humanPlayers[ p ] == botPlayer) {
				humanPlayers.erase( humanPlayers.begin() + p );
			}
		}
	}

	if( humanPlayers.size() > 0 ) {
		result = true;
		for( u32 p = 0; ( p < humanPlayers.size() && result == true ); p++ ) {
			if( !( player[ humanPlayers[ p ] ].getX() == goal.getX() && player[ humanPlayers[ p ] ].getY() == goal.getY() ) ) {
				result = false;
			}
		}

	}
	return result;
}

Player* GameManager::getPlayer( u8 p ) {
	if( p < numPlayers ) {
		return &player.at( p );
	} else {
		wcerr << "getPlayer() error: Request for player (" << static_cast<int>( p ) << ">= numPlayers (" << static_cast<int>( numPlayers ) << ")" << endl;
		return NULL;
	}
}

void GameManager::drawAll() {
	driver->beginScene();
	if( !showingLoadingScreen ) {
		drawBackground();

		for( u8 x = 0; x < cols; x++ ) { //It's inefficient to do this here and have similar nested loops below drawing the walls, but I want these drawn before the players, and the players drawn before the walls.
			for( u8 y = 0; y < rows; y++ ) {
				if( markTrails && maze[ x ][ y ].visited ) {
					s32 dotSize = cellWidth / 5;

					if( dotSize < 2 ) {
						dotSize = 2;
					}

					driver->draw2DRectangle( maze[ x ][ y ].getVisitorColor() , core::rect<s32>( core::position2d<s32>(( x * cellWidth ) + ( 0.5 * cellWidth ) - ( 0.5 * dotSize ), ( y * cellHeight ) + ( 0.5 * cellHeight ) - ( 0.5 * dotSize ) ), core::dimension2d<s32>( dotSize, dotSize ) ) );
				}
			}
		}

		for( u32 ps = 0; ps < playerStart.size(); ps++ ) { //Put this in a separate loop from the players (below) so that the players would all be drawn after the playerStarts.
			playerStart[ ps ].draw( driver, cellWidth, cellHeight );
		}

		for( u8 p = 0; p < numPlayers; p++ ) {
			player[ p ].draw( driver, cellWidth, cellHeight );
		}

		goal.draw( driver, cellWidth, cellHeight );

		video::SColor wallColor = WHITE;
		video::SColor lockColor = BROWN;

		for( u8 x = 0; x < cols; x++ ) {
			for( u8 y = 0; y < rows; y++ ) {
				if( maze[x][y].getTop() == '1' ) {
					driver->draw2DLine( core::position2d<s32>( x * cellWidth, y * cellHeight ), core::position2d<s32>(( x + 1 )*cellWidth, y * cellHeight ), wallColor );
				} else if( maze[x][y].getTop() == 'l' ) {
					driver->draw2DLine( core::position2d<s32>( x * cellWidth, y * cellHeight ), core::position2d<s32>(( x + 1 )*cellWidth, y * cellHeight ), lockColor );
				}

				if( maze[x][y].getLeft() == '1' ) {
					driver->draw2DLine( core::position2d<s32>( x * cellWidth, y * cellHeight ), core::position2d<s32>( x * cellWidth, ( y + 1 )*cellHeight ), wallColor );
				} else if( maze[x][y].getLeft() == 'l' ) {
					driver->draw2DLine( core::position2d<s32>( x * cellWidth, y * cellHeight ), core::position2d<s32>( x * cellWidth, ( y + 1 )*cellHeight ), lockColor );
				}

				//Only cells on the right or bottom edge of the maze should have anything other than '0' as right or bottom, and then it should only be a solid '1'
				if( maze[x][y].getRight() == '1' ) {
					driver->draw2DLine( core::position2d<s32>(( x + 1 )*cellWidth, y * cellHeight ), core::position2d<s32>(( x + 1 )*cellWidth, ( y + 1 )*cellHeight ), wallColor );
				}

				if( maze[x][y].getBottom() == '1' ) {
					driver->draw2DLine( core::position2d<s32>( x * cellWidth, ( y + 1 )*cellHeight ), core::position2d<s32>(( x + 1 )*cellWidth, ( y + 1 )*cellHeight ), wallColor );
				}
			}
		}

		for( u32 i = 0; i < stuff.size(); i++ ) {
			stuff[i].draw( driver, cellWidth, cellHeight );
		}

		if( showingMenu ) {
			newGame.draw( driver );
			loadMaze.draw( driver );
			saveMaze.draw( driver );
			exitGame.draw( driver );
			backToGame.draw( driver );
		}


		u32 spaceBetween = windowSize.Height / 30;
		u32 textY = spaceBetween;

		time_t currentTime = time( NULL );
		wchar_t clockTime[9];
		wcsftime( clockTime, 9, L"%H:%M:%S", localtime( &currentTime ) );
		core::dimension2d<u32> tempDimensions = clockFont->getDimension( core::stringw( clockTime ).c_str() );
		core::rect<s32> tempRectangle( viewportSize.Width + 1, textY, tempDimensions.Width + ( viewportSize.Width + 1 ), tempDimensions.Height + textY );
		clockFont->draw( clockTime, tempRectangle, LIGHTMAGENTA, true, true, &tempRectangle );

		core::stringw timeLabel( L"Time:" );
		textY += tempDimensions.Height;
		tempDimensions = textFont->getDimension( timeLabel.c_str() );
		tempRectangle = core::rect<s32>( viewportSize.Width + 1, textY, tempDimensions.Width + ( viewportSize.Width + 1 ), tempDimensions.Height + textY );
		textFont->draw( L"Time:", tempRectangle, YELLOW, true, true, &tempRectangle );
		core::stringw timerStr( "" );
		timerStr += ( timer->getTime() / 1000 );
		timerStr += L" seconds";
		textY += tempDimensions.Height;
		tempDimensions = textFont->getDimension( timerStr.c_str() );
		tempRectangle = core::rect<s32>( viewportSize.Width + 1, textY, tempDimensions.Width + ( viewportSize.Width + 1 ), tempDimensions.Height + textY );
		textFont->draw( timerStr, tempRectangle, YELLOW, true, true, &tempRectangle );

		core::stringw keysFoundStr( L"Keys found:" );
		textY += tempDimensions.Height;
		tempDimensions = textFont->getDimension( keysFoundStr.c_str() );
		tempRectangle = core::rect<s32>( viewportSize.Width + 1, textY, tempDimensions.Width + ( viewportSize.Width + 1 ), tempDimensions.Height + textY );
		textFont->draw( keysFoundStr, tempRectangle, YELLOW, true, true, &tempRectangle );

		core::stringw keyStr;
		keyStr += numKeysFound;
		keyStr += L"/";
		keyStr += numLocks;
		textY += tempDimensions.Height;
		tempDimensions = textFont->getDimension( keyStr.c_str() );
		tempRectangle = core::rect<s32>( viewportSize.Width + 1, textY, tempDimensions.Width + ( viewportSize.Width + 1 ), tempDimensions.Height + textY );
		textFont->draw( keyStr, tempRectangle, YELLOW, true, true, &tempRectangle );

		core::stringw seedLabel( L"Random seed:" );
		textY += tempDimensions.Height;
		tempDimensions = textFont->getDimension( seedLabel.c_str() );
		tempRectangle = core::rect<s32>( viewportSize.Width + 1, textY, tempDimensions.Width + ( viewportSize.Width + 1 ), tempDimensions.Height + textY );
		textFont->draw( seedLabel, tempRectangle, YELLOW, true, true, &tempRectangle );

		core::stringw seedStr( randomSeed );
		textY += tempDimensions.Height;
		tempDimensions = textFont->getDimension( seedStr.c_str() );
		tempRectangle = core::rect<s32>( viewportSize.Width + 1, textY, tempDimensions.Width + ( viewportSize.Width + 1 ), tempDimensions.Height + textY );
		textFont->draw( seedStr, tempRectangle, YELLOW, true, true, &tempRectangle );

		core::stringw headfor( L"Head for" );
		textY += tempDimensions.Height;
		tempDimensions = textFont->getDimension( headfor.c_str() );

		if( textY < (( windowSize.Height / 2 ) - tempDimensions.Height ) ) {
			textY = (( windowSize.Height / 2 ) - tempDimensions.Height );
		}

		if( numKeysFound >= numLocks ) {
			tempRectangle = core::rect<s32>( viewportSize.Width + 1, textY, tempDimensions.Width + ( viewportSize.Width + 1 ), tempDimensions.Height + textY );
			textFont->draw( headfor, tempRectangle, LIGHTMAGENTA, true, true, &tempRectangle );
		}

		core::stringw theexit( L"the exit!" );
		textY += tempDimensions.Height;
		tempDimensions = textFont->getDimension( theexit.c_str() );

		if( numKeysFound >= numLocks ) {
			tempRectangle = core::rect<s32>( viewportSize.Width + 1, textY, tempDimensions.Width + ( viewportSize.Width + 1 ), tempDimensions.Height + textY );
			textFont->draw( theexit, tempRectangle, LIGHTCYAN, true, true, &tempRectangle );
		}

		if( playMusic ) {
			core::stringw nowplaying( L"Now playing:" );
			textY += tempDimensions.Height;
			tempDimensions = textFont->getDimension( nowplaying.c_str() );
			tempRectangle = core::rect<s32>( viewportSize.Width + 1, textY, tempDimensions.Width + ( viewportSize.Width + 1 ), tempDimensions.Height + textY );
			textFont->draw( nowplaying, tempRectangle, YELLOW, true, true, &tempRectangle );

			textY += tempDimensions.Height;
			tempDimensions = musicTagFont->getDimension( musicTitle.c_str() );
			tempRectangle = core::rect<s32>( viewportSize.Width + 1, textY, tempDimensions.Width + ( viewportSize.Width + 1 ), tempDimensions.Height + textY );
			musicTagFont->draw( musicTitle, tempRectangle, LIGHTGREEN, true, true, &tempRectangle );

			core::stringw by( L"by" );
			textY += tempDimensions.Height;
			tempDimensions = textFont->getDimension( by.c_str() );
			tempRectangle = core::rect<s32>( viewportSize.Width + 1, textY, tempDimensions.Width + ( viewportSize.Width + 1 ), tempDimensions.Height + textY );
			textFont->draw( by, tempRectangle, YELLOW, true, true, &tempRectangle );

			textY += tempDimensions.Height;
			tempDimensions = musicTagFont->getDimension( musicArtist.c_str() );
			tempRectangle = core::rect<s32>( viewportSize.Width + 1, textY, tempDimensions.Width + ( viewportSize.Width + 1 ), tempDimensions.Height + textY );
			musicTagFont->draw( musicArtist, tempRectangle, LIGHTGREEN, true, true, &tempRectangle );

			core::stringw fromalbum( L"from album" );
			textY += tempDimensions.Height;
			tempDimensions = textFont->getDimension( fromalbum.c_str() );
			tempRectangle = core::rect<s32>( viewportSize.Width + 1, textY, tempDimensions.Width + ( viewportSize.Width + 1 ), tempDimensions.Height + textY );
			textFont->draw( fromalbum, tempRectangle, YELLOW, true, true, &tempRectangle );

			textY += tempDimensions.Height;
			tempDimensions = musicTagFont->getDimension( musicAlbum.c_str() );
			tempRectangle = core::rect<s32>( viewportSize.Width + 1, textY, tempDimensions.Width + ( viewportSize.Width + 1 ), tempDimensions.Height + textY );
			musicTagFont->draw( musicAlbum, tempRectangle, LIGHTGRAY, true, true, &tempRectangle );
		}

		gui->drawAll();
	} else {
		core::dimension2d<u32> tempDimensions = loadingFont->getDimension( loading.c_str() );
		u32 textY = 0;
		core::rect< s32 > tempRectangle( ( windowSize.Width / 2 ) - ( tempDimensions.Width / 2 ), textY, ( windowSize.Width / 2 ) + ( tempDimensions.Width / 2 ), tempDimensions.Height + textY );
		loadingFont->draw( loading, tempRectangle, YELLOW, true, true, &tempRectangle );

		textY = tempDimensions.Height + textY + 1;
		tempDimensions = tipFont->getDimension( proTipStart.c_str() );
		tempRectangle = core::rect< s32 >( 0, textY, tempDimensions.Width, tempDimensions.Height + textY );
		tipFont->draw( proTipStart, tempRectangle, LIGHTCYAN, true, true, &tempRectangle );

		u32 textX = tempDimensions.Width + 1;
		tempDimensions = tipFont->getDimension( proTips.at( currentProTip ).c_str() );
		tempRectangle = core::rect< s32 >( textX, textY, tempDimensions.Width + textX, tempDimensions.Height + textY );
		tipFont->draw( proTips.at( currentProTip ), tempRectangle, WHITE, true, true, &tempRectangle );
	}

	driver->endScene();
}

void GameManager::movePlayerOnX( u8 p, s8 direction ) {
	if( numPlayers > p ) {
		if( direction < 0 ) {
			if( player[ p ].getX() > 0 && maze[ player[ p ].getX() ][ player[ p ].getY() ].getLeft() == '0' ) {
				player[ p ].moveX( -1 );
			}
		} else {
			if( player[ p ].getX() < ( cols - 1 ) && maze[ player[ p ].getX() + 1 ][ player[ p ].getY() ].getLeft() == '0' ) {
				player[ p ].moveX( 1 );
			}
		}

		network.sendPlayerPos( p, player[p].getX(), player[p].getY() );
		maze[ player[ p ].getX()][ player[ p ].getY()].visited = true;

		if( player[ p ].stepsTaken % 2 == 0 ) {
			maze[ player[ p ].getX()][ player[ p ].getY()].setVisitorColor( player[ p ].getColorTwo() );
		} else {
			maze[ player[ p ].getX()][ player[ p ].getY()].setVisitorColor( player[ p ].getColorOne() );
		}
	}
}

void GameManager::movePlayerOnY( u8 p, s8 direction ) {
	if( numPlayers > p ) {
		if( direction < 0 ) {
			if( player[p].getY() > 0 && maze[player[p].getX()][player[p].getY()].getTop() == '0' ) {
				player[p].moveY( -1 );
			}
		} else {
			if( player[p].getY() < ( rows - 1 ) && maze[player[p].getX()][player[p].getY() + 1].getTop() == '0' ) {
				player[p].moveY( 1 );
			}
		}

		network.sendPlayerPos( p, player[p].getX(), player[p].getY() );
		maze[ player[ p ].getX()][ player[ p ].getY()].visited = true;

		if( player[ p ].stepsTaken % 2 == 0 ) {
			maze[ player[ p ].getX()][ player[ p ].getY()].setVisitorColor( player[ p ].getColorTwo() );
		} else {
			maze[ player[ p ].getX()][ player[ p ].getY()].setVisitorColor( player[ p ].getColorOne() );
		}
	}
}

bool GameManager::OnEvent( const SEvent& event ) {
	switch( event.EventType ) {
		case EET_KEY_INPUT_EVENT: {
				if( event.KeyInput.PressedDown && !( showingMenu || showingLoadingScreen ) ) {
					switch( event.KeyInput.Key ) {
						case KEY_SNAPSHOT:
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
						case KEY_ESCAPE: {
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
								if( exitGame.isWithin( event.MouseInput.X, event.MouseInput.Y ) ) {
									device->closeDevice();
									donePlaying = true;
									return true;
								} else if( loadMaze.isWithin( event.MouseInput.X, event.MouseInput.Y ) ) {
									fileChooser = gui->addFileOpenDialog( L"Select a Maze", true, 0, -1 );
									//wcout << L"Directory: " << fileChooser->getDirectoryName().c_str() << endl;
									//loadFromFile();
									return true;
								} else if( saveMaze.isWithin( event.MouseInput.X, event.MouseInput.Y ) ) {
									saveToFile();
									return true;
								} else if( newGame.isWithin( event.MouseInput.X, event.MouseInput.Y ) ) {
									//randomSeed = time( NULL );
									resetThings();
									makeRandomLevel();

									/*for( u8 p = 0; p < numPlayers; p++ ) {
										player[p].setPos( playerStart[p].getX(), playerStart[p].getY() );
									}*/

									return true;
								} else if( backToGame.isWithin( event.MouseInput.X, event.MouseInput.Y ) ) {
									showingMenu = false;
									return true;
								}
							}

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

							viewportSize.set( windowSize.Width - ( windowSize.Width / 6 ), windowSize.Height - 1 );
							loadFonts();
							return true;
						}
						break;
					case USER_EVENT_JOYSTICK_UP: {
							wcout << L"Joystick moved up" << endl;
							movePlayerOnY( 0, -1 );
							return true;
						}
						break;
					case USER_EVENT_JOYSTICK_DOWN: {
							wcout << L"Joystick moved down" << endl;
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
					core::array<s16> verticalAxes;
					verticalAxes.push_back( SEvent::SJoystickEvent::AXIS_Y );

					bool joystickMovedUp = false;
					bool joystickMovedDown = false;
					bool joystickMovedRight = false;
					bool joystickMovedLeft = false;

					//wcout << L"Joystick event" << endl;
					for( u32 i = 0; i < verticalAxes.size(); i++ ) {
						//wcout << L"Joystick event" << endl;
						if( event.JoystickEvent.Axis[i] >= ( 32767 / 2 ) ) {
							wcout << L"Axis value: " << event.JoystickEvent.Axis[i] << endl;
							joystickMovedUp = true;
						} else if( event.JoystickEvent.Axis[i] <= ( -32767 / 2 ) ) {
							wcout << L"Axis value: " << event.JoystickEvent.Axis[i] << endl;
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
						loadFromFile( fileChooser->getFileName() );
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

