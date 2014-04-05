#include "colors.h"
#include "MazeManager.h"
#include "GameManager.h"

#include <boost/filesystem/fstream.hpp>
#ifdef HAVE_IOSTREAM
#include <iostream>
#endif //HAVE_IOSTREAM


//Recursively searches the maze to see if it can get from start to end
bool MazeManager::canGetTo( uint_fast8_t startX, uint_fast8_t startY, uint_fast8_t goalX, uint_fast8_t goalY ) {
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
	} catch ( std::exception &e ) {
		std::wcerr << L"Error in MazeManager::canGetTo(): " << e.what() << std::endl;
		return false;
	}
}

//Iterates through all collectables, calls canGetTo on their locations
bool MazeManager::canGetToAllCollectables( uint_fast8_t startX, uint_fast8_t startY ) {
	try {
		bool result = true;

		for( decltype( gameManager->stuff.size() ) i = 0; ( i < gameManager->stuff.size() && result == true ); ++i ) {

			//Do this here because canGetTo() uses the visited variable
			for( decltype( cols ) x = 0; x < cols; ++x ) {
				for( decltype( rows ) y = 0; y < rows; ++y ) {
					maze[ x ][ y ].visited = false;
				}
			}

			bool otherResult = canGetTo( startX, startY, gameManager->stuff[ i ].getX(), gameManager->stuff[ i ].getY() );

			if( otherResult == false ) {
				result = false;
			}
		}

		return result;
	} catch ( std::exception &e ) {
		std::wcerr << L"Error in MazeManager::canGetToAllCollectables(): " << e.what() << std::endl;
		return false;
	}
}

void MazeManager::draw( video::IVideoDriver* driver, uint_fast16_t cellWidth, uint_fast16_t cellHeight ) {
	try {
		video::SColor wallColor = WHITE;
		video::SColor lockColor = BROWN;
		video::SColor acidProofWallColor = LIGHTGREEN;
		video::SColor wallShadowColor = BLACK;
		video::SColor lockShadowColor = BLACK;
		video::SColor acidProofWallShadowColor = BLACK;
		core::position2d< s32 > shadowOffset( 1, 1 );

		for( decltype( cols ) x = 0; x < cols; ++x ) {
			for( decltype( rows ) y = 0; y < rows; ++y ) {
				if( maze[ x ][ y ].visible ) {
					if( maze[ x ][ y ].getTop() == MazeCell::WALL ) {
						driver->draw2DLine( shadowOffset + core::position2d< s32 >( cellWidth * x, cellHeight * y ), shadowOffset + core::position2d< s32 >(cellWidth * ( x + 1 ), cellHeight * y ), wallShadowColor );
						driver->draw2DLine( core::position2d< s32 >( cellWidth * x, cellHeight * y ), core::position2d< s32 >(cellWidth * ( x + 1 ), cellHeight * y ), wallColor );
					} else if( maze[ x ][ y ].getTop() == MazeCell::ACIDPROOF ) {
						driver->draw2DLine( shadowOffset + core::position2d< s32 >( cellWidth * x, cellHeight * y ), shadowOffset + core::position2d< s32 >(cellWidth * ( x + 1 ), cellHeight * y ), acidProofWallShadowColor );
						driver->draw2DLine( core::position2d< s32 >( cellWidth * x, cellHeight * y ), core::position2d< s32 >(cellWidth * ( x + 1 ), cellHeight * y ), acidProofWallColor );
					} else if( maze[ x ][ y ].getTop() == MazeCell::LOCK ) {
						driver->draw2DLine( shadowOffset + core::position2d< s32 >( cellWidth * x, cellHeight * y ), shadowOffset + core::position2d< s32 >(cellWidth * ( x + 1 ), cellHeight * y ), lockShadowColor );
						driver->draw2DLine( core::position2d< s32 >( cellWidth * x, cellHeight * y ), core::position2d< s32 >(cellWidth * ( x + 1 ), cellHeight * y ), lockColor );
					}

					if( maze[ x ][ y ].getLeft() == MazeCell::WALL ) {
						driver->draw2DLine( shadowOffset + core::position2d< s32 >( cellWidth * x, cellHeight * y ), shadowOffset + core::position2d< s32 >( cellWidth * x, cellHeight * ( y + 1 ) ), wallShadowColor );
						driver->draw2DLine( core::position2d< s32 >( cellWidth * x, cellHeight * y ), core::position2d< s32 >( cellWidth * x, cellHeight * ( y + 1 ) ), wallColor );
					} else if( maze[ x ][ y ].getLeft() == MazeCell::ACIDPROOF ) {
						driver->draw2DLine( shadowOffset + core::position2d< s32 >( cellWidth * x, cellHeight * y ), shadowOffset + core::position2d< s32 >( cellWidth * x, cellHeight * ( y + 1 ) ), acidProofWallShadowColor );
						driver->draw2DLine( core::position2d< s32 >( cellWidth * x, cellHeight * y ), core::position2d< s32 >( cellWidth * x, cellHeight * ( y + 1 ) ), acidProofWallColor );
					} else if( maze[ x ][ y ].getLeft() == MazeCell::LOCK ) {
						driver->draw2DLine( shadowOffset + core::position2d< s32 >( cellWidth * x, cellHeight * y ), shadowOffset + core::position2d< s32 >( cellWidth * x, cellHeight * ( y + 1 ) ), lockShadowColor );
						driver->draw2DLine( core::position2d< s32 >( cellWidth * x, cellHeight * y ), core::position2d< s32 >( cellWidth * x, cellHeight * ( y + 1 ) ), lockColor );
					}

					//Only cells on the right or bottom edge of the maze should have anything other than NONE as right or bottom, and then it should only be a solid WALL
					if( maze[ x ][ y ].getRight() == MazeCell::ACIDPROOF ) {
						driver->draw2DLine( shadowOffset + core::position2d< s32 >( cellWidth * ( x + 1 ), cellHeight * y ), shadowOffset + core::position2d< s32 >( cellWidth * ( x + 1 ), cellHeight * ( y + 1 ) ), wallShadowColor );
						driver->draw2DLine( core::position2d< s32 >( cellWidth * ( x + 1 ), cellHeight * y ), core::position2d< s32 >( cellWidth * ( x + 1 ), cellHeight * ( y + 1 ) ), acidProofWallColor );
					}

					if( maze[ x ][ y ].getBottom() == MazeCell::ACIDPROOF ) {
						driver->draw2DLine( shadowOffset + core::position2d< s32 >( cellWidth * x, cellHeight * ( y + 1 ) ), shadowOffset + core::position2d< s32 >( cellWidth * ( x + 1 ), cellHeight * ( y + 1 ) ), wallShadowColor );
						driver->draw2DLine( core::position2d< s32 >( cellWidth * x, cellHeight * ( y + 1 ) ), core::position2d< s32 >( cellWidth * ( x + 1 ), cellHeight * ( y + 1 ) ), acidProofWallColor );
					}
				}
			}
		}
	} catch ( std::exception &e ) {
		std::wcerr << L"Error in MazeManager::draw(): " << e.what() << std::endl;
	}
}

//Does everything involved in making the maze, calls other functions as needed.
void MazeManager::makeRandomLevel() {
	try {
		gameManager->drawAll();
		// Flawfinder: ignore
		srand( gameManager->randomSeed ); //randomSeed is set either by resetThings() or by loadFromFile()
		{
			decltype( cols ) tempCols = rand() % 28 + 2; //I don't remember where I got the 28. The 2 is so there's some minimum amount.
			decltype( rows ) tempRows = tempCols + ( rand() % 5 ); //Again, no idea where the 5 came from.
			newMaze( tempCols, tempRows );
		}

		for( decltype( cols ) x = 0; x < cols; ++x ) {
			for( decltype( rows ) y = 0; y < rows; ++y ) {
				maze[ x ][ y ].setOriginalTop( MazeCell::WALL );
				maze[ x ][ y ].setOriginalLeft( MazeCell::WALL );
				maze[ x ][ y ].setOriginalRight( MazeCell::NONE );
				maze[ x ][ y ].setOriginalBottom( MazeCell::NONE );
				maze[ x ][ y ].visited = false;
			}
		}

		for( decltype( gameManager->numPlayers ) p = 0; p < gameManager->numPlayers; ++p ) {
			gameManager->playerStart[ p ].reset();
		}
		
		{
			decltype( cols ) goalX = rand() % cols;
			decltype( rows ) goalY = rand() % rows;
			gameManager->goal.setX( goalX );
			gameManager->goal.setY( goalY );
			//Make the goal inaccessible unless we've found all the keys (locks are place elsewhere in the code but one lock does get placed at the goal)
			maze[ goalX ][ goalY ].setOriginalTop( MazeCell::ACIDPROOF );
			maze[ goalX ][ goalY ].setOriginalLeft( MazeCell::ACIDPROOF );
			if( goalX < ( cols - 1 ) ) {
				maze[ goalX + 1 ][ goalY ].setOriginalLeft( MazeCell::ACIDPROOF );
			}
			if( goalY < ( rows - 1 ) ) {
				maze[ goalX ][ goalY + 1 ].setOriginalTop( MazeCell::ACIDPROOF );
			}
			
			recurseRandom( goalX, goalY, 0, 0 ); //Start recursion from the goal's position; for some reason that makes the mazes harder than if we started recursion from the player's starting point.
		}
		
		//Add walls at maze borders
		for( decltype( cols ) x = 0; x < cols; ++x ) {
			maze[ x ][ 0 ].setOriginalTop( MazeCell::ACIDPROOF );
			maze[ x ][ rows-1 ].setOriginalBottom( MazeCell::ACIDPROOF );
		}

		for( decltype( rows ) y = 0; y < rows; ++y ) {
			maze[ 0 ][ y ].setOriginalLeft( MazeCell::ACIDPROOF );
			maze[ cols-1 ][ y ].setOriginalRight( MazeCell::ACIDPROOF );
		}

		for( decltype( cols ) x = 1; x < cols; ++x ) {
			for( decltype( rows ) y = 0; y < rows; ++y ) {
				maze[ x-1 ][ y ].setOriginalRight( maze[ x ][ y ].getLeft() );
			}
		}

		for( decltype( cols ) x = 0; x < cols; ++x ) {
			for( decltype( rows ) y = 1; y < rows; ++y ) {
				maze[ x ][ y-1 ].setOriginalBottom( maze[ x ][ y ].getTop() );
			}
		}

		//Find all dead ends. I'm sure it would be more efficient to do this during maze generation rather than going back through afterward, but I can't be bothered with that now.
		std::vector< decltype( cols ) > deadEndsX;
		std::vector< decltype( rows ) > deadEndsY;

		for( decltype( cols ) x = 0; x < cols; ++x ) {
			for( decltype( rows ) y = 0; y < rows; ++y ) {
				if( maze[ x ][ y ].isDeadEnd() ) {
					deadEndsX.push_back( x );
					deadEndsY.push_back( y );
				}
			}
		}

		//Remove player starts from list of dead ends
		for( decltype( gameManager->numPlayers ) p = 0; p < gameManager->numPlayers; ++p ) {
			for( decltype( deadEndsX.size() ) i = 0; i < deadEndsX.size(); ++i ) {
				if( gameManager->playerStart.at( p ).getX() == deadEndsX.at( i ) && gameManager->playerStart.at( p ).getY() == deadEndsY.at( i ) ) {
					deadEndsX.erase( deadEndsX.begin() + i );
					deadEndsY.erase( deadEndsY.begin() + i );
				}
			}
		}

		//Remove goal from list of dead ends
		for( decltype( deadEndsX.size() ) i = 0; i < deadEndsX.size(); ++i ) {
			if( gameManager->goal.getX() == deadEndsX.at( i ) && gameManager->goal.getY() == deadEndsY.at( i ) ) {
				deadEndsX.erase( deadEndsX.begin() + i );
				deadEndsY.erase( deadEndsY.begin() + i );
			}
		}
		
		{ //Decide how many keys/locks to use (# of keys = # of locks)
			decltype( gameManager->numLocks ) temp = rand() % cols;
			if( deadEndsX.size() > 0 ) {
				temp = temp % deadEndsX.size();
			}
			gameManager->numLocks = temp;
		}

		//gameManager->numLocks = rand() % ( cols * rows ); //Uncomment this for a crazy number of keys!

		decltype( gameManager->numLocks ) numKeys = gameManager->numLocks;

		//Place keys in dead ends
		for( decltype( numKeys ) k = 0; k < numKeys; ++k ) {

			if( deadEndsX.size() == 0 ) {
				deadEndsX.push_back( gameManager->playerStart.at( 0 ).getX() );
				deadEndsY.push_back( gameManager->playerStart.at( 0 ).getY() );
			}
			
			{ //Pick one of the dead ends randomly.
				decltype( deadEndsX.size() ) chosen = rand() % deadEndsX.size();

				{ //Finally, create a key and put it there.
					Collectable temp;
					temp.setX( deadEndsX.at( chosen ) );
					temp.setY( deadEndsY.at( chosen ) );
					temp.setType( Collectable::KEY );
					temp.loadTexture( gameManager->driver );
					gameManager->stuff.push_back( temp );
				}

				//Remove chosen from the list of dead ends so no other keys go there
				deadEndsX.erase( deadEndsX.begin() + chosen );
				deadEndsY.erase( deadEndsY.begin() + chosen );
			}
		}
		
		{
			uint_fast8_t acidChance = UINT_FAST8_MAX; //Acid is supposed to be really rare.
			if( gameManager->getDebugStatus() ) {
				acidChance = 1; //If the game is being debugged, ensure the acid is always there - it may be what's being debugged.
			}
			if( rand() % acidChance == 0 ) {
				if( deadEndsX.empty() ) { //If all the dead ends have been filled with other collectables
					Collectable temp;
					temp.setX( rand() % cols );
					temp.setY( rand() % rows );
					temp.setType( Collectable::ACID );
					temp.loadTexture( gameManager->driver );
					gameManager->stuff.push_back( temp );
				} else {
					//Pick one of the dead ends randomly.
					decltype( deadEndsX.size() ) chosen = rand() % deadEndsX.size();

					{ //Finally, create an acid and put it there.
						Collectable temp;
						temp.setX( deadEndsX.at( chosen ) );
						temp.setY( deadEndsY.at( chosen ) );
						temp.setType( Collectable::ACID );
						temp.loadTexture( gameManager->driver );
						gameManager->stuff.push_back( temp );
					}

					//Remove chosen from the list of dead ends so no other stuff goes there
					deadEndsX.erase( deadEndsX.begin() + chosen );
					deadEndsY.erase( deadEndsY.begin() + chosen );
				}
			}
		}

		for( decltype( gameManager->numPlayers ) p = 0; p < gameManager->numPlayers; ++p ) {
			gameManager->player[ p ].setPos( gameManager->playerStart[ p ].getX(), gameManager->playerStart[ p ].getY() );
		}

		if( gameManager->numLocks > 0 ) {
			//Place locks
			//Place first lock at the gameManager->goal
			if( maze[ gameManager->goal.getX() ][ gameManager->goal.getY() ].getTop() == MazeCell::NONE ) {
				maze[ gameManager->goal.getX() ][ gameManager->goal.getY() ].setOriginalTop( MazeCell::LOCK );
			} else if( maze[ gameManager->goal.getX() ][ gameManager->goal.getY() ].getLeft() == MazeCell::NONE ) {
				maze[ gameManager->goal.getX() ][ gameManager->goal.getY() ].setOriginalLeft( MazeCell::LOCK );
			} else if( maze[ gameManager->goal.getX() ][ gameManager->goal.getY() + 1 ].getTop() == MazeCell::NONE ) {
				maze[ gameManager->goal.getX() ][ gameManager->goal.getY() + 1 ].setOriginalTop( MazeCell::LOCK );
			} else if( maze[ gameManager->goal.getX() + 1 ][ gameManager->goal.getY() ].getLeft() == MazeCell::NONE ) {
				maze[ gameManager->goal.getX() + 1 ][ gameManager->goal.getY() ].setOriginalLeft( MazeCell::LOCK );
			}

			decltype( gameManager->numLocks ) numLocksPlaced = 1;

			while( gameManager->device->run() != false && numLocksPlaced < gameManager->numLocks && gameManager->timer->getTime() < gameManager->timeStartedLoading + gameManager->loadingDelay ) {
				decltype( cols ) tempX = rand() % cols;
				decltype( rows ) tempY = rand() % rows;

				if( maze[ tempX ][ tempY ].getTop() == MazeCell::NONE ) {
					maze[ tempX ][ tempY ].setOriginalTop( MazeCell::LOCK );

					if( canGetToAllCollectables( gameManager->playerStart[ 0 ].getX(), gameManager->playerStart[ 0 ].getY() ) ) {
						numLocksPlaced += 1;
						if( gameManager->getDebugStatus() ) {
							std::wcout << L"Placed lock " << numLocksPlaced << L" at " << tempX << L"x" << tempY << std::endl;
						}
					} else {
						maze[ tempX ][ tempY ].setOriginalTop( MazeCell::NONE );
					}
				} else if( maze[ tempX ][ tempY ].getLeft() == MazeCell::NONE ) {
					maze[ tempX ][ tempY ].setOriginalLeft( MazeCell::LOCK );

					if( canGetToAllCollectables( gameManager->playerStart[ 0 ].getX(), gameManager->playerStart[ 0 ].getY() ) ) {
						numLocksPlaced += 1;
						if( gameManager->getDebugStatus() ) {
							std::wcout << L"Placed lock " << numLocksPlaced << L" at " << tempX << L"x" << tempY << std::endl;
						}
					} else {
						maze[ tempX ][ tempY ].setOriginalLeft( MazeCell::NONE );
					}
				}
			}

			gameManager->timer->stop();
			gameManager->timer->setTime( 0 );
			if( gameManager->getDebugStatus() ) {
				std::wcout << L"numLocksPlaced: " << numLocksPlaced << L"\tnumLocks: " << gameManager->numLocks << std::endl;
			}

			if( numLocksPlaced < gameManager->numLocks ) {
				decltype( numLocksPlaced ) keysToRemove = gameManager->numLocks - numLocksPlaced;

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

		for( decltype( cols ) x = 0; x < cols; ++x ) {
			for( decltype( rows ) y = 0; y < rows; ++y ) {
				maze[ x ][ y ].visited = false;
			}
		}

		for( decltype( gameManager->numPlayers ) p = 0; p < gameManager->numPlayers; ++p ) {
			maze[ gameManager->playerStart[ p ].getX() ][ gameManager->playerStart[ p ].getY() ].visited = true;
			maze[ gameManager->playerStart[ p ].getX() ][ gameManager->playerStart[ p ].getY() ].setVisitorColor( gameManager->player[ p ].getColorTwo() );
		}
	} catch ( std::exception &e ) {
		std::wcerr << L"Error in MazeManager::makeRandomLevel(): " << e.what() << std::endl;
	}
}

MazeManager::MazeManager() {
	try {
		//resizeMaze() will set cols and rows to whatever gets passed into it; we're making them zero here only so that resizeMaze() doesn't try to copy from the nonexistent previous maze.
		cols = 0;
		rows = 0;
		maze = nullptr;
		gameManager = nullptr;
	} catch ( std::exception &e ) {
		std::wcerr << L"Error in MazeManager::MazeManager(): " << e.what() << std::endl;
	}
}

MazeManager::~MazeManager() {
	try {
		for( decltype( cols ) i = 0 ; i < cols ; ++i ) {
			delete [ ] maze[ i ];
		}

		delete [ ] maze ;
	} catch ( std::exception &e ) {
		std::wcerr << L"Error in MazeManager::~MazeManager(): " << e.what() << std::endl;
	}
}

//Creates a new maze of the desired size, then deletes old maze
void MazeManager::newMaze( uint_fast8_t newCols, uint_fast8_t newRows ) {
	try {
		decltype( cols ) oldCols = cols;

		MazeCell** newMaze = new MazeCell *[ newCols ];

		for( decltype( newCols ) i = 0 ; i < newCols ; ++i ) {
			newMaze[ i ] = new MazeCell[ newRows ];
		}

		for( decltype( oldCols ) i = 0 ; i < oldCols ; ++i ) {
			delete [ ] maze[ i ];
		}

		delete [ ] maze ;

		cols = newCols;
		rows = newRows;

		maze = newMaze;
	} catch ( std::exception &e ) {
		std::wcerr << L"Error in MazeManager::resizeMaze(): " << e.what() << std::endl;
	}
}

//Generates the maze recursively
void MazeManager::recurseRandom( uint_fast8_t x, uint_fast8_t y, uint_fast16_t depth, uint_fast16_t numSoFar ) {
	try {
		//At one point I included the following two lines because I hoped to show the maze as it was being generated. Might still add that as an option. Now they're there so that the loading screen gets drawn and lasts long enough to read it.
		gameManager->device->run();
		gameManager->drawAll();

		maze[ x ][ y ].visited = true;
		maze[ x ][ y ].id = numSoFar;

		for( decltype( gameManager->numPlayers ) p = 0; p < gameManager->numPlayers; ++p ) {
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
						maze[ x ][ y ].setOriginalLeft( MazeCell::NONE );

						recurseRandom( x - 1, y, depth + 1, numSoFar );
					}

					break;

				case 1: //Right

					if( x < cols - 1 && maze[ x+1 ][ y ].visited == false ) {
						maze[ x+1 ][ y ].setOriginalLeft( MazeCell::NONE );

						recurseRandom( x + 1, y, depth + 1, numSoFar );
					}

					break;

				case 2: //Up

					if( y > 0 && maze[ x ][ y-1 ].visited == false ) {
						maze[ x ][ y ].setOriginalTop( MazeCell::NONE );

						recurseRandom( x, y - 1, depth + 1, numSoFar );
					}

					break;

				case 3: //Down

					if( y < rows - 1 && maze[ x ][ y+1 ].visited == false ) {
						maze[ x ][ y+1 ].setOriginalTop( MazeCell::NONE );

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
	} catch ( std::exception &e ) {
		std::wcerr << L"Error in MazeManager::recurseRandom(): " << e.what() << std::endl;
	}
}

bool MazeManager::saveToFile() {
	try {
		return saveToFile( L"default.maz" );
	} catch ( std::exception &e ) {
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
			gameManager->gui->addMessageBox( L"Maze saved", gameManager->stringConverter.toStdWString( message ).c_str() ); //stringConverter.toWCharArray( message ) );
			file.close();
		} else {
			core::stringw message( L"Cannot save to file " );
			message += gameManager->stringConverter.toIrrlichtStringW( dest.wstring() );
			std::wcerr << gameManager->stringConverter.toStdWString( message ) << std::endl; //stringConverter.toWCharArray( message ) << std::endl;
			gameManager->gui->addMessageBox( L"Maze NOT saved", gameManager->stringConverter.toStdWString( message ).c_str() ); //stringConverter.toWCharArray( message ) );
		}

		return true;
	} catch( const boost::filesystem::filesystem_error &e ) {
		std::wcerr << L"Boost Filesystem error in MazeManager::saveToFile(): " << e.what() << std::endl;
		return false;
	} catch( std::exception &e ) {
		std::wcerr << L"non-Boost-Filesystem error in MazeManager::saveToFile(): " << e.what() << std::endl;
		return false;
	} catch( std::wstring &e ) {
		std::wcerr << L"non-Boost-Filesystem error in MazeManager::saveToFile(): " << e << std::endl;
		return false;
	}
}

void MazeManager::setGameManager( GameManager* newGM ) {
	try {
		gameManager = newGM;
	} catch ( std::exception &e ) {
		std::wcerr << L"Error in MazeManager::setGameManager(): " << e.what() << std::endl;
	}
}
