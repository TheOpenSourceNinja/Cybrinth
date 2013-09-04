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
#include <string>
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

		void allKeysFound();
		bool atGoal();
		bool doneWaiting();
		uint_least8_t getPlayer();
		void keyFound( uint_fast8_t s );
		void move(); //Needs to call GameManager's movePlayerOnX and movePlayerOnY functions.
		void reset();
		void setPlayer( uint_least8_t newPlayer );
		void setup( MazeCell ** newMaze, uint_least8_t newCols, uint_least8_t newRows,  GameManager * newGM ); //Provides the AI with whatever info it needs to work.
	protected:
	private:
		bool alreadyVisited( irr::core::position2d< uint_least8_t > position );
		bool alreadyVisitedDFS( irr::core::position2d< uint_least8_t > position );
		void findSolution();
		void findSolutionDFS( irr::core::position2d< uint_least8_t > currentPosition );
		void findSolutionDFS( std::vector< irr::core::position2d< uint_least8_t > > partialSolution, irr::core::position2d< uint_least8_t > currentPosition );

		enum algorithm_t{ DEPTH_FIRST_SEARCH };
		algorithm_t algorithm;
		std::vector< irr::core::position2d< uint_least8_t > > cellsVisited;
		uint_least8_t cols;
		uint_least8_t controlsPlayer;
		enum direction_t{ UP, DOWN, LEFT, RIGHT };
		std::vector< irr::core::position2d< uint_least8_t > > DFSCellsVisited;
		GameManager * gm;
		uint_least8_t keyImSeeking;
		uint_least32_t lastTimeMoved;
		MazeCell ** maze;
		uint_least16_t movementDelay; //How long to delay between movements
		//uint_least8_t numKeysInSolution;
		std::vector< std::vector< irr::core::position2d< uint_least8_t > > > pathsToLockedCells;
		std::vector< irr::core::position2d< uint_least8_t > > pathTaken;
		uint_least8_t rows;
		std::vector< irr::core::position2d< uint_least8_t > > solution;
		bool solved;
		bool startSolved;
};

#endif // AI_H
