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

#ifndef AI_H
#define AI_H

#include <irrlicht.h>
#include <vector>
#include "MazeCell.h"

//#include "GameManager.h"
class GameManager; //Avoids circular dependency

class AI {
	public:
		/** Default constructor */
		AI();
		/** Default destructor */
		virtual ~AI();
		void setPlayer( uint8_t newPlayer );
		void setup( MazeCell ** newMaze, uint8_t newCols, uint8_t newRows,  GameManager * newGM ); //Provides the AI with whatever info it needs to work.
		void move(); //Needs to call GameManager's movePlayerOnX and movePlayerOnY functions.
		void allKeysFound();
		uint32_t lastTimeMoved;
		uint8_t getPlayer();
		uint32_t movementDelay; //How long to delay between movements
	protected:
	private:
		GameManager * gm;
		uint8_t controlsPlayer;
		std::vector< irr::core::position2d< uint8_t > > cellsVisited;
		std::vector< irr::core::position2d< uint8_t > > pathTaken;
		std::vector< std::vector< irr::core::position2d< uint8_t > > > pathsToLockedCells;
		bool alreadyVisited( irr::core::position2d< uint8_t > position );
		uint8_t cols;
		uint8_t rows;
		MazeCell ** maze;
};

#endif // AI_H
