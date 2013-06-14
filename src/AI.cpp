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
#include "Player.h"

AI::AI() {
	setPlayer( 0 );
	setup( NULL, 0, 0, NULL );
	lastTimeMoved = 0;
	movementDelay = 300; //TODO: Allow custom AI movement delays for increased/decreased challenge
}

AI::AI( u8 newPlayer ) {
	setPlayer( newPlayer );
	setup( NULL, 0, 0, NULL );
	lastTimeMoved = 0;
	movementDelay = 300;
}

void AI::setPlayer( u8 newPlayer ) {
	controlsPlayer = newPlayer;
}

u8 AI::getPlayer() {
	return controlsPlayer;
}

void AI::setup( MazeCell ** newMaze, u8 newCols, u8 newRows, GameManager *newGM ) {
	maze = newMaze;
	cols = newCols;
	rows = newRows;
	pathTaken.clear();
	cellsVisited.clear();
	
	for( std::vector< core::dimension2d< u8 > >::size_type i = 0; i < pathsToLockedCells.size(); i++ ) {
		pathsToLockedCells[ i ].clear();
	}
	pathsToLockedCells.clear();
	gm = newGM;
}

void AI::allKeysFound() { //Right now it simply clears pathTaken and cellsVisited, effectively starting the maze exploration over. Definitely a kluge, but it works.
	//pathTaken.clear();
	//cellsVisited.clear();
	if( gm->debug ) {
		std::wcout << L"All keys found" << std::endl;
	}
	core::position2d< u8 > currentPosition( gm->getPlayer( controlsPlayer )->getX(), gm->getPlayer( controlsPlayer )->getY() );
	
	for( std::vector< std::vector< core::position2d< u8 > > >::size_type o = 0; o < pathsToLockedCells.size(); o++ ) {
		
		for( std::vector< core::position2d< u8 > >::size_type i = 0; i < pathsToLockedCells[ o ].size(); i++ ) {

			std::vector< core::position2d< u8 > >::size_type j = 0;
			while( j < cellsVisited.size() ) {
				
				if( ( cellsVisited[ j ].X == pathsToLockedCells[ o ][ i ].X && cellsVisited[ j ].Y == pathsToLockedCells[ o ][ i ].Y ) ) {
					if( gm->debug ) {
						gm->maze[cellsVisited[ j ].X][cellsVisited[ j ].Y].visited = false; //So we can infer the path from AI to locks
					}
					cellsVisited.erase( cellsVisited.begin() + j );
					j--;
				}
				j++;
			}
		}
		
		//Reduce memory usage: We're done with these now, so clear them.
		//pathsToLockedCells[ o ].clear();
		//pathsToLockedCells[ o ].shrink_to_fit(); C++11 not GCC's default yet
		std::vector< core::position2d< u8 > >().swap( pathsToLockedCells[ o ] );
	}
	//Reduce memory usage: We're done with these now, so clear them.
	//pathsToLockedCells.clear();
	//pathsToLockedCells.shrink_to_fit(); C++11 not GCC's default yet
	std::vector< std::vector< core::position2d< u8 > > >().swap( pathsToLockedCells );
}

bool AI::alreadyVisited( core::position2d< u8 > position ) {
	bool result = false;

	std::vector< core::position2d< u8 > >::size_type i = 0;
	while( i < cellsVisited.size() && result != true ) {
		if( cellsVisited[ i ].X == position.X && cellsVisited[ i ].Y == position.Y ) {
			result = true;
		}
		i++;
	}

	return result;
}

void AI::move() {
	lastTimeMoved = gm->timer->getRealTime();
	//Player* player = gm->getPlayer( controlsPlayer );
	core::position2d< u8 > currentPosition( gm->getPlayer( controlsPlayer )->getX(), gm->getPlayer( controlsPlayer )->getY() );

	if( pathTaken.size() == 0 ) { //Ensures that the player's start position is marked as visited
		pathTaken.push_back( currentPosition );
	}

	if( !alreadyVisited( currentPosition ) ) {
		cellsVisited.push_back( currentPosition );
	}

	std::vector<char> possibleDirections;
	if( !( currentPosition.X == gm->goal.getX() && currentPosition.Y == gm->goal.getY() ) ) {
		if( maze[ currentPosition.X ][ currentPosition.Y ].hasLock() ) {
			pathsToLockedCells.push_back( vector< core::position2d< u8 > >() );
			pathsToLockedCells.back().push_back( currentPosition );
		}
		if( currentPosition.X < ( cols - 1 ) && maze[ currentPosition.X + 1 ][ currentPosition.Y ].hasLock() ) {
			pathsToLockedCells.push_back( vector< core::position2d< u8 > >() );
			pathsToLockedCells.back().push_back( core::position2d< u8 >( currentPosition.X + 1, currentPosition.Y ) );
		}
		if( currentPosition.Y < ( rows - 1 ) && maze[ currentPosition.X ][ currentPosition.Y + 1 ].hasLock() ) {
			pathsToLockedCells.push_back( vector< core::position2d< u8 > >() );
			pathsToLockedCells.back().push_back( core::position2d< u8 >( currentPosition.X, currentPosition.Y + 1 ) );
		}

		if( currentPosition.Y > 0 && maze[ currentPosition.X ][ currentPosition.Y ].getTop() == '0' && !alreadyVisited( core::position2d< u8 >( currentPosition.X, currentPosition.Y - 1 ) ) ) {
			possibleDirections.push_back('u');
		}
		if( currentPosition.X > 0 && maze[ currentPosition.X ][ currentPosition.Y ].getLeft() == '0' && !alreadyVisited( core::position2d< u8 >( currentPosition.X - 1, currentPosition.Y ) ) ) {
			possibleDirections.push_back('l');
		}
		if( currentPosition.Y < (rows - 1) && maze[ currentPosition.X ][ currentPosition.Y + 1 ].getTop() == '0' && !alreadyVisited( core::position2d< u8 >( currentPosition.X, currentPosition.Y + 1 ) ) ) {
			possibleDirections.push_back('d');
		}
		if( currentPosition.X < (cols - 1) && maze[ currentPosition.X + 1 ][ currentPosition.Y ].getLeft() == '0' && !alreadyVisited( core::position2d< u8 >( currentPosition.X + 1, currentPosition.Y ) ) ) {
			possibleDirections.push_back('r');
		}
	}

	//Go back to previous position
	if( possibleDirections.size() == 0 && pathTaken.size() != 0 && !( currentPosition.X == gm->goal.getX() && currentPosition.Y == gm->goal.getY() ) ) {
		pathTaken.pop_back();
		core::position2d< u8 > oldPosition = pathTaken.back();
		for( std::vector< std::vector< core::dimension2d< u8 > > >::size_type o = 0; o < pathsToLockedCells.size(); o++ ) {
			pathsToLockedCells[ o ].push_back( oldPosition );
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
	} else if ( !( currentPosition.X == gm->goal.getX() && currentPosition.Y == gm->goal.getY() ) ) { //Go to next position
		u8 choiceNum = rand() % possibleDirections.size();
		char choice = possibleDirections.at( choiceNum );
		switch( choice ) {
			case 'u': {
				core::position2d< u8 > position( currentPosition.X, currentPosition.Y - 1 );
				pathTaken.push_back( position );
				for( std::vector< std::vector< core::dimension2d< u8 > > >::size_type o = 0; o < pathsToLockedCells.size(); o++ ) {
					//for( std::vector< core::dimension2d< u8 > >::size_type i = 0; i < pathsToLockedCells[ o ].size(); i++ ) {
						pathsToLockedCells[ o ].push_back( position );
					//}
				}
				gm->movePlayerOnY( controlsPlayer, -1 );
			} break;
			case 'd': {
				core::position2d< u8 > position( currentPosition.X, currentPosition.Y + 1 );
				pathTaken.push_back( position );
				for( std::vector< std::vector< core::dimension2d< u8 > > >::size_type o = 0; o < pathsToLockedCells.size(); o++ ) {
					//for( std::vector< core::dimension2d< u8 > >::size_type i = 0; i < pathsToLockedCells[ o ].size(); i++ ) {
						pathsToLockedCells[ o ].push_back( position );
					//}
				}
				gm->movePlayerOnY( controlsPlayer, 1 );
			} break;
			case 'l': {
				core::position2d< u8 > position( currentPosition.X - 1, currentPosition.Y );
				pathTaken.push_back( position );
				for( std::vector< std::vector< core::dimension2d< u8 > > >::size_type o = 0; o < pathsToLockedCells.size(); o++ ) {
					//for( std::vector< core::dimension2d< u8 > >::size_type i = 0; i < pathsToLockedCells[ o ].size(); i++ ) {
						pathsToLockedCells[ o ].push_back( position );
					//}
				}
				gm->movePlayerOnX( controlsPlayer, -1 );
			} break;
			case 'r': {
				core::position2d< u8 > position( currentPosition.X + 1, currentPosition.Y );
				pathTaken.push_back( position );
				for( std::vector< std::vector< core::dimension2d< u8 > > >::size_type o = 0; o < pathsToLockedCells.size(); o++ ) {
					//for( std::vector< core::dimension2d< u8 > >::size_type i = 0; i < pathsToLockedCells[ o ].size(); i++ ) {
						pathsToLockedCells[ o ].push_back( position );
					//}
				}
				gm->movePlayerOnX( controlsPlayer, 1 );
			} break;
		}
	}
}

AI::~AI() {
	//dtor
}
