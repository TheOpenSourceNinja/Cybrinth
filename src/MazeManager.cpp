/**
 * @file
 * @author James Dearing <dearingj@lifetime.oregonstate.edu>
 * 
 * @section LICENSE
 * Copyright © 2012-2015.
 * This file is part of Cybrinth.
 *
 * Cybrinth is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 *
 * Cybrinth is distributed in the hope that it will be fun, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License along with Cybrinth. If not, see <http://www.gnu.org/licenses/>.
 * 
 * @section DESCRIPTION
 * The MazeManager class is responsible for creating, loading, and saving mazes.
 */

#include "colors.h"
#include "CustomException.h"
#include "MazeManager.h"
#include "MainGame.h"
#include "SettingsManager.h"

#include <boost/filesystem/fstream.hpp>
#ifdef HAVE_IOSTREAM
#include <iostream>
#endif //HAVE_IOSTREAM


//Recursively searches the maze to see if it can get from start to end
bool MazeManager::canGetTo( uint_fast8_t startX, uint_fast8_t startY, uint_fast8_t goalX, uint_fast8_t goalY ) {
	try {
		bool found = false;

		maze[ startX ][ startY ].visited = true;

		if( startX == goalX and startY == goalY ) {
			found = true;
		} else {
			if( startY > 0 and maze[ startX ][ startY ].getTop() == MazeCell::NONE and maze[ startX ][ startY - 1 ].visited == false ) {
				found = canGetTo( startX, startY - 1, goalX, goalY );
			}

			if( found == false and startY < ( rows - 1 ) and maze[ startX ][ startY + 1 ].getTop() == MazeCell::NONE and maze[ startX ][ startY + 1 ].visited == false ) {
				found = canGetTo( startX, startY + 1, goalX, goalY );
			}

			if( found == false and startX < ( cols - 1 ) and maze[ startX + 1 ][ startY ].getLeft() == MazeCell::NONE and maze[ startX + 1 ][ startY ].visited == false ) {
				found = canGetTo( startX + 1, startY, goalX, goalY );
			}

			if( found == false and startX > 0 and maze[ startX ][ startY ].getLeft() == MazeCell::NONE and maze[ startX - 1 ][ startY ].visited == false ) {
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

		for( decltype( mainGame->stuff.size() ) i = 0; ( i < mainGame->stuff.size() and result == true ); ++i ) {

			//Do this here because canGetTo() uses the visited variable
			for( decltype( cols ) x = 0; x < cols; ++x ) {
				for( decltype( rows ) y = 0; y < rows; ++y ) {
					maze[ x ][ y ].visited = false;
				}
			}

			bool otherResult = canGetTo( startX, startY, mainGame->stuff[ i ].getX(), mainGame->stuff[ i ].getY() );

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

void MazeManager::draw( irr::IrrlichtDevice* device, uint_fast16_t cellWidth, uint_fast16_t cellHeight ) {
	try {
		auto* driver = device->getVideoDriver();
		irr::video::SColor wallColor = WHITE;
		irr::video::SColor lockColor = BROWN;
		irr::video::SColor acidProofWallColor = LIGHTGREEN;
		irr::video::SColor wallShadowColor = BLACK;
		irr::video::SColor lockShadowColor = BLACK;
		irr::video::SColor acidProofWallShadowColor = BLACK;
		irr::core::position2d< irr::s32 > shadowOffset( 1, 1 );

		for( decltype( cols ) x = 0; x < cols; ++x ) {
			for( decltype( rows ) y = 0; y < rows; ++y ) {
				if( maze[ x ][ y ].topVisible ) {
					if( maze[ x ][ y ].getTop() == MazeCell::WALL ) {
						driver->draw2DLine( shadowOffset + irr::core::position2d< irr::s32 >( cellWidth * x, cellHeight * y ), shadowOffset + irr::core::position2d< irr::s32 >( cellWidth * ( x + 1 ), cellHeight * y ), wallShadowColor );
						driver->draw2DLine( irr::core::position2d< irr::s32 >( cellWidth * x, cellHeight * y ), irr::core::position2d< irr::s32 >( cellWidth * ( x + 1 ), cellHeight * y ), wallColor );
					} else if( maze[ x ][ y ].getTop() == MazeCell::ACIDPROOF ) {
						driver->draw2DLine( shadowOffset + irr::core::position2d< irr::s32 >( cellWidth * x, cellHeight * y ), shadowOffset + irr::core::position2d< irr::s32 >( cellWidth * ( x + 1 ), cellHeight * y ), acidProofWallShadowColor );
						driver->draw2DLine( irr::core::position2d< irr::s32 >( cellWidth * x, cellHeight * y ), irr::core::position2d< irr::s32 >( cellWidth * ( x + 1 ), cellHeight * y ), acidProofWallColor );
					} else if( maze[ x ][ y ].getTop() == MazeCell::LOCK ) {
						driver->draw2DLine( shadowOffset + irr::core::position2d< irr::s32 >( cellWidth * x, cellHeight * y ), shadowOffset + irr::core::position2d< irr::s32 >( cellWidth * ( x + 1 ), cellHeight * y ), lockShadowColor );
						driver->draw2DLine( irr::core::position2d< irr::s32 >( cellWidth * x, cellHeight * y ), irr::core::position2d< irr::s32 >( cellWidth * ( x + 1 ), cellHeight * y ), lockColor );
					}
				}
				
				if( maze[ x ][ y ].leftVisible ) {
					if( maze[ x ][ y ].getLeft() == MazeCell::WALL ) {
						driver->draw2DLine( shadowOffset + irr::core::position2d< irr::s32 >( cellWidth * x, cellHeight * y ), shadowOffset + irr::core::position2d< irr::s32 >( cellWidth * x, cellHeight * ( y + 1 ) ), wallShadowColor );
						driver->draw2DLine( irr::core::position2d< irr::s32 >( cellWidth * x, cellHeight * y ), irr::core::position2d< irr::s32 >( cellWidth * x, cellHeight * ( y + 1 ) ), wallColor );
					} else if( maze[ x ][ y ].getLeft() == MazeCell::ACIDPROOF ) {
						driver->draw2DLine( shadowOffset + irr::core::position2d< irr::s32 >( cellWidth * x, cellHeight * y ), shadowOffset + irr::core::position2d< irr::s32 >( cellWidth * x, cellHeight * ( y + 1 ) ), acidProofWallShadowColor );
						driver->draw2DLine( irr::core::position2d< irr::s32 >( cellWidth * x, cellHeight * y ), irr::core::position2d< irr::s32 >( cellWidth * x, cellHeight * ( y + 1 ) ), acidProofWallColor );
					} else if( maze[ x ][ y ].getLeft() == MazeCell::LOCK ) {
						driver->draw2DLine( shadowOffset + irr::core::position2d< irr::s32 >( cellWidth * x, cellHeight * y ), shadowOffset + irr::core::position2d< irr::s32 >( cellWidth * x, cellHeight * ( y + 1 ) ), lockShadowColor );
						driver->draw2DLine( irr::core::position2d< irr::s32 >( cellWidth * x, cellHeight * y ), irr::core::position2d< irr::s32 >( cellWidth * x, cellHeight * ( y + 1 ) ), lockColor );
					}
				}
				
				if( maze[ x ][ y ].rightVisible ) {
					//Only cells on the right or bottom edge of the maze should have anything other than NONE as right or bottom, and then it should only be a solid WALL
					if( maze[ x ][ y ].getRight() == MazeCell::ACIDPROOF ) {
						driver->draw2DLine( shadowOffset + irr::core::position2d< irr::s32 >( cellWidth * ( x + 1 ), cellHeight * y ), shadowOffset + irr::core::position2d< irr::s32 >( cellWidth * ( x + 1 ), cellHeight * ( y + 1 ) ), wallShadowColor );
						driver->draw2DLine( irr::core::position2d< irr::s32 >( cellWidth * ( x + 1 ), cellHeight * y ), irr::core::position2d< irr::s32 >( cellWidth * ( x + 1 ), cellHeight * ( y + 1 ) ), acidProofWallColor );
					}
				}
				if( maze[ x ][ y ].bottomVisible ) {	
					if( maze[ x ][ y ].getBottom() == MazeCell::ACIDPROOF ) {
						driver->draw2DLine( shadowOffset + irr::core::position2d< irr::s32 >( cellWidth * x, cellHeight * ( y + 1 ) ), shadowOffset + irr::core::position2d< irr::s32 >( cellWidth * ( x + 1 ), cellHeight * ( y + 1 ) ), wallShadowColor );
						driver->draw2DLine( irr::core::position2d< irr::s32 >( cellWidth * x, cellHeight * ( y + 1 ) ), irr::core::position2d< irr::s32 >( cellWidth * ( x + 1 ), cellHeight * ( y + 1 ) ), acidProofWallColor );
					}
				}
			}
		}
	} catch ( std::exception &e ) {
		std::wcerr << L"Error in MazeManager::draw(): " << e.what() << std::endl;
	}
}

irr::core::stringw MazeManager::getFileTypeExtension() {
	return fileTypeExtension;
}

irr::core::stringw MazeManager::getFileTypeName() {
	return fileTypeName;
}

bool MazeManager::loadFromFile() {
	try {
		return loadFromFile( L"default.maz" );
	} catch( std::exception &e ) {
		std::wcerr << L"Error in MazeManager::loadFromFile(): " << e.what() << std::endl;
		return false;
	}
}

bool MazeManager::loadFromFile( boost::filesystem::path src ) {
	try {
		//cppcheck-suppress duplicateIf
		if( mainGame == 0 or mainGame == NULL or mainGame == nullptr ) {
			throw( CustomException( L"setMainGame() has not been called yet." ) );
		}
		if( mainGame->getDebugStatus() ) {
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
			decltype( mainGame->getRandomSeed() ) newRandomSeed;
			file >> newRandomSeed;
			file.close();
			mainGame->newMaze( newRandomSeed );
			return true;
		} else {
			throw( CustomException( std::wstring( L"Cannot open file: \"" ) + src.wstring() + L"\"" ) );
		}
	} catch( const boost::filesystem::filesystem_error &e ) {
		std::wcerr << L"Boost Filesystem error in MazeManager::loadFromFile(): " << e.what() << std::endl;
	} catch( std::exception &e ) {
		std::wcerr << L"non-Boost-Filesystem error in MazeManager::loadFromFile(): " << e.what() << std::endl;
	}
	return false;
}

//Figures out which cells should be visible from the given position
void MazeManager::makeCellsVisible( uint_fast8_t x, uint_fast8_t y ) {
	if( hideUnseen ) { //No need to do anything if they're all visible anyway
		for( auto yprime = y; yprime <= y; --yprime ) { //When yprime wraps around, we're done
			maze[ x ][ yprime ].topVisible = true;
			if( maze[ x ][ yprime ].getTop() not_eq MazeCell::NONE ) {
				break;
			}
		}
		for( auto yprime = y + 1; yprime < rows; ++yprime ) {
			maze[ x ][ yprime ].topVisible = true;
			if( maze[ x ][ yprime ].getTop() not_eq MazeCell::NONE ) {
				break;
			}
		}
		for( auto xprime = x; xprime <= x; --xprime ) { //When xprime wraps around, we're done
			maze[ xprime ][ y ].leftVisible = true;
			if( maze[ xprime ][ y ].getLeft() not_eq MazeCell::NONE ) {
				break;
			}
		}
		for( auto xprime = x + 1; xprime < cols; ++xprime ) {
			maze[ xprime ][ y ].leftVisible = true;
			if( maze[ xprime ][ y ].getLeft() not_eq MazeCell::NONE ) {
				break;
			}
		}
	}
}

//Does everything involved in making the maze, calls other functions as needed.
void MazeManager::makeRandomLevel() {
	try {
		mainGame->drawAll();
		// Flawfinder: ignore
		//srand( mainGame->randomSeed ); //randomSeed is set either by resetThings() or by loadFromFile()
		{
			decltype( cols ) tempCols = mainGame->getRandomNumber() % 28 + 2; //I don't remember where I got the 28. The 2 is arbitrary so there's some minimum amount.
			decltype( rows ) tempRows = tempCols + ( mainGame->getRandomNumber() % 5 ); //Again, no idea where the 5 came from.
			newMaze( tempCols, tempRows );
		}
		mainGame->setLoadingPercentage( mainGame->getLoadingPercentage() + 1 );
		mainGame->drawAll();
		
		//Set whether the cells are visible. Those on the border are changed later.
		for( decltype( cols ) x = 0; x < cols; ++x ) {
			for( decltype( rows ) y = 0; y < rows; ++y ) {
				maze[ x ][ y ].topVisible = not hideUnseen;
				maze[ x ][ y ].leftVisible = not hideUnseen;
			}
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
		
		mainGame->setLoadingPercentage( mainGame->getLoadingPercentage() + 1 );
		mainGame->drawAll();

		for( decltype( settingsManager->numPlayers ) p = 0; p < settingsManager->numPlayers; ++p ) {
			mainGame->playerStart[ p ].reset();
		}
		
		{
			decltype( cols ) goalX = mainGame->getRandomNumber() % cols;
			decltype( rows ) goalY = mainGame->getRandomNumber() % rows;
			mainGame->goal.setX( goalX );
			mainGame->goal.setY( goalY );
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
		
		mainGame->setLoadingPercentage( mainGame->getLoadingPercentage() + 1 );
		mainGame->drawAll();
		
		//Add walls at maze borders
		for( decltype( cols ) x = 0; x < cols; ++x ) {
			maze[ x ][ 0 ].setOriginalTop( MazeCell::ACIDPROOF );
			maze[ x ][ 0 ].topVisible = true;
			maze[ x ][ rows-1 ].setOriginalBottom( MazeCell::ACIDPROOF );
			maze[ x ][ rows-1 ].bottomVisible = true;
		}

		for( decltype( rows ) y = 0; y < rows; ++y ) {
			maze[ 0 ][ y ].setOriginalLeft( MazeCell::ACIDPROOF );
			maze[ 0 ][ y ].leftVisible = true;
			maze[ cols-1 ][ y ].setOriginalRight( MazeCell::ACIDPROOF );
			maze[ cols-1 ][ y ].rightVisible = true;
		}
		
		mainGame->setLoadingPercentage( mainGame->getLoadingPercentage() + 1 );
		mainGame->drawAll();
		
		//Make MazeCell::isDeadEnd() work
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
		
		mainGame->setLoadingPercentage( mainGame->getLoadingPercentage() + 1 );
		mainGame->drawAll();

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
		
		mainGame->setLoadingPercentage( mainGame->getLoadingPercentage() + 1 );
		mainGame->drawAll();

		//Remove player starts from list of dead ends
		for( decltype( settingsManager->numPlayers ) p = 0; p < settingsManager->numPlayers; ++p ) {
			for( decltype( deadEndsX.size() ) i = 0; i < deadEndsX.size(); ++i ) {
				if( mainGame->playerStart.at( p ).getX() == deadEndsX.at( i ) and mainGame->playerStart.at( p ).getY() == deadEndsY.at( i ) ) {
					deadEndsX.erase( deadEndsX.begin() + i );
					deadEndsY.erase( deadEndsY.begin() + i );
				}
			}
		}
		
		mainGame->setLoadingPercentage( mainGame->getLoadingPercentage() + 1 );
		mainGame->drawAll();

		//Remove goal from list of dead ends
		for( decltype( deadEndsX.size() ) i = 0; i < deadEndsX.size(); ++i ) {
			if( mainGame->goal.getX() == deadEndsX.at( i ) and mainGame->goal.getY() == deadEndsY.at( i ) ) {
				deadEndsX.erase( deadEndsX.begin() + i );
				deadEndsY.erase( deadEndsY.begin() + i );
			}
		}
		
		if( cols > 0 ) { //Decide how many keys/locks to use (# of keys = # of locks)
			decltype( mainGame->numLocks ) temp = mainGame->getRandomNumber() % cols;
			if( deadEndsX.size() > 0 ) {
				temp = temp % deadEndsX.size();
			}
			mainGame->numLocks = temp;
		} else {
			mainGame->numLocks = 0;
		}

		//mainGame->numLocks = mainGame->getRandomNumber() % ( cols * rows ); //Uncomment this for a crazy number of keys!

		decltype( mainGame->numLocks ) numKeys = mainGame->numLocks;

		//Place keys in dead ends
		for( decltype( numKeys ) k = 0; k < numKeys; ++k ) {

			if( deadEndsX.size() == 0 ) {
				deadEndsX.push_back( mainGame->playerStart.at( 0 ).getX() );
				deadEndsY.push_back( mainGame->playerStart.at( 0 ).getY() );
			}
			
			{ //Pick one of the dead ends randomly.
				decltype( deadEndsX.size() ) chosen = mainGame->getRandomNumber() % deadEndsX.size();

				{ //Finally, create a key and put it there.
					Collectable temp;
					temp.setX( deadEndsX.at( chosen ) );
					temp.setY( deadEndsY.at( chosen ) );
					temp.setType( Collectable::KEY );
					temp.loadTexture( mainGame->device );
					mainGame->stuff.push_back( temp );
					if( mainGame->getDebugStatus() ) {
						std::wcout << L"Placing key at " << deadEndsX.at( chosen ) << L"," << deadEndsY.at( chosen ) << std::endl;
					}
				}

				//Remove chosen from the list of dead ends so no other keys go there
				deadEndsX.erase( deadEndsX.begin() + chosen );
				deadEndsY.erase( deadEndsY.begin() + chosen );
			}
		}
		
		mainGame->setLoadingPercentage( mainGame->getLoadingPercentage() + 1 );
		mainGame->drawAll();
		
		{
			uint_fast8_t InverseProbabilityOfAcid = std::min( ( uint_fast8_t ) UINT_FAST8_MAX, ( uint_fast8_t ) mainGame->getMaxRandomNumber() ); //Acid is supposed to be really rare. I call this inverse probability because the higher this number is, the less the probability is. 1 means total, 100% probability. Never set this to zero.
			
			if( mainGame->getDebugStatus() ) {
				InverseProbabilityOfAcid = 1; //If the game is being debugged, ensure the acid is always there - it may be what's being debugged. As Keith Curtis says in 'After the Software Wars', "if the code isn't executed, it probably doesn't work.".
			}
			if( mainGame->getRandomNumber() % InverseProbabilityOfAcid == 0 ) {
				if( deadEndsX.empty() ) { //If all the dead ends have been filled with other collectables
					Collectable temp;
					if( cols > 0 ) { //Clang's static analyzer thinks rows and cols may be zero
						temp.setX( mainGame->getRandomNumber() % cols );
					}
					if( rows > 0 ) {
						temp.setY( mainGame->getRandomNumber() % rows );
					}
					
					temp.setType( Collectable::ACID );
					temp.loadTexture( mainGame->device );
					mainGame->stuff.push_back( temp );
				} else {
					//Pick one of the dead ends randomly.
					decltype( deadEndsX.size() ) chosen = mainGame->getRandomNumber() % deadEndsX.size();

					{ //Finally, create an acid and put it there.
						Collectable temp;
						temp.setX( deadEndsX.at( chosen ) );
						temp.setY( deadEndsY.at( chosen ) );
						temp.setType( Collectable::ACID );
						temp.loadTexture( mainGame->device );
						mainGame->stuff.push_back( temp );
					}

					//Remove chosen from the list of dead ends so no other stuff goes there
					deadEndsX.erase( deadEndsX.begin() + chosen );
					deadEndsY.erase( deadEndsY.begin() + chosen );
				}
			}
		}

		for( decltype( settingsManager->numPlayers ) p = 0; p < settingsManager->numPlayers; ++p ) {
			mainGame->player[ p ].setPos( mainGame->playerStart[ p ].getX(), mainGame->playerStart[ p ].getY() );
		}

		if( mainGame->numLocks > 0 ) {
			//Place locks
			//Place first lock at the mainGame->goal
			if( maze[ mainGame->goal.getX() ][ mainGame->goal.getY() ].getTop() == MazeCell::NONE ) {
				maze[ mainGame->goal.getX() ][ mainGame->goal.getY() ].setOriginalTop( MazeCell::LOCK );
			} else if( maze[ mainGame->goal.getX() ][ mainGame->goal.getY() ].getLeft() == MazeCell::NONE ) {
				maze[ mainGame->goal.getX() ][ mainGame->goal.getY() ].setOriginalLeft( MazeCell::LOCK );
			} else if( maze[ mainGame->goal.getX() ][ mainGame->goal.getY() + 1 ].getTop() == MazeCell::NONE ) {
				maze[ mainGame->goal.getX() ][ mainGame->goal.getY() + 1 ].setOriginalTop( MazeCell::LOCK );
			} else if( maze[ mainGame->goal.getX() + 1 ][ mainGame->goal.getY() ].getLeft() == MazeCell::NONE ) {
				maze[ mainGame->goal.getX() + 1 ][ mainGame->goal.getY() ].setOriginalLeft( MazeCell::LOCK );
			}

			decltype( mainGame->numLocks ) numLocksPlaced = 1;

			while( mainGame->device->run() not_eq false and numLocksPlaced < mainGame->numLocks and mainGame->timer->getTime() < mainGame->timeStartedLoading + mainGame->loadingDelay and cols > 0 and rows > 0 ) {
				decltype( cols ) tempX = mainGame->getRandomNumber() % cols;
				decltype( rows ) tempY = mainGame->getRandomNumber() % rows;

				if( maze[ tempX ][ tempY ].getTop() == MazeCell::NONE ) {
					maze[ tempX ][ tempY ].setOriginalTop( MazeCell::LOCK );

					if( canGetToAllCollectables( mainGame->playerStart[ 0 ].getX(), mainGame->playerStart[ 0 ].getY() ) ) {
						numLocksPlaced += 1;
						if( mainGame->getDebugStatus() ) {
							std::wcout << L"Placed lock " << numLocksPlaced << L" at " << tempX << L"x" << tempY << std::endl;
						}
					} else {
						maze[ tempX ][ tempY ].setOriginalTop( MazeCell::NONE );
					}
				} else if( maze[ tempX ][ tempY ].getLeft() == MazeCell::NONE ) {
					maze[ tempX ][ tempY ].setOriginalLeft( MazeCell::LOCK );

					if( canGetToAllCollectables( mainGame->playerStart[ 0 ].getX(), mainGame->playerStart[ 0 ].getY() ) ) {
						numLocksPlaced += 1;
						if( mainGame->getDebugStatus() ) {
							std::wcout << L"Placed lock " << numLocksPlaced << L" at " << tempX << L"x" << tempY << std::endl;
						}
					} else {
						maze[ tempX ][ tempY ].setOriginalLeft( MazeCell::NONE );
					}
				}
			}

			mainGame->timer->stop();
			mainGame->timer->setTime( 0 );
			if( mainGame->getDebugStatus() ) {
				std::wcout << L"numLocksPlaced: " << numLocksPlaced << L"\tnumLocks: " << mainGame->numLocks << std::endl;
			}

			if( numLocksPlaced < mainGame->numLocks ) {
				decltype( numLocksPlaced ) keysToRemove = mainGame->numLocks - numLocksPlaced;

				for( decltype( mainGame->stuff.size() ) i = 0; ( i < mainGame->stuff.size() and keysToRemove > 0 ); ++i ) {
					if( mainGame->getDebugStatus() ) {
						std::wcout << L"keysToRemove: " << keysToRemove << std::endl;
					}

					if( mainGame->stuff.at( i ).getType() == Collectable::KEY ) {
						mainGame->stuff.erase( mainGame->stuff.begin() + i );
						i = 0;
						keysToRemove -= 1;
					}
				}
			}

			mainGame->numLocks = numLocksPlaced;
		}
		
		mainGame->setLoadingPercentage( mainGame->getLoadingPercentage() + 1 );
		mainGame->drawAll();

		for( decltype( cols ) x = 0; x < cols; ++x ) {
			for( decltype( rows ) y = 0; y < rows; ++y ) {
				maze[ x ][ y ].visited = false;
			}
		}

		for( decltype( settingsManager->numPlayers ) p = 0; p < settingsManager->numPlayers; ++p ) {
			maze[ mainGame->playerStart[ p ].getX() ][ mainGame->playerStart[ p ].getY() ].visited = true;
			maze[ mainGame->playerStart[ p ].getX() ][ mainGame->playerStart[ p ].getY() ].setVisitorColor( mainGame->player[ p ].getColorTwo() );
			makeCellsVisible( mainGame->playerStart[ p ].getX(), mainGame->playerStart[ p ].getY() );
		}
		
		mainGame->setLoadingPercentage( mainGame->getLoadingPercentage() + 1 );
		mainGame->drawAll();
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
		mainGame = nullptr;
		StringConverter sc;
		fileTypeName = sc.toIrrlichtStringW( PACKAGE_NAME );
		fileTypeName.append( L" maze" );
		fileTypeExtension = fileTypeName.subString( 0, 1 ) + L"maze"; //Irrlicht 1.8 or later can use fileTypeName.subString( 0, 1, true ) + L"maze";
		fileTypeExtension.make_lower(); //This line is not necessary in Irrlicht 1.8+ because subString() works differently.
		hideUnseen = false;
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
		mainGame->setLoadingPercentage( mainGame->getLoadingPercentage() + ( 90.0f / ( cols * rows ) ) ); //I figure this recursion takes up about 90% of loading time. That's not based on any measurements, it's just a guess.
		//At one point I included the following line because I hoped to show the maze as it was being generated. Might still add that as an option. Now they're there so that the loading screen gets drawn and lasts long enough to read it.
		mainGame->drawAll();
		
		maze[ x ][ y ].visited = true;
		maze[ x ][ y ].id = numSoFar;
		
		for( decltype( settingsManager->numPlayers ) p = 0; p < settingsManager->numPlayers; ++p ) {
			if( depth >= mainGame->playerStart[ p ].distanceFromExit ) {
				mainGame->playerStart[ p ].setPos( x, y );
				mainGame->playerStart[ p ].distanceFromExit = depth;
			}
		}
		
		bool keepGoing = true;
		
		while( keepGoing ) {
			numSoFar += 1;
			
			switch( mainGame->getRandomNumber() % 4 ) { //4 = number of directions (up, down, left, right)
				case 0: //Left
					
					if( x > 0 and maze[ x-1 ][ y ].visited == false ) {
						maze[ x ][ y ].setOriginalLeft( MazeCell::NONE );
						
						recurseRandom( x - 1, y, depth + 1, numSoFar );
					}
					
					break;
				
				case 1: //Right
					
					if( x < cols - 1 and maze[ x+1 ][ y ].visited == false ) {
						maze[ x+1 ][ y ].setOriginalLeft( MazeCell::NONE );
						
						recurseRandom( x + 1, y, depth + 1, numSoFar );
					}
					
					break;
				
				case 2: //Up
					
					if( y > 0 and maze[ x ][ y-1 ].visited == false ) {
						maze[ x ][ y ].setOriginalTop( MazeCell::NONE );
						
						recurseRandom( x, y - 1, depth + 1, numSoFar );
					}
					
					break;
				
				case 3: //Down
					
					if( y < rows - 1 and maze[ x ][ y+1 ].visited == false ) {
						maze[ x ][ y+1 ].setOriginalTop( MazeCell::NONE );
						
						recurseRandom( x, y + 1, depth + 1, numSoFar );
					}
					
					break;
			}
			
			//If we've reached a dead end, don't keep going. Otherwise do.
			keepGoing = false;
			if( ( x > 0 and maze[ x - 1 ][ y ].visited == false )
					or ( x < cols - 1 and maze[ x + 1 ][ y ].visited == false )
					or ( y > 0 and maze[ x ][ y - 1 ].visited == false )
					or ( y < rows - 1 and maze[ x ][ y + 1 ].visited == false )
			  ) {
				keepGoing = true;
			}
		}
	} catch ( std::exception &e ) {
		std::wcerr << L"Error in MazeManager::recurseRandom(): " << e.what() << std::endl;
	}
}

bool MazeManager::saveToFile( boost::filesystem::path dest ) {
	try {
		
		{ //Append the desired extension to the file name if it's not already present.
			std::wstring destExtension = dest.extension().wstring();
			destExtension.erase( destExtension.begin() ); //The first character is the '.' which we don't include in fileTypeExtension
			if( not fileTypeExtension.equals_ignore_case( mainGame->stringConverter.toIrrlichtStringW( destExtension ) ) ) {
				dest += L".";
				dest += mainGame->stringConverter.toStdWString( fileTypeExtension );
			}
		}
		
		if( is_directory( dest ) ) {
			throw( CustomException( std::wstring( L"Directory specified, file needed: " ) + dest.wstring() ) );
		}
		
		boost::filesystem::wofstream file; //Identical to a standard C++ wofstream, except it takes Boost paths
		file.open( dest, boost::filesystem::wofstream::binary bitor boost::filesystem::wofstream::trunc );
		
		if( file.is_open() ) {
			auto newRandomSeed = mainGame->getRandomSeed();
			file << newRandomSeed;
			mainGame->setRandomSeed( newRandomSeed );
			/*auto rs = mainGame->randomSeed;
			file.write( reinterpret_cast<boost::filesystem::wofstream::char_type *>( &rs ), sizeof( rs ) / sizeof( boost::filesystem::wofstream::char_type ) );*/
			file.close();
			irr::core::stringw message( L"This maze has been saved to the file " );
			message += mainGame->stringConverter.toIrrlichtStringW( dest.wstring() );
			mainGame->gui->addMessageBox( L"Maze saved", mainGame->stringConverter.toStdWString( message ).c_str() ); //stringConverter.toWCharArray( message ) );
		} else {
			irr::core::stringw message( L"Cannot save to file " );
			message += mainGame->stringConverter.toIrrlichtStringW( dest.wstring() );
			std::wcerr << mainGame->stringConverter.toStdWString( message ) << std::endl; //stringConverter.toWCharArray( message ) << std::endl;
			mainGame->gui->addMessageBox( L"Maze NOT saved", mainGame->stringConverter.toStdWString( message ).c_str() ); //stringConverter.toWCharArray( message ) );
		}

		return true;
	} catch( const boost::filesystem::filesystem_error &e ) {
		std::wcerr << L"Boost Filesystem error in MazeManager::saveToFile(): " << e.what() << std::endl;
		return false;
	} catch( std::exception &e ) {
		std::wcerr << L"non-Boost-Filesystem error in MazeManager::saveToFile(): " << e.what() << std::endl;
		return false;
	}
}

void MazeManager::setPointers( MainGame* newMainGame, SettingsManager* newSettingsManager ){
	try {
		mainGame = newMainGame;
		settingsManager = newSettingsManager;
	} catch ( std::exception &e ) {
		std::wcerr << L"Error in MazeManager::setMainGame(): " << e.what() << std::endl;
	}
}
