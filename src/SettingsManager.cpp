#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/trim_all.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/lexical_cast.hpp>
#include <iostream>
#include <irrlicht/irrlicht.h>
#include <SDL_mixer.h>
#include <vector>

#include "CustomException.h"
#include "SettingsManager.h"

#include "MainGame.h"
#include "MazeManager.h"

SettingsManager::SettingsManager() {
	setPointers( nullptr, nullptr, nullptr, nullptr, nullptr, nullptr );
	minimumWindowSize.Width = 640;
	minimumWindowSize.Height = 480;
	showBackgroundsDefault = true;
	fullscreenDefault = false;
	bitsPerPixelDefault = 16;
	vsyncDefault = true;
	driverTypeDefault = irr::video::EDT_OPENGL;
	windowSizeDefault = minimumWindowSize;
	playMusicDefault = true;
	numBotsDefault = 0;
	numPlayersDefault = 1;
	markTrailsDefault = false;
	musicVolumeDefault = 50;
	networkPortDefault = 61187;
	alwaysServerDefault = true;
	isServerDefault = true;
	botsKnowSolutionDefault = false;
	botAlgorithmDefault = AI::DEPTH_FIRST_SEARCH;
	botMovementDelayDefault = 300;
	hideUnseenDefault = false;
	backgroundAnimationsDefault = true;
	
	alwaysServer = alwaysServerDefault;
}

SettingsManager::~SettingsManager() {
}

/**
 * Created because so many preferences are booleans
 * Arguments:
 * --- std::wstring input: a string which should be either "true" or "false" (misspellings are acceptable)
 * Returns: A Boolean indicating whether input is closer to "true" or to "false"
 */
bool SettingsManager::wStringToBool( std::wstring input ) {
	std::vector< std::wstring > possibleChoices = { L"true", L"false" };
	auto choiceNum = spellChecker->indexOfClosestString( input, possibleChoices );
	return( choiceNum == 0 );
}

/**
 * Created because so many preferences are booleans
 * Arguments:
 * --- bool input: a boolean, of course
 * Returns: L"true" or L"false"
 */
std::wstring SettingsManager::boolToWString( bool input ) {
	if( input ) {
		return L"true";
	} else {
		return L"false";
	}
}

void SettingsManager::setBitsPerPixel( uint_fast8_t newBPP ) {
	if( newBPP < 32 ) {
		bitsPerPixel = 16;
	} else {
		bitsPerPixel = 32;
	}
}

void SettingsManager::setMusicVolume( uint_fast8_t newVolume ) {
	musicVolume = std::min( newVolume, ( decltype( musicVolume ) ) 100 );
	mainGame->musicVolumeChanged();
}

void SettingsManager::setPlayMusic( bool newSetting ) {
	playMusic = newSetting;
	mainGame->musicSettingChanged();
}

void SettingsManager::savePrefs() {
	try {
		std::vector< boost::filesystem::path > configFolders = system->getConfigFolders(); // Flawfinder: ignore
		bool prefsFileFound = false;
		
		for( auto it = configFolders.begin(); it not_eq configFolders.end(); ++it ) {
			boost::filesystem::path prefsPath( *it/L"prefs.cfg" );
			
			if( ( exists( prefsPath ) and not is_directory( prefsPath ) ) or not exists( prefsPath ) ) {
				if( debug ) {
					std::wcout << L"Saving preferences to file " << prefsPath.wstring() << std::endl;
				}
				prefsFileFound = true;
				boost::filesystem::wfstream prefsFile;
				prefsFile.open( prefsPath, boost::filesystem::wfstream::out );
				
				if( prefsFile.is_open() ) {
					{
						StringConverter sc;
						
						//Save header comments------------
						prefsFile << L"//Single-line comments like this are allowed, and must begin with two slashes (//). Anything before the slashes is considered not part of a comment." << std::endl;
						prefsFile << L"//Preference and value must be separated by a tab character, not spaces." << std::endl;
						prefsFile << L"//Any preference not specified here will use its default value. The same goes for things specified in invalid ways (i.e. putting letters where numbers should be)." << std::endl;
						prefsFile << L"//Preferences are not case-sensitive. \"Play music\" is the same as \"PLAY muSic\"." << std::endl;
						prefsFile << L"//Preferences and values do not have to be spelled correctly: \"treu\" will be interepreted as \"true\", \"flse\" as \"false\", etc." << std::endl;
						prefsFile << std::endl;
						
						//Done with header comments-------------
						std::wstring commentMark = L"//";
						std::wstring defaultString = L" " + commentMark + L"Default: ";
						std::wstring line = L"------------------------";
						
						
						{ //Graphics tab
							prefsFile << std::endl << commentMark << L"Graphics" << line << std::endl;
							
							prefsFile << possiblePrefs.at( FULLSCREEN ) << L"\t" << boolToWString( fullscreen ) << defaultString << boolToWString( fullscreenDefault ) << L". Determines whether we try to use full-screen graphics." << std::endl;
							
							prefsFile << possiblePrefs.at( BPP ) << L"\t" << bitsPerPixel << defaultString << bitsPerPixelDefault << L". Determines the color depth when running in fullscreen; will be ignored when not running in fullscreen. Note that on the vast majority of systems, changing this setting will have no visible effect." << std::endl;
							
							prefsFile << possiblePrefs.at( VSYNC ) << L"\t" << boolToWString( vsync ) << defaultString << boolToWString( vsyncDefault ) << L". Set this to false if the game seems slow, but expect graphical 'ripping' of moving objects." << std::endl;
							
							{ //driver type
								prefsFile << possiblePrefs.at( DRIVER_TYPE ) << L"\t";
								switch( driverType ) {
									case irr::video::EDT_OPENGL: {
										prefsFile << driverTypes.at( OPENGL );
										break;
									}
									case irr::video::EDT_DIRECT3D9: {
										prefsFile << driverTypes.at( DIRECT3D9 );
										break;
									}
									case irr::video::EDT_DIRECT3D8: {
										prefsFile << driverTypes.at( DIRECT3D8 );
										break;
									}
									case irr::video::EDT_BURNINGSVIDEO: {
										prefsFile << driverTypes.at( BURNINGS );
										break;
									}
									case irr::video::EDT_SOFTWARE: {
										prefsFile << driverTypes.at( SOFTWARE );
										break;
									}
									case irr::video::EDT_NULL: {
										prefsFile << driverTypes.at( DRIVERNULL );
										break;
									}
								}
								
								prefsFile << defaultString;
								
								switch( driverTypeDefault ) {
									case irr::video::EDT_OPENGL: {
										prefsFile << driverTypes.at( OPENGL );
										break;
									}
									case irr::video::EDT_DIRECT3D9: {
										prefsFile << driverTypes.at( DIRECT3D9 );
										break;
									}
									case irr::video::EDT_DIRECT3D8: {
										prefsFile << driverTypes.at( DIRECT3D8 );
										break;
									}
									case irr::video::EDT_BURNINGSVIDEO: {
										prefsFile << driverTypes.at( BURNINGS );
										break;
									}
									case irr::video::EDT_SOFTWARE: {
										prefsFile << driverTypes.at( SOFTWARE );
										break;
									}
									case irr::video::EDT_NULL: {
										prefsFile << driverTypes.at( DRIVERNULL );
										break;
									}
								}
								
								prefsFile << L". Possible values are OpenGL, Direct3D9, Direct3D8, Burning's Video, Software, and NULL (only for debugging, do not use!). If the selected driver type is not available for your system, the game will automatically choose one that is." << std::endl;
							}
							
							prefsFile << possiblePrefs.at( WINDOW_SIZE ) << L"\t" << windowSize.Width << L"x" << windowSize.Height << defaultString << windowSizeDefault.Width << L"x" << windowSizeDefault.Height << L". Determines how big the game window will be in pixels. The numbers must be positive integers separated by an x. Only applicable if not running in fullscreen. If running in fullscreen, the screen resolution will be detected automatically. Playability is not guaranteed at sizes below the default." << std::endl;
							
							prefsFile << possiblePrefs.at( SHOW_BACKGROUNDS ) << L"\t" << boolToWString( showBackgrounds ) << defaultString << boolToWString( showBackgroundsDefault ) << L". Setting this to false can really speed the game up on slow systems like the Raspberry Pi." << std::endl;
							
							prefsFile << possiblePrefs.at( BACKGROUND_ANIMATIONS ) << L"\t" << boolToWString( backgroundAnimations ) << defaultString << boolToWString( backgroundAnimationsDefault ) << L". If set to false, only non-animated backgrounds will be shown." << std::endl;
							
							prefsFile << possiblePrefs.at( MARK_TRAILS ) << L"\t" << boolToWString( markTrails ) << defaultString << boolToWString( markTrailsDefault ) << L". Makes solving the maze easier by marking where you've already been." << std::endl;
							
						}
						
						{ //Sound tab
							prefsFile << std::endl << commentMark << L"Sound" << line << std::endl;
							
							prefsFile << possiblePrefs.at( PLAY_MUSIC ) << L"\t" << boolToWString( playMusic ) << defaultString << boolToWString( playMusicDefault ) << L". If set to true, the game will search for music files in the ./music folder and attempt to play them. Supported music formats may vary from system to system, but generally will include WAVE (.wav), MOD (.mod, .xm, .s3m, .669, .it, or .med), MIDI (.mid), OGG Vorbis (.ogg), MP3 (.mp3), and FLAC (.flac)." << std::endl;
							
							prefsFile << possiblePrefs.at( VOLUME ) << L"\t" << musicVolume << defaultString << musicVolumeDefault << L". Sets the music volume. Must be an integer between 0 and 100. The volume can be adjusted as the game is playing." << std::endl;
							
						}
						
						
						{ //Multiplayer tab
							prefsFile << std::endl << commentMark << L"Multiplayer" << line << std::endl;
							
							prefsFile << possiblePrefs.at( NUMPLAYERS ) << L"\t" << numPlayers << defaultString << numPlayersDefault<< L". This represents the total number of bots and human players. It must be an integer greater than or equal to the number of AI bots and less than or equal to 255. The game supports up to 4 human players on one keyboard, plus any number using other control devices. Online play to come soon." << std::endl;
							
							prefsFile << possiblePrefs.at( NUMBOTS ) << L"\t" << numBots << defaultString << numBotsDefault << L". Sets the number of AI bots you play against. Must be an integer less than or equal to the number of players." << std::endl;
							
							prefsFile << possiblePrefs.at( NETWORK_PORT ) << L"\t" << networkPort << defaultString << networkPortDefault << L". This controls which port the server listens for connections on and the clients will attempt to connect to. Ports below 1024 may not work if you're on a Unix-like system and don't have superuser privileges, see https://en.wikipedia.org/w/index.php?title=List_of_TCP_and_UDP_port_numbers&oldid=501310028#Well-known_ports" << std::endl;
							
							prefsFile << possiblePrefs.at( ALWAYS_SERVER ) << L"\t" << boolToWString( alwaysServer ) << defaultString << boolToWString( alwaysServerDefault ) << L" for now. Sets whether this copy of the program will always assume itself to be a server or ask." << std::endl;
							
							prefsFile << possiblePrefs.at( MOVEMENT_DELAY ) << L"\t" << botMovementDelay << defaultString << botMovementDelayDefault << L". The minimum amount of time in milliseconds that all bots will wait between moves. The actual waiting time depends on your computer's processor speed and clock precision. Must be an integer between 0 and 65,535." << std::endl;
							
							{
								AI temp;
								prefsFile << possiblePrefs.at( ALGORITHM ) << L"\t" << temp.stringFromAlgorithm( botAlgorithm ) << defaultString << temp.stringFromAlgorithm( botAlgorithmDefault ) << L". Controls which algorithm bots use to solve the maze. Possible values are Depth-First Search (will always find a way to a key/goal, not necessarily the nearest key/goal), Iterative Deepening Depth-First Search (will always find the nearest key/goal, but is really slow. May cause the game to freeze for short periods of time. Not recommended for slow computers!), Left Hand Rule and Right Hand Rule (inefficient), and Dijkstra (experimental!)." << std::endl;
							}
							
							prefsFile << possiblePrefs.at( SOLUTION_KNOWN ) << L"\t" << boolToWString( botsKnowSolution ) << defaultString << boolToWString( botsKnowSolutionDefault ) << L". Whether the bots know the solution or have to find it as they play. Note that they do not necessarily know the *best* solution, just one that works." << std::endl;
							
						}
						
						{ //Miscellaneous tab
							prefsFile << std::endl << commentMark << L"Miscellaneous" << line << std::endl;
							
							prefsFile << possiblePrefs.at( DEBUG ) << L"\t" << boolToWString( debug ) << defaultString << boolToWString( debugDefault ) << L". Makes the program output more text to standard output. Also makes the AIs insanely fast." << std::endl;
							
							prefsFile << possiblePrefs.at( HIDE_UNSEEN ) << L"\t" << boolToWString( mazeManager->hideUnseen ) << defaultString << boolToWString( hideUnseenDefault ) << L". Hides parts of the maze that no player has seen yet (seen means unobstructed line-of-sight from any player's position)" << std::endl;
							
						}
					}
					
					prefsFile.close();
				}
			}
		}
	} catch( std::exception &e ) {
		std::wcerr << L"Error in MainGame::savePrefs(): " << e.what() << std::endl;
	}
	
	if( debug ) {
		std::wcout << L"end of savePrefs()" << std::endl;
	}
}

uint_fast8_t SettingsManager::getBitsPerPixel() {
	return bitsPerPixel;
}

irr::core::dimension2d< irr::u32 > SettingsManager::getMinimumWindowSize() {
	return minimumWindowSize;
}

uint_fast8_t SettingsManager::getMusicVolume() {
	return musicVolume;
}

bool SettingsManager::getPlayMusic() {
	return playMusic;
}

irr::core::dimension2d< irr::u32 > SettingsManager::getWindowSize() {
	return windowSize;
}

/**
 * Reads preferences from prefs.cfg. Sets defaults for any preference not found in the file. If the file does not exist, creates it.
 */
void SettingsManager::readPrefs() {
	try {
		#if defined DEBUG
			debug = true;
		#else
			debug = false;
		#endif
		if( debug ) {
			std::wcout << L"readPrefs() called" << std::endl;
		}

		//Set default prefs, in case we can't get them from the file
		resetToDefaults();
		
		auto prefsNotFound = possiblePrefs;
		
		std::vector< boost::filesystem::path > configFolders = system->getConfigFolders(); // Flawfinder: ignore
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
								auto preferenceNum = spellChecker->indexOfClosestString( preference, possiblePrefs );
								
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
									case ALGORITHM: { //L"bots' solving algorithm"
										AI temp;
										temp.setup( mainGame, false, AI::ALGORITHM_DO_NOT_USE, 0 );
										botAlgorithm = temp.algorithmFromString( choice );
										break;
									}
									
									case VOLUME: { //L"volume"
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
									
									case NUMBOTS: { //L"number of bots"
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
									
									case SHOW_BACKGROUNDS: { //L"show backgrounds"
										showBackgrounds = wStringToBool( choice );
										break;
									}
									
									case FULLSCREEN: { //L"fullscreen"
										fullscreen = wStringToBool( choice );
										break;
									}
									
									case MARK_TRAILS: { //L"mark player trails"
										markTrails = wStringToBool( choice );
										break;
									}
									
									case DEBUG: { //L"debug"
										#ifndef DEBUG
											debug = wStringToBool( choice );
										#endif
										
										if( debug ) {
											std::wcout << L"Debug is ON" << std::endl;
										}
										break;
									}
									
									case BPP: { //L"bits per pixel"
										try {
											uint_fast8_t choiceAsInt = boost::lexical_cast< uint_fast16_t >( choice );
											setBitsPerPixel( choiceAsInt );
											
										} catch( boost::bad_lexical_cast &e ) {
											std::wcerr << L"Error reading bitsPerPixel preference (is it not a number?) on line " << lineNum << L": " << e.what() << std::endl;
										}
										break;
									}
									
									case VSYNC: { //L"wait for vertical sync"
										vsync = wStringToBool( choice );
										break;
									}
									
									case DRIVER_TYPE: { //L"driver type"
										std::vector< std::wstring > possibleChoices = driverTypes;
										choice = possibleChoices.at( spellChecker->indexOfClosestString( choice, possibleChoices ) );
										
										if( choice == possibleChoices.at( OPENGL ) ) { //L"opengl"
											driverType = irr::video::EDT_OPENGL;
										} else if( choice == possibleChoices.at( DIRECT3D9 ) ) { //L"direct3d9"
											driverType = irr::video::EDT_DIRECT3D9;
										} else if( choice == possibleChoices.at( DIRECT3D8 ) ) { //L"direct3d8"
											driverType = irr::video::EDT_DIRECT3D8;
										} else if( choice == possibleChoices.at( BURNINGS ) ) { //L"burning's video"
											driverType = irr::video::EDT_BURNINGSVIDEO;
										} else if( choice == possibleChoices.at( SOFTWARE ) ) { //L"software"
											driverType = irr::video::EDT_SOFTWARE;
										} else if( choice == possibleChoices.at( DRIVERNULL ) ) { //L"null"
											driverType = irr::video::EDT_NULL;
										}
										
										if( debug ) {
											std::wcout << L"Selected driver type is " << choice << std::endl;
										}
										
										break;
									}
									
									case NUMPLAYERS: { //L"number of players"
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
									
									case WINDOW_SIZE: { //L"window size"
										try {
											size_t locationOfX = choice.find( L"x" );
											std::wstring width = choice.substr( 0, locationOfX );
											std::wstring height = choice.substr( locationOfX + 1 );
											if( debug ) {
												std::wcout << L"Window size: " << width << L"x" << height << std::endl;
											}
											
											decltype( windowSize.Width ) widthAsInt = boost::lexical_cast< decltype( windowSize.Width ) >( width );
											decltype( windowSize.Height ) heightAsInt = boost::lexical_cast< decltype( windowSize.Height ) >( height );
											
											setWindowSize( irr::core::dimension2d< decltype( windowSize.Height ) >( widthAsInt, heightAsInt ) ); 
											
											/*if( widthAsInt < 160 or heightAsInt < 240 ) {
												std::wcerr << L"Error reading window size: Width and/or height are really really tiny. Sorry but you'll have to recompile the game yourself if you want a window that small." << std::endl;
											} else if( widthAsInt == 160 and heightAsInt == 240 ) {
												std::wcout << L"Rock on, CGA graphics. Rock on." << std::endl;
												windowSize = irr::core::dimension2d< decltype( windowSize.Height ) >( widthAsInt, heightAsInt );
											} else {
												windowSize = irr::core::dimension2d< decltype( windowSize.Height ) >( widthAsInt, heightAsInt );
											}*/
										} catch( boost::bad_lexical_cast e ) {
											std::wcerr << L"Error reading window size. It must be composed of two integers separated by an x, e.g. 640x480." << std::endl;
										}
										break;
									}
									
									case PLAY_MUSIC: { //L"play music"
										setPlayMusic( wStringToBool( choice ) );
										break;
									}
									
									case NETWORK_PORT: { //L"network port"
										if( debug ) {
											std::wcout << L"Network port: " << choice << std::endl;
										}
										
										try {
											//StringConverter sc;
											//network->setPort( sc.toStdString( choice ) );
											networkPort = boost::lexical_cast< decltype( networkPort ) >( choice );
										} catch( boost::bad_lexical_cast &e ) {
											std::wcerr << L"Error reading network port (is it not a number?) on line " << lineNum << L": " << e.what() << std::endl;
										}
										break;
									}
									
									case ALWAYS_SERVER: { //L"always server"
										alwaysServer = wStringToBool( choice );
										isServer = alwaysServer;
										break;
									}
									
									case SOLUTION_KNOWN: { //L"bots know the solution"
										botsKnowSolution = wStringToBool( choice );
										break;
									}
									
									case MOVEMENT_DELAY: { //L"bot movement delay"
										try {
											botMovementDelay = boost::lexical_cast< uint_fast16_t >( choice );
										} catch( boost::bad_lexical_cast &e ) {
											std::wcerr << L"Error reading botMovementDelay preference (is it not a number?) on line " << lineNum << L": " << e.what() << std::endl;
										}
										break;
									}
									
									case HIDE_UNSEEN: { //L"hide unseen maze areas"
										mazeManager->hideUnseen = wStringToBool( choice );
										break;
									}
									
									case BACKGROUND_ANIMATIONS: { //L"ackground animations"
										backgroundAnimations = wStringToBool( choice );
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
					
					//Note: Just because the library supports a driver type doesn't mean we can actually use it. A loop similar to the above is used in the MainGame constructor where we call createDevice(). Therefore, the final driverType may not be what is set here.
					if( driverType == irr::video::EDT_NULL ) {
						std::wcerr << L"Error: No graphical output driver types are available. Using NULL type!! Also enabling debug." << std::endl;
						debug = true;
					}
				}
			}
		}

		if( not isServer ) {
			std::wcout << L"(S)erver or (c)lient? " << std::endl;
			wchar_t a;
			std::wcin >> a;
			isServer = ( a == L's' or a == L'S' );
		}
		
		if( not prefsFileFound ) {
			throw( CustomException( L"prefs.cfg does not exist or is not readable in any of the folders that were searched." ) );
		}
	} catch( std::exception &e ) {
		std::wcerr << L"Error in MainGame::readPrefs(): " << e.what() << std::endl;
	}
	
	if( debug ) {
		std::wcout << L"end of readPrefs()" << std::endl;
	}
}


/**
 * @brief Sets default settings.
 * Moved this code out of readPrefs() and into a separate function so that settingsScreen can use it too.
 */
void SettingsManager::resetToDefaults() {
	auto oldPlayMusic = playMusic; //This needs to be known at the end of this function
	
	showBackgrounds = showBackgroundsDefault;
	fullscreen = fullscreenDefault;
	bitsPerPixel = bitsPerPixelDefault;
	vsync = vsyncDefault;
	driverType = driverTypeDefault;
	windowSize = windowSizeDefault;
	playMusic = playMusicDefault;
	numBots = numBotsDefault;
	numPlayers = numPlayersDefault;
	markTrails = markTrailsDefault;
	musicVolume = musicVolumeDefault;
	networkPort = networkPortDefault;
	alwaysServer = alwaysServerDefault;
	isServer = isServerDefault;
	botsKnowSolution = botsKnowSolutionDefault;
	botAlgorithm = botAlgorithmDefault;
	botMovementDelay = botMovementDelayDefault;
	mazeManager->hideUnseen = hideUnseenDefault;
	backgroundAnimations = backgroundAnimationsDefault;
	
	if( oldPlayMusic not_eq playMusic ) {
		mainGame->musicSettingChanged();
	}
}

void SettingsManager::setPointers( irr::IrrlichtDevice* newDevice, MainGame* newMainGame, MazeManager* newMazeManager, NetworkManager* newNetwork, SpellChecker* newSpellChecker, SystemSpecificsManager* newSystem ) {
	device = newDevice;
	mainGame = newMainGame;
	mazeManager = newMazeManager;
	network = newNetwork;
	spellChecker = newSpellChecker;
	system = newSystem;
}

void SettingsManager::setWindowSize( irr::core::dimension2d< irr::u32 > newSize ) {
	if( !allowSmallSize && ( newSize.Width < minimumWindowSize.Width or newSize.Height < minimumWindowSize.Height ) ) {
		std::wcerr << L"Error reading window size: Width and/or height are really really tiny. Sorry but you'll have to recompile the game yourself if you want a window that small." << std::endl;
	} else if( newSize.Width == 160 and newSize.Height == 240 ) {
		std::wcout << L"Rock on, CGA graphics. Rock on." << std::endl;
		windowSize = irr::core::dimension2d< decltype( windowSize.Height ) >( newSize.Width, newSize.Height );
	} else {
		windowSize = irr::core::dimension2d< decltype( windowSize.Height ) >( newSize.Width, newSize.Height );
	}
}