#include "colors.h"
#include "MazeManager.h"
#include "GameManager.h"

#define BOOST_FILESYSTEM_NO_DEPRECATED //Recommended by the Boost filesystem library documentation to prevent us from using functions which will be removed in later versions
#include <filesystem/fstream.hpp>
#include <iostream>


//Recursively searches the maze to see if it can get from start to end
bool MazeManager::canGetTo( uint_least8_t startX, uint_least8_t startY, uint_least8_t goalX, uint_least8_t goalY ) {
	try {
		bool found = false;

		maze[ startX ][ startY ].visited = true;

		if( startX == goalX && startY == goalY ) {
			found = true;
		} else {
			if( startY > 0 && maze[ startX ][ startY ].getTop() == MazeCell::NONE && maze[ startX ][ startY - 1 ].visited == false ) {
				found = canGetTo( startX, startY - 1, goalX, goalY );
			}

			if( found == false && startY < ( rows - 1 ) && maze[ startX ][ startY + 1 ].getTop() == MazeCell::NONE && maze[ startX ][ startY + 1 ].visited == false ) {
				found = canGetTo( startX, startY + 1, goalX, goalY );
			}

			if( found == false && startX < ( cols - 1 ) && maze[ startX + 1 ][ startY ].getLeft() == MazeCell::NONE && maze[ startX + 1 ][ startY ].visited == false ) {
				found = canGetTo( startX + 1, startY, goalX, goalY );
			}

			if( found == false && startX > 0 && maze[ startX ][ startY ].getLeft() == MazeCell::NONE && maze[ startX - 1 ][ startY ].visited == false ) {
				found = canGetTo( startX - 1, startY, goalX, goalY );
			}
		}

		return found;
	} catch ( std::exception e ) {
		std::wcerr << L"Error in MazeManager::canGetTo(): " << e.what() << std::endl;
		return false;
	}
}

//Iterates through all collectables, calls canGetTo on their locations
bool MazeManager::canGetToAllCollectables( uint_least8_t startX, uint_least8_t startY ) {
	try {
		bool result = true;

		for( uint_fast16_t i = 0; ( i < gameManager->stuff.size() && result == true ); ++i ) {

			//Do this here because canGetTo() uses the visited variable
			for( uint_fast8_t x = 0; x < cols; ++x ) {
				for( uint_fast8_t y = 0; y < rows; ++y ) {
					maze[ x ][ y ].visited = false;
				}
			}

			bool otherResult = canGetTo( startX, startY, gameManager->stuff[ i ].getX(), gameManager->stuff[ i ].getY() );

			if( otherResult == false ) {
				result = false;
			}
		}

		return result;
	} catch ( std::exception e ) {
		std::wcerr << L"Error in MazeManager::canGetToAllCollectables(): " << e.what() << std::endl;
		return false;
	}
}

void MazeManager::draw( video::IVideoDriver* driver, uint_least16_t cellWidth, uint_least16_t cellHeight ) {
	try {
		video::SColor wallColor = WHITE;
		video::SColor lockColor = BROWN;
		video::SColor wallShadowColor = BLACK;
		video::SColor lockShadowColor = BLACK;
		core::position2d< s32 > shadowOffset( 1, 1 );

		for( uint_fast8_t x = 0; x < cols; ++x ) {
			for( uint_fast8_t y = 0; y < rows; ++y ) {
				if( maze[ x ][ y ].visible ) {
					if( maze[ x ][ y ].getTop() == MazeCell::WALL ) {
						driver->draw2DLine( shadowOffset + core::position2d< s32 >( cellWidth * x, cellHeight * y ), shadowOffset + core::position2d< s32 >(cellWidth * ( x + 1 ), cellHeight * y ), wallShadowColor );
						driver->draw2DLine( core::position2d< s32 >( cellWidth * x, cellHeight * y ), core::position2d< s32 >(cellWidth * ( x + 1 ), cellHeight * y ), wallColor );
					} else if( maze[ x ][ y ].getTop() == MazeCell::LOCK ) {
						driver->draw2DLine( shadowOffset + core::position2d< s32 >( cellWidth * x, cellHeight * y ), shadowOffset + core::position2d< s32 >(cellWidth * ( x + 1 ), cellHeight * y ), lockShadowColor );
						driver->draw2DLine( core::position2d< s32 >( cellWidth * x, cellHeight * y ), core::position2d< s32 >(cellWidth * ( x + 1 ), cellHeight * y ), lockColor );
					}

					if( maze[ x ][ y ].getLeft() == MazeCell::WALL ) {
						driver->draw2DLine( shadowOffset + core::position2d< s32 >( cellWidth * x, cellHeight * y ), shadowOffset + core::position2d< s32 >( cellWidth * x, cellHeight * ( y + 1 ) ), wallShadowColor );
						driver->draw2DLine( core::position2d< s32 >( cellWidth * x, cellHeight * y ), core::position2d< s32 >( cellWidth * x, cellHeight * ( y + 1 ) ), wallColor );
					} else if( maze[ x ][ y ].getLeft() == MazeCell::LOCK ) {
						driver->draw2DLine( shadowOffset + core::position2d< s32 >( cellWidth * x, cellHeight * y ), shadowOffset + core::position2d< s32 >( cellWidth * x, cellHeight * ( y + 1 ) ), lockShadowColor );
						driver->draw2DLine( core::position2d< s32 >( cellWidth * x, cellHeight * y ), core::position2d< s32 >( cellWidth * x, cellHeight * ( y + 1 ) ), lockColor );
					}

					//Only cells on the right or bottom edge of the maze should have anything other than NONE as right or bottom, and then it should only be a solid WALL
					if( maze[ x ][ y ].getRight() == MazeCell::WALL ) {
						driver->draw2DLine( shadowOffset + core::position2d< s32 >( cellWidth * ( x + 1 ), cellHeight * y ), shadowOffset + core::position2d< s32 >( cellWidth * ( x + 1 ), cellHeight * ( y + 1 ) ), wallShadowColor );
						driver->draw2DLine( core::position2d< s32 >( cellWidth * ( x + 1 ), cellHeight * y ), core::position2d< s32 >( cellWidth * ( x + 1 ), cellHeight * ( y + 1 ) ), wallColor );
					}

					if( maze[ x ][ y ].getBottom() == MazeCell::WALL ) {
						driver->draw2DLine( shadowOffset + core::position2d< s32 >( cellWidth * x, cellHeight * ( y + 1 ) ), shadowOffset + core::position2d< s32 >( cellWidth * ( x + 1 ), cellHeight * ( y + 1 ) ), wallShadowColor );
						driver->draw2DLine( core::position2d< s32 >( cellWidth * x, cellHeight * ( y + 1 ) ), core::position2d< s32 >( cellWidth * ( x + 1 ), cellHeight * ( y + 1 ) ), wallColor );
					}
				}
			}
		}
	} catch ( std::exception e ) {
		std::wcerr << L"Error in MazeManager::draw(): " << e.what() << std::endl;
	}
}

bool MazeManager::existsAnythingAt( uint_least8_t x, uint_least8_t y ) {
	try {
		bool result = false;

		if( gameManager->goal.getX() == x && gameManager->goal.getY() == y ) {
			result = true;
		}

		for( uint_fast16_t i = 0; ( result == false && i < gameManager->stuff.size() ); ++i ) {
			if( gameManager->stuff[ i ].getX() == x && gameManager->stuff[ i ].getY() == y ) {
				result = true;
			}
		}

		return result;
	} catch ( std::exception e ) {
		std::wcerr << L"Error in MazeManager::existsAnythingAt(): " << e.what() << std::endl;
		return false;
	}
}

bool MazeManager::loadFromFile() {
	try {
		return loadFromFile( L"default.maz" );
	} catch ( std::exception e ) {
		std::wcerr << L"Error in MazeManager::loadFromFile(): " << e.what() << std::endl;
		return false;
	}
}

bool MazeManager::loadFromFile( boost::filesystem::path src ) {
	try {
		if( !exists( src ) ) {
			throw( std::wstring( L"File not found: " ) + src.wstring() );
		} else if( is_directory( src ) ) {
			throw( std::wstring( L"Directory specified, file needed: " ) + src.wstring() );
		}

		boost::filesystem::wifstream file; //Identical to a standard C++ fstream, except it takes Boost paths
		file.open( src );

		if( file.is_open() ) {
			gameManager->resetThings();
			file >> gameManager->randomSeed;
			file.close();
			srand( gameManager->randomSeed );
			makeRandomLevel();
			return true;
		} else {
			throw( std::wstring( L"Cannot open file: " ) + src.wstring() );
		}
	} catch( const boost::filesystem::filesystem_error e ) {
		std::wcerr << L"Boost Filesystem error in MazeManager::loadFromFile: " << e.what() << std::endl;
		return false;
	} catch( std::exception e ) {
		std::wcerr << L"non-Boost-Filesystem error in MazeManager::loadFromFile: " << e.what() << std::endl;
		return false;
	} catch( std::wstring e ) {
		std::wcerr << L"non-Boost-Filesystem error in MazeManager::loadFromFile: " << e << std::endl;
		return false;
	}
}

//Does everything involved in making the maze, calls other functions as needed.
void MazeManager::makeRandomLevel() {
	try {
		gameManager->drawAll();

		srand( gameManager->randomSeed ); //randomSeed is set either by resetThings() or by loadFromFile()
		uint_least8_t tempCols = rand() % 28 + 2;
		uint_least8_t tempRows = tempCols + ( rand() % 5 );
		resizeMaze( tempCols, tempRows );

		for( uint_fast8_t x = 0; x < cols; ++x ) {
			for( uint_fast8_t y = 0; y < rows; ++y ) {
				maze[ x ][ y ].setTop( MazeCell::WALL );
				maze[ x ][ y ].setLeft( MazeCell::WALL );
				maze[ x ][ y ].setRight( MazeCell::NONE );
				maze[ x ][ y ].setBottom( MazeCell::NONE );
				maze[ x ][ y ].visited = false;
			}
		}

		for( uint_fast8_t p = 0; p < gameManager->numPlayers; ++p ) {
			gameManager->playerStart[ p ].reset();
		}

		//do {
		//player.setX(rand() % cols);
		//player.setY(rand() % rows);
		gameManager->goal.setX( rand() % cols );
		gameManager->goal.setY( rand() % rows );
		//} while (player.getX() == gameManager->goal.getX() && player.getY() == gameManager->goal.getY());

		gameManager->numLocks = ( rand() % 10 ) % cols;
		//numLocks = rand() % ( cols * rows ); //Uncomment this for a crazy number of keys!

		recurseRandom( gameManager->goal.getX(), gameManager->goal.getY(), 0, 0 ); //Start recursion from the gameManager->goal; for some reason that makes the mazes harder than if we started recursion from the player's starting point.

		//Add walls at maze borders
		for( uint_fast8_t x = 0; x < cols; ++x ) {
			maze[ x ][ 0 ].setTop( MazeCell::WALL );
			maze[ x ][ rows-1 ].setBottom( MazeCell::WALL );
		}

		for( uint_fast8_t y = 0; y < rows; ++y ) {
			maze[ 0 ][ y ].setLeft( MazeCell::WALL );
			maze[ cols-1 ][ y ].setRight( MazeCell::WALL );
		}

		for( uint_fast8_t x = 1; x < cols; ++x ) {
			for( uint_fast8_t y = 0; y < rows; ++y ) {
				maze[ x-1 ][ y ].setRight( maze[ x ][ y ].getLeft() );
			}
		}

		for( uint_fast8_t x = 0; x < cols; ++x ) {
			for( uint_fast8_t y = 1; y < rows; ++y ) {
				maze[ x ][ y-1 ].setBottom( maze[ x ][ y ].getTop() );
			}
		}

		//Find all dead ends. I'm sure it would be more efficient to do this during maze generation rather than going back through afterward, but I can't be bothered with that now.
		std::vector<uint_least8_t> deadEndsX;
		std::vector<uint_least8_t> deadEndsY;

		for( uint_fast8_t x = 0; x < cols; ++x ) {
			for( uint_fast8_t y = 0; y < rows; ++y ) {
				if( maze[ x ][ y ].isDeadEnd() ) {
					deadEndsX.push_back( x );
					deadEndsY.push_back( y );
				}
			}
		}

		//Remove player starts from list of dead ends
		for( uint_fast8_t p = 0; p < gameManager->numPlayers; ++p ) {
			for( uint_fast16_t i = 0; i < deadEndsX.size(); ++i ) {
				if( gameManager->playerStart.at( p ).getX() == deadEndsX.at( i ) && gameManager->playerStart.at( p ).getY() == deadEndsY.at( i ) ) {
					deadEndsX.erase( deadEndsX.begin() + i );
					deadEndsY.erase( deadEndsY.begin() + i );
				}
			}
		}

		//Remove gameManager->goal from list of dead ends
		for( uint_fast16_t i = 0; i < deadEndsX.size(); ++i ) {
			if( gameManager->goal.getX() == deadEndsX.at( i ) && gameManager->goal.getY() == deadEndsY.at( i ) ) {
				deadEndsX.erase( deadEndsX.begin() + i );
				deadEndsY.erase( deadEndsY.begin() + i );
			}
		}

		if( gameManager->numLocks > deadEndsX.size() ) {
			gameManager->numLocks = deadEndsX.size();
		}

		uint_least8_t numKeys = gameManager->numLocks;

		//Place keys in dead ends
		//vector<uint_least8_t> keyPlaceX;
		//vector<uint_least8_t> keyPlaceY;

		for( uint_fast8_t k = 0; k < numKeys; ++k ) {
			//vector<uint_least8_t> chosenPlaces;

			if( deadEndsX.size() == 0 ) {
				deadEndsX.push_back( gameManager->playerStart.at( 0 ).getX() );
				deadEndsY.push_back( gameManager->playerStart.at( 0 ).getY() );
			}
			//Pick one of the dead ends randomly.
			uint_least8_t chosen = rand() % deadEndsX.size();

			//Finally, create a key and put it there.
			Collectable temp;
			temp.setX( deadEndsX.at( chosen ) );
			temp.setY( deadEndsY.at( chosen ) );
			temp.setType( Collectable::KEY );
			temp.loadTexture( gameManager->driver );
			gameManager->stuff.push_back( temp );

			//Remove chosen from the list of dead ends so no other keys go there
			deadEndsX.erase( deadEndsX.begin() + chosen );
			deadEndsY.erase( deadEndsY.begin() + chosen );
		}

		for( uint_fast8_t p = 0; p < gameManager->numPlayers; ++p ) {
			gameManager->player[ p ].setPos( gameManager->playerStart[ p ].getX(), gameManager->playerStart[ p ].getY() );
		}

		if( gameManager->numLocks > 0 ) {
			//Place locks
			//Place first lock at the gameManager->goal
			if( maze[ gameManager->goal.getX() ][ gameManager->goal.getY() ].getTop() == MazeCell::NONE ) {
				maze[ gameManager->goal.getX() ][ gameManager->goal.getY() ].setTop( MazeCell::LOCK );
			} else if( maze[ gameManager->goal.getX() ][ gameManager->goal.getY() ].getLeft() == MazeCell::NONE ) {
				maze[ gameManager->goal.getX() ][ gameManager->goal.getY() ].setLeft( MazeCell::LOCK );
			} else if( maze[ gameManager->goal.getX() ][ gameManager->goal.getY() + 1 ].getTop() == MazeCell::NONE ) {
				maze[ gameManager->goal.getX() ][ gameManager->goal.getY() + 1 ].setTop( MazeCell::LOCK );
			} else if( maze[ gameManager->goal.getX() + 1 ][ gameManager->goal.getY() ].getLeft() == MazeCell::NONE ) {
				maze[ gameManager->goal.getX() + 1 ][ gameManager->goal.getY() ].setLeft( MazeCell::LOCK );
			}

			uint_fast8_t numLocksPlaced = 1;

			while( gameManager->device->run() != false && numLocksPlaced < gameManager->numLocks && gameManager->timer->getTime() < gameManager->timeStartedLoading + gameManager->loadingDelay ) {
				uint_least8_t tempX = rand() % cols;
				uint_least8_t tempY = rand() % rows;

				if( maze[ tempX ][ tempY ].getTop() == MazeCell::NONE ) {
					maze[ tempX ][ tempY ].setTop( MazeCell::LOCK );

					if( canGetToAllCollectables( gameManager->playerStart[ 0 ].getX(), gameManager->playerStart[ 0 ].getY() ) ) {
						numLocksPlaced += 1;
						if( gameManager->getDebugStatus() ) {
							std::wcout << L"Placed lock " << numLocksPlaced << L" at " << tempX << L"x" << tempY << std::endl;
						}
					} else {
						maze[ tempX ][ tempY ].setTop( MazeCell::NONE );
					}
				} else if( maze[ tempX ][ tempY ].getLeft() == MazeCell::NONE ) {
					maze[ tempX ][ tempY ].setLeft( MazeCell::LOCK );

					if( canGetToAllCollectables( gameManager->playerStart[ 0 ].getX(), gameManager->playerStart[ 0 ].getY() ) ) {
						numLocksPlaced += 1;
						if( gameManager->getDebugStatus() ) {
							std::wcout << L"Placed lock " << numLocksPlaced << L" at " << tempX << L"x" << tempY << std::endl;
						}
					} else {
						maze[ tempX ][ tempY ].setLeft( MazeCell::NONE );
					}
				}
			}

			gameManager->timer->stop();
			gameManager->timer->setTime( 0 );
			if( gameManager->getDebugStatus() ) {
				std::wcout << L"numLocksPlaced: " << numLocksPlaced << L"\tnumLocks: " << gameManager->numLocks << std::endl;
			}

			if( numLocksPlaced < gameManager->numLocks ) {
				uint_fast8_t keysToRemove = gameManager->numLocks - numLocksPlaced;

				for( uint_fast16_t i = 0; ( i < gameManager->stuff.size() && keysToRemove > 0 ); ++i ) {
					if( gameManager->getDebugStatus() ) {
						std::wcout << L"keysToRemove: " << keysToRemove << std::endl;
					}

					if( gameManager->stuff.at( i ).getType() == Collectable::KEY ) {
						gameManager->stuff.erase( gameManager->stuff.begin() + i );
						i = 0;
						keysToRemove -= 1;
					}
				}
			}

			numKeys = gameManager->numLocks = numLocksPlaced;
		}

		for( uint_fast8_t x = 0; x < cols; ++x ) {
			for( uint_fast8_t y = 0; y < rows; ++y ) {
				maze[ x ][ y ].visited = false;
			}
		}

		for( uint_fast8_t p = 0; p < gameManager->numPlayers; ++p ) {
			maze[ gameManager->playerStart[ p ].getX() ][ gameManager->playerStart[ p ].getY() ].visited = true;
			maze[ gameManager->playerStart[ p ].getX() ][ gameManager->playerStart[ p ].getY() ].setVisitorColor( gameManager->player[ p ].getColorTwo() );
		}

		//Set up bots; Not necessary here since the GameManager does this too
		/*if( gameManager->numBots > 0 ) {
			for( uint_fast8_t i = 0; i < gameManager->numBots; ++i ) {
				gameManager->bot[ i ].setup( maze, cols, rows, gameManager );
			}
		}*/
	} catch ( std::exception e ) {
		std::wcerr << L"Error in MazeManager::makeRandomLevel(): " << e.what() << std::endl;
	}
}

MazeManager::MazeManager() {
	try {
		//resizeMaze() will set cols and rows to whatever gets passed into it; we're making them zero here only so that resizeMaze() doesn't try to copy from the nonexistent previous maze.
		cols = 0;
		rows = 0;
	} catch ( std::exception e ) {
		std::wcerr << L"Error in MazeManager::MazeManager(): " << e.what() << std::endl;
	}
}

MazeManager::~MazeManager() {
	try {
		for( uint_fast8_t i = 0 ; i < cols ; ++i ) {
			delete [ ] maze[ i ];
		}

		delete [ ] maze ;
	} catch ( std::exception e ) {
		std::wcerr << L"Error in MazeManager::~MazeManager(): " << e.what() << std::endl;
	}
}

//Generates the maze recursively
void MazeManager::recurseRandom( uint_least8_t x, uint_least8_t y, uint_least16_t depth, uint_least16_t numSoFar ) {
	try {
		gameManager->device->run();
		gameManager->drawAll();

		maze[ x ][ y ].visited = true;
		maze[ x ][ y ].id = numSoFar;

		for( uint_fast8_t p = 0; p < gameManager->numPlayers; ++p ) {
			if( depth >= gameManager->playerStart[ p ].distanceFromExit ) {
				gameManager->playerStart[ p ].setPos( x, y );
				gameManager->playerStart[ p ].distanceFromExit = depth;
			}
		}

		bool keepGoing = true;

		while( keepGoing ) {
			numSoFar += 1;

			switch( rand() % 4 ) { //4 = number of directions (up, down, left, right)
				case 0: //Left

					if( x > 0 && maze[ x-1 ][ y ].visited == false ) {
						maze[ x ][ y ].setLeft( MazeCell::NONE );

						recurseRandom( x - 1, y, depth + 1, numSoFar );
					}

					break;

				case 1: //Right

					if( x < cols - 1 && maze[ x+1 ][ y ].visited == false ) {
						maze[ x+1 ][ y ].setLeft( MazeCell::NONE );

						recurseRandom( x + 1, y, depth + 1, numSoFar );
					}

					break;

				case 2: //Up

					if( y > 0 && maze[ x ][ y-1 ].visited == false ) {
						maze[ x ][ y ].setTop( MazeCell::NONE );

						recurseRandom( x, y - 1, depth + 1, numSoFar );
					}

					break;

				case 3: //Down

					if( y < rows - 1 && maze[ x ][ y+1 ].visited == false ) {
						maze[ x ][ y+1 ].setTop( MazeCell::NONE );

						recurseRandom( x, y + 1, depth + 1, numSoFar );
					}

					break;
			}

			//If we've reached a dead end, don't keep going. Otherwise do.
			keepGoing = false;
			if(( x > 0 && maze[ x-1 ][ y ].visited == false )
					|| ( x < cols - 1 && maze[ x+1 ][ y ].visited == false )
					|| ( y > 0 && maze[ x ][ y-1 ].visited == false )
					|| ( y < rows - 1 && maze[ x ][ y+1 ].visited == false )
			  ) {
				keepGoing = true;
			}
		}
	} catch ( std::exception e ) {
		std::wcerr << L"Error in MazeManager::recurseRandom(): " << e.what() << std::endl;
	}
}

//Creates a new maze of the desired size, copies from old maze to new, then deletes old maze
void MazeManager::resizeMaze( uint_least8_t newCols, uint_least8_t newRows ) {
	try {
		uint_least8_t oldCols = cols;
		uint_least8_t oldRows = rows;

		if( gameManager->getDebugStatus() && ( newCols < oldCols || newRows < oldRows ) ) {
			std::wcerr << L"Warning: New maze size smaller than old in some dimension. newCols: " << static_cast<unsigned int>( newCols ) << L" oldCols: " << static_cast<unsigned int>( oldCols ) << L" newRows: " << static_cast<unsigned int>( newRows ) << L" oldRows: " << static_cast<unsigned int>( oldRows ) << std::endl;
		}

		MazeCell** temp = new MazeCell *[ newCols ];

		for( uint_fast8_t i = 0 ; i < newCols ; ++i ) {
			temp[ i ] = new MazeCell[ newRows ];
		}

		uint_least8_t colsToCopy;

		if( newCols > oldCols ) {
			colsToCopy = oldCols;
		} else {
			colsToCopy = newCols;
		}

		uint_least8_t rowsToCopy;

		if( newRows > oldRows ) {
			rowsToCopy = oldRows;
		} else {
			rowsToCopy = newRows;
		}

		for( uint_fast8_t x = 0; x < colsToCopy; ++x ) {
			for( uint_fast8_t y = 0; y < rowsToCopy; ++y ) {
				temp[ x ][ y ] = maze[ x ][ y ];
			}
		}

		for( uint_fast8_t i = 0 ; i < oldCols ; ++i ) {
			delete [ ] maze[ i ];
		}

		delete [ ] maze ;

		cols = newCols;
		rows = newRows;

		maze = temp;
	} catch ( std::exception e ) {
		std::wcerr << L"Error in MazeManager::resizeMaze(): " << e.what() << std::endl;
	}
}

bool MazeManager::saveToFile() {
	try {
		return saveToFile( L"default.maz" );
	} catch ( std::exception e ) {
		std::wcerr << L"Error in MazeManager::saveToFile(): " << e.what() << std::endl;
		return false;
	}
}

bool MazeManager::saveToFile( boost::filesystem::path dest ) {
	try {
		if( is_directory( dest ) ) {
			throw( std::wstring( L"Directory specified, file needed: " ) + dest.wstring() );
		}

		boost::filesystem::wofstream file; //Identical to a standard C++ wofstream, except it takes Boost paths
		file.open( dest );

		if( file.is_open() ) {
			file << gameManager->randomSeed;
			core::stringw message( L"This maze has been saved to the file " );
			message += gameManager->stringConverter.toIrrlichtStringW( dest.wstring() );
			gameManager->gui->addMessageBox( L"Maze saved", message.c_str() );
			file.close();
		} else {
			core::stringw message( L"Cannot save to file " );
			message += gameManager->stringConverter.toIrrlichtStringW( dest.wstring() );
			std::wcerr << message.c_str() << std::endl;
			gameManager->gui->addMessageBox( L"Maze NOT saved", message.c_str() );
		}

		return true;
	} catch( const boost::filesystem::filesystem_error e ) {
		std::wcerr << L"Boost Filesystem error in MazeManager::saveToFile(): " << e.what() << std::endl;
		return false;
	} catch( std::exception e ) {
		std::wcerr << L"non-Boost-Filesystem error in MazeManager::saveToFile(): " << e.what() << std::endl;
		return false;
	} catch( std::wstring e ) {
		std::wcerr << L"non-Boost-Filesystem error in MazeManager::saveToFile(): " << e << std::endl;
		return false;
	}
}

void MazeManager::setGameManager( GameManager* newGM ) {
	try {
		gameManager = newGM;
	} catch ( std::exception e ) {
		std::wcerr << L"Error in MazeManager::setGameManager(): " << e.what() << std::endl;
	}
}
