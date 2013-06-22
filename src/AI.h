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

using namespace irr;

class AI {
	public:
		/** Default constructor */
		AI();
		/** Default destructor */
		virtual ~AI();
		void setPlayer( u8 newPlayer );
		void setup( MazeCell ** newMaze, u8 newCols, u8 newRows,  GameManager * newGM ); //Provides the AI with whatever info it needs to work.
		void move(); //Needs to call GameManager's movePlayerOnX and movePlayerOnY functions.
		void allKeysFound();
		u32 lastTimeMoved;
		u8 getPlayer();
		u32 movementDelay; //How long to delay between movements
	protected:
	private:
		GameManager * gm;
		u8 controlsPlayer;
		std::vector< core::position2d< u8 > > cellsVisited;
		std::vector< core::position2d< u8 > > pathTaken;
		std::vector< std::vector< core::position2d< u8 > > > pathsToLockedCells;
		bool alreadyVisited( core::position2d< u8 > position );
		u8 cols;
		u8 rows;
		MazeCell ** maze;
};

#endif // AI_H
