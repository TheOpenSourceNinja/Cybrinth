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

#include "AI.h"
#include "GameManager.h"
#include "MazeManager.h"
#include "Player.h"

AI::AI() {
	try {
		setPlayer( 0 );
		setup( NULL, 0, 0, NULL );
		lastTimeMoved = 0;
		movementDelay = 300; //TODO: Allow custom AI movement delays for increased/decreased challenge
	} catch( std::exception e ) {
		std::wcerr << L"Error in AI::AI(): " << e.what() << std::endl;
	}
}

void AI::setPlayer( uint_least8_t newPlayer ) {
	try {
		controlsPlayer = newPlayer;
	} catch( std::exception e ) {
		std::wcerr << L"Error in AI::setPlayer(): " << e.what() << std::endl;
	}
}

uint_least8_t AI::getPlayer() {
	try {
		return controlsPlayer;
	} catch( std::exception e ) {
		std::wcerr << L"Error in AI::AI(): " << e.what() << std::endl;
		return 0; //This should never be reached. If it is, good luck to the real player 0.
	}
}

void AI::setup( MazeCell ** newMaze, uint_least8_t newCols, uint_least8_t newRows, GameManager *newGM ) {
	try {
		maze = newMaze;
		cols = newCols;
		rows = newRows;
		pathTaken.clear();
		cellsVisited.clear();

		for( std::vector< core::dimension2d< uint_least8_t > >::size_type i = 0; i < pathsToLockedCells.size(); i++ ) {
			pathsToLockedCells.at( i ).clear();
		}
		pathsToLockedCells.clear();
		gm = newGM;
	} catch( std::exception e ) {
		std::wcerr << L"Error in AI::setup(): " << e.what() << std::endl;
	}
}

void AI::allKeysFound() { //Makes the bot 'forget' that it has visited certain maze cells
	try {
		//pathTaken.clear();
		//cellsVisited.clear();
		if( gm->getDebugStatus() ) {
			std::wcout << L"Bot " << controlsPlayer << L" acknowledging all keys found" << std::endl;
		}
		core::position2d< uint_least8_t > currentPosition( gm->getPlayer( controlsPlayer )->getX(), gm->getPlayer( controlsPlayer )->getY() );

		for( std::vector< std::vector< core::position2d< uint_least8_t > > >::size_type o = 0; o < pathsToLockedCells.size(); o++ ) {

			for( std::vector< core::position2d< uint_least8_t > >::size_type i = 0; i < pathsToLockedCells.at( o ).size(); i++ ) {

				std::vector< core::position2d< uint_least8_t > >::size_type j = 0;
				while( j < cellsVisited.size() ) {

					if( ( cellsVisited.at( j ).X == pathsToLockedCells.at( o )[ i ].X && cellsVisited.at( j ).Y == pathsToLockedCells.at( o )[ i ].Y ) ) {
						cellsVisited.erase( cellsVisited.begin() + j );
						j--;
					}
					j++;
				}
			}

			//Reduce memory usage: We're done with these now, so clear them.
			pathsToLockedCells.at( o ).clear();
			pathsToLockedCells.at( o ).shrink_to_fit(); //If your compilter doesn't support C++11 or later, comment this line and uncomment the next
			//std::vector< core::position2d< uint_least8_t > >().swap( pathsToLockedCells.at( o ) );
		}
		//Reduce memory usage: We're done with these now, so clear them.
		pathsToLockedCells.clear();
		pathsToLockedCells.shrink_to_fit(); //If your compilter doesn't support C++11 or later, comment this line and uncomment the next
		//std::vector< std::vector< core::position2d< uint_least8_t > > >().swap( pathsToLockedCells );
	} catch( std::exception e ) {
		std::wcerr << L"Error in AI::allKeysFound(): " << e.what() << std::endl;
	}
}

bool AI::alreadyVisited( core::position2d< uint_least8_t > position ) {
	try {
		bool result = false;

		std::vector< core::position2d< uint_least8_t > >::size_type i = 0;
		while( i < cellsVisited.size() && result != true ) {
			if( cellsVisited.at( i ).X == position.X && cellsVisited.at( i ).Y == position.Y ) {
				result = true;
			}
			i++;
		}
		return result;
	} catch( std::exception e ) {
		std::wcerr << L"Error in AI::alreadyVisited(): " << e.what() << std::endl;
		return false;
	}
}

void AI::move() {
	try {
		lastTimeMoved = gm->timer->getRealTime();
		core::position2d< uint_least8_t > currentPosition( gm->getPlayer( controlsPlayer )->getX(), gm->getPlayer( controlsPlayer )->getY() );

		if( pathTaken.size() == 0 ) { //Ensures that the player's start position is marked as visited
			pathTaken.push_back( currentPosition );
		}

		if( !alreadyVisited( currentPosition ) ) {
			cellsVisited.push_back( currentPosition );
		}

		std::vector< char > possibleDirections;
		if( !( currentPosition.X == gm->getGoal()->getX() && currentPosition.Y == gm->getGoal()->getY() ) ) {

			//Check for locks
			if( maze[ currentPosition.X ][ currentPosition.Y ].hasLock() ) {
				pathsToLockedCells.push_back( std::vector< core::position2d< uint_least8_t > >() );
				pathsToLockedCells.back().push_back( currentPosition );
			}
			if( currentPosition.X < ( cols - 1 ) && maze[ currentPosition.X + 1 ][ currentPosition.Y ].hasLeftLock() ) {
				pathsToLockedCells.push_back( std::vector< core::position2d< uint_least8_t > >() );
				pathsToLockedCells.back().push_back( currentPosition );
				pathsToLockedCells.back().push_back( core::position2d< uint_least8_t >( currentPosition.X + 1, currentPosition.Y ) );
			}
			if( currentPosition.Y < ( rows - 1 ) && maze[ currentPosition.X ][ currentPosition.Y + 1 ].hasTopLock() ) {
				pathsToLockedCells.push_back( std::vector< core::position2d< uint_least8_t > >() );
				pathsToLockedCells.back().push_back( currentPosition );
				pathsToLockedCells.back().push_back( core::position2d< uint_least8_t >( currentPosition.X, currentPosition.Y + 1 ) );
			}

			//See which direction(s) the bot can move
			if( currentPosition.Y > 0 && maze[ currentPosition.X ][ currentPosition.Y ].getTop() == 'n' && !alreadyVisited( core::position2d< uint_least8_t >( currentPosition.X, currentPosition.Y - 1 ) ) ) {
				possibleDirections.push_back('u');
			}
			if( currentPosition.X > 0 && maze[ currentPosition.X ][ currentPosition.Y ].getLeft() == 'n' && !alreadyVisited( core::position2d< uint_least8_t >( currentPosition.X - 1, currentPosition.Y ) ) ) {
				possibleDirections.push_back('l');
			}
			if( currentPosition.Y < (rows - 1) && maze[ currentPosition.X ][ currentPosition.Y + 1 ].getTop() == 'n' && !alreadyVisited( core::position2d< uint_least8_t >( currentPosition.X, currentPosition.Y + 1 ) ) ) {
				possibleDirections.push_back('d');
			}
			if( currentPosition.X < (cols - 1) && maze[ currentPosition.X + 1 ][ currentPosition.Y ].getLeft() == 'n' && !alreadyVisited( core::position2d< uint_least8_t >( currentPosition.X + 1, currentPosition.Y ) ) ) {
				possibleDirections.push_back('r');
			}
		}

		//If we can't go anywhere new, go back to previous position
		if( possibleDirections.size() == 0 && pathTaken.size() != 0 && !( currentPosition.X == gm->getGoal()->getX() && currentPosition.Y == gm->getGoal()->getY() ) ) {
			pathTaken.pop_back();
			core::position2d< uint_least8_t > oldPosition = pathTaken.back();
			for( std::vector< std::vector< core::dimension2d< uint_least8_t > > >::size_type o = 0; o < pathsToLockedCells.size(); o++ ) {
				if( pathsToLockedCells.at( o ).back() != oldPosition ) {
					pathsToLockedCells.at( o ).push_back( oldPosition );
				}
			}
			if( oldPosition.X < currentPosition.X ) {
				gm->movePlayerOnX( controlsPlayer, -1 );
			} else if( oldPosition.X > currentPosition.X ) {
				gm->movePlayerOnX( controlsPlayer, 1 );
			} else if( oldPosition.Y < currentPosition.Y ) {
				gm->movePlayerOnY( controlsPlayer, -1 );
			} else { //if( oldPosition.Y > currentPosition.Y ) {
				gm->movePlayerOnY( controlsPlayer, 1 );
			}
		} else if ( !( currentPosition.X == gm->getGoal()->getX() && currentPosition.Y == gm->getGoal()->getY() ) ) { //Go to next position
			//uint_fast8_t choiceNum = rand() % possibleDirections.size();
			char choice = possibleDirections.at( rand() % possibleDirections.size() );
			switch( choice ) {
				case 'u': {
					core::position2d< uint_least8_t > position( currentPosition.X, currentPosition.Y - 1 );
					pathTaken.push_back( position );
					for( std::vector< std::vector< core::dimension2d< uint_least8_t > > >::size_type o = 0; o < pathsToLockedCells.size(); o++ ) {
						//for( std::vector< core::dimension2d< uint_least8_t > >::size_type i = 0; i < pathsToLockedCells.at( o ).size(); i++ ) {
							pathsToLockedCells.at( o ).push_back( position );
						//}
					}
					gm->movePlayerOnY( controlsPlayer, -1 );
				} break;
				case 'd': {
					core::position2d< uint_least8_t > position( currentPosition.X, currentPosition.Y + 1 );
					pathTaken.push_back( position );
					for( std::vector< std::vector< core::dimension2d< uint_least8_t > > >::size_type o = 0; o < pathsToLockedCells.size(); o++ ) {
						//for( std::vector< core::dimension2d< uint_least8_t > >::size_type i = 0; i < pathsToLockedCells.at( o ).size(); i++ ) {
							pathsToLockedCells.at( o ).push_back( position );
						//}
					}
					gm->movePlayerOnY( controlsPlayer, 1 );
				} break;
				case 'l': {
					core::position2d< uint_least8_t > position( currentPosition.X - 1, currentPosition.Y );
					pathTaken.push_back( position );
					for( std::vector< std::vector< core::dimension2d< uint_least8_t > > >::size_type o = 0; o < pathsToLockedCells.size(); o++ ) {
						//for( std::vector< core::dimension2d< uint_least8_t > >::size_type i = 0; i < pathsToLockedCells.at( o ).size(); i++ ) {
							pathsToLockedCells.at( o ).push_back( position );
						//}
					}
					gm->movePlayerOnX( controlsPlayer, -1 );
				} break;
				case 'r': {
					core::position2d< uint_least8_t > position( currentPosition.X + 1, currentPosition.Y );
					pathTaken.push_back( position );
					for( std::vector< std::vector< core::dimension2d< uint_least8_t > > >::size_type o = 0; o < pathsToLockedCells.size(); o++ ) {
						//for( std::vector< core::dimension2d< uint_least8_t > >::size_type i = 0; i < pathsToLockedCells.at( o ).size(); i++ ) {
							pathsToLockedCells.at( o ).push_back( position );
						//}
					}
					gm->movePlayerOnX( controlsPlayer, 1 );
				} break;
			}
		}
	} catch ( std::exception e ) {
		std::wcerr << L"Error in AI::move(): " << e.what() << std::endl;
	}
}

AI::~AI() {
	try {
	} catch ( std::exception e ) {
		std::wcerr << L"Error in AI::~AI(): " << e.what() << std::endl;
	}
}
