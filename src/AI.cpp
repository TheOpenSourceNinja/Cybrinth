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
	gm = newGM;
}

void AI::allKeysFound() { //Right now it simply clears pathTaken and cellsVisited, effectively starting the maze exploration over. Definitely a kluge, but it works.
	pathTaken.clear();
	cellsVisited.clear();
	/*for( size_t l = 0; l < lockedCells.size(); l++ ) {
		for( size_t c = 0; c < cellsVisited.size(); c++ ) {
			if( lockedCells[ l ] == cellsVisited[ c ] ) {
				cellsVisited.erase( cellsVisited.begin() + c );
			}
		}
	}*/
}

bool AI::alreadyVisited( core::position2d< u8 > position ) {
	bool result = false;

	size_t i = 0;
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
	Player* player = gm->getPlayer( controlsPlayer );
	u8 playerX = player->getX();
	u8 playerY = player->getY();
	core::position2d< u8 > currentPosition( playerX, playerY );

	if( pathTaken.size() == 0 ) { //Ensures that the player's start position is marked as visited
		pathTaken.push_back( currentPosition );
	}

	if( !alreadyVisited( currentPosition ) ) {
		cellsVisited.push_back( currentPosition );
	}

	vector<char> possibleDirections;
	if( !( playerX == gm->goal.getX() && playerY == gm->goal.getY() ) ) {
		if( maze[ playerX ][ playerY ].hasLock() ) {
			lockedCells.push_back( currentPosition );
		}
		if( playerX < ( cols - 1 ) && maze[ playerX + 1 ][ playerY ].hasLock() ) {
			lockedCells.push_back( core::position2d< u8 >( playerX + 1, playerY ) );
		}
		if( playerY < ( rows - 1 ) && maze[ playerX ][ playerY + 1 ].hasLock() ) {
			lockedCells.push_back( core::position2d< u8 >( playerX, playerY + 1 ) );
		}

		if( playerY > 0 && maze[ playerX ][ playerY ].getTop() == '0' && !alreadyVisited( core::position2d< u8 >( playerX, playerY - 1 ) ) ) {
			possibleDirections.push_back('u');
		}
		if( playerX > 0 && maze[ playerX ][ playerY ].getLeft() == '0' && !alreadyVisited( core::position2d< u8 >( playerX - 1, playerY ) ) ) {
			possibleDirections.push_back('l');
		}
		if( playerY < (rows - 1) && maze[ playerX ][ playerY + 1 ].getTop() == '0' && !alreadyVisited( core::position2d< u8 >( playerX, playerY + 1 ) ) ) {
			possibleDirections.push_back('d');
		}
		if( playerX < (cols - 1) && maze[ playerX + 1 ][ playerY ].getLeft() == '0' && !alreadyVisited( core::position2d< u8 >( playerX + 1, playerY ) ) ) {
			possibleDirections.push_back('r');
		}
	}

	//Go back to previous position
	if( possibleDirections.size() == 0 && pathTaken.size() != 0 && !( playerX == gm->goal.getX() && playerY == gm->goal.getY() ) ) {
		pathTaken.pop_back();
		core::position2d< u8 > oldPosition = pathTaken.back();
		if( oldPosition.X < playerX ) {
			gm->movePlayerOnX( controlsPlayer, -1 );
		} else if( oldPosition.X > playerX ) {
			gm->movePlayerOnX( controlsPlayer, 1 );
		} else if( oldPosition.Y < playerY ) {
			gm->movePlayerOnY( controlsPlayer, -1 );
		} else { //if( oldPosition.Y > playerY ) {
			gm->movePlayerOnY( controlsPlayer, 1 );
		}
	} else if ( !( playerX == gm->goal.getX() && playerY == gm->goal.getY() ) ) {
		u8 choiceNum = rand() % possibleDirections.size();
		char choice = possibleDirections.at( choiceNum );
		switch( choice ) {
			case 'u': {
				pathTaken.push_back( core::position2d< u8 >( playerX, playerY - 1 ) );
				gm->movePlayerOnY( controlsPlayer, -1 );
			} break;
			case 'd': {
				pathTaken.push_back( core::position2d< u8 >( playerX, playerY + 1 ) );
				gm->movePlayerOnY( controlsPlayer, 1 );
			} break;
			case 'l': {
				pathTaken.push_back( core::position2d< u8 >( playerX - 1, playerY ) );
				gm->movePlayerOnX( controlsPlayer, -1 );
			} break;
			case 'r': {
				pathTaken.push_back( core::position2d< u8 >( playerX + 1, playerY ) );
				gm->movePlayerOnX( controlsPlayer, 1 );
			} break;
		}
	}
}

AI::~AI() {
	//dtor
}
