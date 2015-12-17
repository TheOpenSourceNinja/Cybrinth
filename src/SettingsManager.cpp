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
						
						{
							AI temp;
							prefsFile << possiblePrefs.at( ALGORITHM ) << L"\t" << temp.stringFromAlgorithm( botAlgorithm ) << std::endl;
						}
						
						prefsFile << possiblePrefs.at( VOLUME ) << L"\t" << musicVolume << std::endl;
						
						prefsFile << possiblePrefs.at( NUMBOTS ) << L"\t" << numBots << std::endl;
						
						prefsFile << possiblePrefs.at( SHOW_BACKGROUNDS ) << L"\t" << boolToWString( showBackgrounds ) << std::endl;
						
						prefsFile << possiblePrefs.at( FULLSCREEN ) << L"\t" << boolToWString( fullscreen ) << std::endl;
						
						prefsFile << possiblePrefs.at( MARK_TRAILS ) << L"\t" << boolToWString( markTrails ) << std::endl;
						
						prefsFile << possiblePrefs.at( DEBUG ) << L"\t" << boolToWString( debug ) << std::endl;
						
						prefsFile << possiblePrefs.at( BPP ) << L"\t" << bitsPerPixel << std::endl;
						
						prefsFile << possiblePrefs.at( VSYNC ) << L"\t" << boolToWString( vsync ) << std::endl;
						
						{
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
							
							prefsFile << std::endl;
						}
						
						prefsFile << possiblePrefs.at( NUMPLAYERS ) << L"\t" << numPlayers << std::endl;
						
						prefsFile << possiblePrefs.at( WINDOW_SIZE ) << L"\t" << windowSize.Width << L"x" << windowSize.Height << std::endl;
						
						prefsFile << possiblePrefs.at( PLAY_MUSIC ) << L"\t" << boolToWString( playMusic ) << std::endl;
						
						prefsFile << possiblePrefs.at( NETWORK_PORT ) << L"\t" << sc.toStdWString( network->getPort() ) << std::endl;
						
						prefsFile << possiblePrefs.at( ALWAYS_SERVER ) << L"\t" << boolToWString( alwaysServer ) << std::endl;
						
						prefsFile << possiblePrefs.at( SOLUTION_KNOWN ) << L"\t" << boolToWString( botsKnowSolution ) << std::endl;
						
						prefsFile << possiblePrefs.at( MOVEMENT_DELAY ) << L"\t" << botMovementDelay << std::endl;
						
						prefsFile << possiblePrefs.at( HIDE_UNSEEN ) << L"\t" << boolToWString( mazeManager->hideUnseen ) << std::endl;
						
						prefsFile << possiblePrefs.at( BACKGROUND_ANIMATIONS ) << L"\t" << boolToWString( backgroundAnimations ) << std::endl;
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

bool SettingsManager::getPlayMusic() {
	return playMusic;
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
	networkPort = 61187;
	alwaysServer = true;
	isServer = alwaysServer;
	botsKnowSolution = false;
	botAlgorithm = AI::DEPTH_FIRST_SEARCH;
	botMovementDelay = 300;
	mazeManager->hideUnseen = false;
	backgroundAnimations = true;
	
	mainGame->musicSettingChanged();
}

void SettingsManager::setPointers( irr::IrrlichtDevice* newDevice, MainGame* newMainGame, MazeManager* newMazeManager, NetworkManager* newNetwork, SpellChecker* newSpellChecker, SystemSpecificsManager* newSystem ) {
	device = newDevice;
	mainGame = newMainGame;
	mazeManager = newMazeManager;
	network = newNetwork;
	spellChecker = newSpellChecker;
	system = newSystem;
}
