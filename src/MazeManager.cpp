#include "colors.h"
#include "MazeManager.h"
#include "GameManager.h"

#define BOOST_FILESYSTEM_NO_DEPRECATED //Recommended by the Boost filesystem library documentation to prevent us from using functions which will be removed in later versions
#include <filesystem/fstream.hpp>

using namespace std;

//Recursively searches the maze to see if it can get from start to end
bool MazeManager::canGetTo( uint8_t startX, uint8_t startY, uint8_t goalX, uint8_t goalY ) {
	bool found = false;
	if( gameManager->getDebugStatus() ) {
		wcout << L"Searching for a way from " << static_cast<unsigned int>( startX ) << L"x" << static_cast<unsigned int>( startY ) << L" to " << static_cast<unsigned int>( goalX ) << L"x" << static_cast<unsigned int>( goalY ) << endl;
	}
	maze[startX][startY].visited = true;
	
	if( startX == goalX && startY == goalY ) {
		found = true;
	} else {
		if( startY > 0 && maze[startX][startY].getTop() == 'n' && maze[startX][startY - 1].visited == false ) {
			found = canGetTo( startX, startY - 1, goalX, goalY );
		}
		
		if( found == false && startY < ( rows - 1 ) && maze[startX][startY + 1].getTop() == 'n' && maze[startX][startY + 1].visited == false ) {
			found = canGetTo( startX, startY + 1, goalX, goalY );
		}
		
		if( found == false && startX < ( cols - 1 ) && maze[startX + 1][startY].getLeft() == 'n' && maze[startX + 1][startY].visited == false ) {
			found = canGetTo( startX + 1, startY, goalX, goalY );
		}
		
		if( found == false && startX > 0 && maze[startX][startY].getLeft() == 'n' && maze[startX - 1][startY].visited == false ) {
			found = canGetTo( startX - 1, startY, goalX, goalY );
		}
	}
	
	return found;
}

//Iterates through all collectables, calls canGetTo on their locations
bool MazeManager::canGetToAllCollectables( uint8_t startX, uint8_t startY ) {
	bool result = true;
	
	for( uint32_t i = 0; ( i < gameManager->stuff.size() && result == true ); i++ ) {
	
		//Do this here because canGetTo() uses the visited variable
		for( uint8_t x = 0; x < cols; x++ ) {
			for( uint8_t y = 0; y < rows; y++ ) {
				maze[x][y].visited = false;
			}
		}
		
		bool otherResult = canGetTo( startX, startY, gameManager->stuff[i].getX(), gameManager->stuff[i].getY() );
		
		if( otherResult == false ) {
			result = false;
		}
	}
	
	return result;
}

void MazeManager::draw( video::IVideoDriver* driver, uint32_t cellWidth, uint32_t cellHeight ) {
	video::SColor wallColor = WHITE;
	video::SColor lockColor = BROWN;
	
	for( uint8_t x = 0; x < cols; x++ ) {
		for( uint8_t y = 0; y < rows; y++ ) {
			if( maze[x][y].visible ) {
				if( maze[x][y].getTop() == 'w' ) {
					driver->draw2DLine( core::position2d< irr::s32 >( cellWidth * x, cellHeight * y ), core::position2d< irr::s32 >(cellWidth * ( x + 1 ), cellHeight * y ), wallColor );
				} else if( maze[x][y].getTop() == 'l' ) {
					driver->draw2DLine( core::position2d< irr::s32 >( cellWidth * x, cellHeight * y ), core::position2d< irr::s32 >(cellWidth * ( x + 1 ), cellHeight * y ), lockColor );
				}
				
				if( maze[x][y].getLeft() == 'w' ) {
					driver->draw2DLine( core::position2d< irr::s32 >( cellWidth * x, cellHeight * y ), core::position2d< irr::s32 >( cellWidth * x, cellHeight * ( y + 1 ) ), wallColor );
				} else if( maze[x][y].getLeft() == 'l' ) {
					driver->draw2DLine( core::position2d< irr::s32 >( cellWidth * x, cellHeight * y ), core::position2d< irr::s32 >( cellWidth * x, cellHeight * ( y + 1 ) ), lockColor );
				}
				
				//Only cells on the right or bottom edge of the maze should have anything other than 'n' as right or bottom, and then it should only be a solid 'w'
				if( maze[x][y].getRight() == 'w' ) {
					driver->draw2DLine( core::position2d< irr::s32 >( cellWidth * ( x + 1 ), cellHeight * y ), core::position2d< irr::s32 >( cellWidth * ( x + 1 ), cellHeight * ( y + 1 ) ), wallColor );
				}
				
				if( maze[x][y].getBottom() == 'w' ) {
					driver->draw2DLine( core::position2d< irr::s32 >( cellWidth * x, cellHeight * ( y + 1 ) ), core::position2d< irr::s32 >( cellWidth * ( x + 1 ), cellHeight * ( y + 1 ) ), wallColor );
				}
			}
		}
	}
}

bool MazeManager::existsAnythingAt( uint8_t x, uint8_t y ) {
	bool result = false;
	
	if( gameManager->goal.getX() == x && gameManager->goal.getY() == y ) {
		result = true;
	}
	
	for( uint32_t i = 0; ( result == false && i < gameManager->stuff.size() ); i++ ) {
		if( gameManager->stuff[ i ].getX() == x && gameManager->stuff[ i ].getY() == y ) {
			result = true;
		}
	}
	
	return result;
}

bool MazeManager::loadFromFile() {
	return loadFromFile( L"default.maz" );
}

bool MazeManager::loadFromFile( boost::filesystem::path src ) {
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
			gameManager->resetThings();
			file >> gameManager->randomSeed;
			file.close();
			srand( gameManager->randomSeed );
			makeRandomLevel();
			return true;
		} else {
			wcerr << L"Cannot open file " << src << endl;
			return false;
		}
	} catch( const boost::filesystem::filesystem_error& e ) {
		wcerr << e.what() << endl;
		return false;
	} catch( exception& e ) {
		wcerr << e.what() << endl;
		return false;
	}
	
	return false;
}

//Does everything involved in making the maze, calls other functions as needed.
void MazeManager::makeRandomLevel() {
	gameManager->drawAll();
	
	srand( gameManager->randomSeed ); //randomSeed is set either by resetThings() or by loadFromFile()
	uint8_t tempCols = rand() % 28 + 2;
	uint8_t tempRows = tempCols + ( rand() % 5 );
	resizeMaze( tempCols, tempRows );
	
	for( uint8_t x = 0; x < cols; x++ ) {
		for( uint8_t y = 0; y < rows; y++ ) {
			maze[x][y].setTop( 'w' );
			maze[x][y].setLeft( 'w' );
			maze[x][y].setRight( 'n' );
			maze[x][y].setBottom( 'n' );
			maze[x][y].visited = false;
		}
	}
	
	for( uint8_t p = 0; p < gameManager->numPlayers; p++ ) {
		gameManager->playerStart[p].reset();
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
	for( uint8_t x = 0; x < cols; x++ ) {
		maze[x][0].setTop( 'w' );
		maze[x][rows-1].setBottom( 'w' );
	}
	
	for( uint8_t y = 0; y < rows; y++ ) {
		maze[0][y].setLeft( 'w' );
		maze[cols-1][y].setRight( 'w' );
	}
	
	for( uint8_t x = 1; x < cols; x++ ) {
		for( uint8_t y = 0; y < rows; y++ ) {
			maze[x-1][y].setRight( maze[x][y].getLeft() );
		}
	}
	
	for( uint8_t x = 0; x < cols; x++ ) {
		for( uint8_t y = 1; y < rows; y++ ) {
			maze[x][y-1].setBottom( maze[x][y].getTop() );
		}
	}
	
	//Find all dead ends. I'm sure it would be more efficient to do this during maze generation rather than going back through afterward, but I can't be bothered with that now.
	vector<uint8_t> deadEndsX;
	vector<uint8_t> deadEndsY;
	
	for( uint8_t x = 0; x < cols; x++ ) {
		for( uint8_t y = 0; y < rows; y++ ) {
			if( maze[x][y].isDeadEnd() ) {
				deadEndsX.push_back( x );
				deadEndsY.push_back( y );
			}
		}
	}
	
	//Remove player starts from list of dead ends
	for( uint8_t p = 0; p < gameManager->numPlayers; p++ ) {
		for( uint32_t i = 0; i < deadEndsX.size(); i++ ) {
			if( gameManager->playerStart[p].getX() == deadEndsX[i] && gameManager->playerStart[p].getY() == deadEndsY[i] ) {
				deadEndsX.erase( deadEndsX.begin() + i );
				deadEndsY.erase( deadEndsY.begin() + i );
			}
		}
	}
	
	//Remove gameManager->goal from list of dead ends
	for( uint32_t i = 0; i < deadEndsX.size(); i++ ) {
		if( gameManager->goal.getX() == deadEndsX[i] && gameManager->goal.getY() == deadEndsY[i] ) {
			deadEndsX.erase( deadEndsX.begin() + i );
			deadEndsY.erase( deadEndsY.begin() + i );
		}
	}
	
	if( gameManager->numLocks > deadEndsX.size() ) {
		gameManager->numLocks = deadEndsX.size();
	}
	
	uint8_t numKeys = gameManager->numLocks;
	
	//Place keys in dead ends
	vector<uint8_t> keyPlaceX;
	vector<uint8_t> keyPlaceY;
	
	for( uint8_t k = 0; k < numKeys; k++ ) {
		vector<uint8_t> chosenPlaces;
		
		if( deadEndsX.size() == 0 ) {
			deadEndsX.push_back( gameManager->playerStart[0].getX() );
			deadEndsY.push_back( gameManager->playerStart[0].getY() );
		}
		//Pick one of the dead ends randomly.
		uint8_t chosen = rand() % deadEndsX.size();
		
		//Finally, create a key and put it there.
		Collectable temp;
		temp.setX( deadEndsX.at( chosen ) );
		temp.setY( deadEndsY.at( chosen ) );
		temp.setType( COLLECTABLE_KEY );
		temp.loadImage( gameManager->driver );
		gameManager->stuff.push_back( temp );
		
		//Remove chosen from the list of dead ends so no other keys go there
		deadEndsX.erase( deadEndsX.begin() + chosen );
		deadEndsY.erase( deadEndsY.begin() + chosen );
	}
	
	for( uint8_t p = 0; p < gameManager->numPlayers; p++ ) {
		gameManager->player[p].setPos( gameManager->playerStart[p].getX(), gameManager->playerStart[p].getY() );
	}
	
	if( gameManager->numLocks > 0 ) {
		//Place locks
		//Place first lock at the gameManager->goal
		if( maze[gameManager->goal.getX()][gameManager->goal.getY()].getTop() == 'n' ) {
			maze[gameManager->goal.getX()][gameManager->goal.getY()].setTop( 'l' );
		} else if( maze[gameManager->goal.getX()][gameManager->goal.getY()].getLeft() == 'n' ) {
			maze[gameManager->goal.getX()][gameManager->goal.getY()].setLeft( 'l' );
		} else if( maze[gameManager->goal.getX()][gameManager->goal.getY() + 1].getTop() == 'n' ) {
			maze[gameManager->goal.getX()][gameManager->goal.getY() + 1].setTop( 'l' );
		} else if( maze[gameManager->goal.getX() + 1][gameManager->goal.getY()].getLeft() == 'n' ) {
			maze[gameManager->goal.getX() + 1][gameManager->goal.getY()].setLeft( 'l' );
		}
		
		uint8_t numLocksPlaced = 1;
		
		while( gameManager->device->run() != false && numLocksPlaced < gameManager->numLocks && gameManager->timer->getTime() < gameManager->timeStartedLoading + gameManager->loadingDelay ) {
			uint8_t tempX = rand() % cols;
			uint8_t tempY = rand() % rows;
			
			if( maze[tempX][tempY].getTop() == 'n' ) {
				maze[tempX][tempY].setTop( 'l' );
				
				if( canGetToAllCollectables( gameManager->playerStart[0].getX(), gameManager->playerStart[0].getY() ) ) {
					numLocksPlaced += 1;
					if( gameManager->getDebugStatus() ) {
						wcout << L"Placed lock " << static_cast<unsigned int>( numLocksPlaced ) << L" at " << static_cast<unsigned int>( tempX ) << L"x" << static_cast<unsigned int>( tempY ) << endl;
					}
				} else {
					maze[tempX][tempY].setTop( 'n' );
				}
			} else if( maze[tempX][tempY].getLeft() == 'n' ) {
				maze[tempX][tempY].setLeft( 'l' );
				
				if( canGetToAllCollectables( gameManager->playerStart[0].getX(), gameManager->playerStart[0].getY() ) ) {
					numLocksPlaced += 1;
					if( gameManager->getDebugStatus() ) {
						wcout << L"Placed lock " << static_cast<unsigned int>( numLocksPlaced ) << L" at " << static_cast<unsigned int>( tempX ) << L"x" << static_cast<unsigned int>( tempY ) << endl;
					}
				} else {
					maze[tempX][tempY].setLeft( 'n' );
				}
			}
		}
		
		gameManager->timer->stop();
		gameManager->timer->setTime( 0 );
		if( gameManager->getDebugStatus() ) {
			wcout << L"numLocksPlaced: " << static_cast<unsigned int>( numLocksPlaced ) << L"\tnumLocks: " << static_cast<unsigned int>( gameManager->numLocks ) << endl;
		}
		
		if( numLocksPlaced < gameManager->numLocks ) {
			int keysToRemove = gameManager->numLocks - numLocksPlaced;
			
			for( uint32_t i = 0; ( i < gameManager->stuff.size() && keysToRemove > 0 ); i++ ) {
				if( gameManager->getDebugStatus() ) {
					wcout << L"keysToRemove: " << static_cast<unsigned int>( keysToRemove ) << endl;
				}
				
				if( gameManager->stuff[i].getType() == COLLECTABLE_KEY ) {
					gameManager->stuff.erase( gameManager->stuff.begin() + i );
					i = 0;
					keysToRemove -= 1;
				}
			}
		}
		
		numKeys = gameManager->numLocks = numLocksPlaced;
	}
	
	for( uint8_t x = 0; x < cols; x++ ) {
		for( uint8_t y = 0; y < rows; y++ ) {
			maze[x][y].visited = false;
		}
	}
	
	for( uint8_t p = 0; p < gameManager->numPlayers; p++ ) {
		maze[ gameManager->playerStart[ p ].getX()][ gameManager->playerStart[ p ].getY()].visited = true;
		maze[ gameManager->playerStart[ p ].getX()][ gameManager->playerStart[ p ].getY()].setVisitorColor( gameManager->player[ p ].getColorTwo() );
	}
	
	//Set up bots;
	if( gameManager->numBots > 0 ) {
		for( uint8_t i = 0; i < gameManager->numBots; i++ ) {
			gameManager->bot[ i ].setup( maze, cols, rows, gameManager );
		}
	}
}

MazeManager::MazeManager()
{
	//resizeMaze() will set cols and rows to whatever gets passed into it; we're making them zero here only so that resizeMaze() doesn't try to copy from the nonexistent previous maze.
	cols = 0;
	rows = 0;
}

MazeManager::~MazeManager()
{
	for( uint8_t i = 0 ; i < cols ; i++ ) {
		delete [] maze[ i ];
	}
		
	delete [] maze ;
}

//Generates the maze recursively
void MazeManager::recurseRandom( uint8_t x, uint8_t y, uint16_t depth, uint16_t numSoFar ) {
	gameManager->device->run();
	gameManager->drawAll();
	
	maze[x][y].visited = true;
	maze[x][y].id = numSoFar;
	
	for( uint8_t p = 0; p < gameManager->numPlayers; p++ ) {
		if( depth >= gameManager->playerStart[p].distanceFromExit ) {
			gameManager->playerStart[p].setPos( x, y );
			gameManager->playerStart[p].distanceFromExit = depth;
		}
	}
	
	bool keepGoing = true;
	
	while( keepGoing ) {
		numSoFar += 1;
		
		switch( rand() % 4 ) { //4 = number of directions (up, down, left, right)
			case 0: //Left
			
				if( x > 0 && maze[x-1][y].visited == false ) {
					maze[x][y].setLeft( 'n' );
					
					recurseRandom( x - 1, y, depth + 1, numSoFar );
				}
				
				break;
				
			case 1: //Right
			
				if( x < cols - 1 && maze[x+1][y].visited == false ) {
					maze[x+1][y].setLeft( 'n' );
					
					recurseRandom( x + 1, y, depth + 1, numSoFar );
				}
				
				break;
				
			case 2: //Up
			
				if( y > 0 && maze[x][y-1].visited == false ) {
					maze[x][y].setTop( 'n' );
					
					recurseRandom( x, y - 1, depth + 1, numSoFar );
				}
				
				break;
				
			case 3: //Down
			
				if( y < rows - 1 && maze[x][y+1].visited == false ) {
					maze[x][y+1].setTop( 'n' );
					
					recurseRandom( x, y + 1, depth + 1, numSoFar );
				}
				
				break;
		}
		
		//If we've reached a dead end, don't keep going. Otherwise do.
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

//Creates a new maze of the desired size, copies from old maze to new, then deletes old maze
void MazeManager::resizeMaze( uint8_t newCols, uint8_t newRows ) {
	uint8_t oldCols = cols;
	uint8_t oldRows = rows;
	
	if( gameManager->getDebugStatus() && ( newCols < oldCols || newRows < oldRows ) ) {
		wcerr << L"Warning: New maze size smaller than old in some dimension. newCols: " << static_cast<unsigned int>( newCols ) << L" oldCols: " << static_cast<unsigned int>( oldCols ) << L" newRows: " << static_cast<unsigned int>( newRows ) << L" oldRows: " << static_cast<unsigned int>( oldRows ) << endl;
	}
	
	MazeCell** temp = new MazeCell *[newCols];
	
	for( int i = 0 ; i < newCols ; i++ ) {
		temp[i] = new MazeCell[newRows];
	}
		
	uint8_t colsToCopy;
	
	if( newCols > oldCols ) {
		colsToCopy = oldCols;
	} else {
		colsToCopy = newCols;
	}
	
	uint8_t rowsToCopy;
	
	if( newRows > oldRows ) {
		rowsToCopy = oldRows;
	} else {
		rowsToCopy = newRows;
	}
	
	for( uint8_t x = 0; x < colsToCopy; x++ ) {
		for( uint8_t y = 0; y < rowsToCopy; y++ ) {
			temp[x][y] = maze[x][y];
		}
	}
	
	for( uint8_t i = 0 ; i < oldCols ; i++ ) {
		delete [] maze[ i ];
	}
		
	delete [] maze ;
	
	cols = newCols;
	rows = newRows;
	
	maze = temp;
}

bool MazeManager::saveToFile() {
	return saveToFile( L"default.maz" );
}

bool MazeManager::saveToFile( boost::filesystem::path dest ) {
	try {
		if( is_directory( dest ) ) {
			wcerr << L"Error: Directory specified, file needed. " << dest << endl;
			return false;
		}
		
		boost::filesystem::wofstream file; //Identical to a standard C++ wofstream, except it takes Boost paths
		file.open( dest, ios::out );
		
		if( file.is_open() ) {
			file << gameManager->randomSeed;
			irr::core::stringw message( L"This maze has been saved to the file " );
			message += gameManager->stringConverter.convert( dest.wstring() );
			gameManager->gui->addMessageBox( L"Maze saved", message.c_str() );
			file.close();
		} else {
			irr::core::stringw message( L"Cannot save to file " );
			message += gameManager->stringConverter.convert( dest.wstring() );
			wcerr << message.c_str() << endl;
			gameManager->gui->addMessageBox( L"Maze NOT saved", message.c_str() );
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

void MazeManager::setGameManager( GameManager* newGM ) {
	gameManager = newGM;
}