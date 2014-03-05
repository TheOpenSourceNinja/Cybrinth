/**
 * Copyright © 2012-2014 James Dearing.
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
#include "PreprocessorCommands.h"

//#include "GameManager.h"
class GameManager; //Avoids circular dependency

class AI {
	public:
		/** Default constructor */
		AI();
		/** Default destructor */
		virtual ~AI();

		enum algorithm_t{ DEPTH_FIRST_SEARCH, ITERATIVE_DEEPENING_DEPTH_FIRST_SEARCH, RIGHT_HAND_RULE, LEFT_HAND_RULE };
		void allKeysFound();
		bool atGoal();
		bool doneWaiting();
		uint_fast8_t getPlayer();
		void keyFound( uint_fast8_t s );
		void move(); //Needs to call GameManager's movePlayerOnX and movePlayerOnY functions.
		void reset();
		void setPlayer( uint_fast8_t newPlayer );
		void setup( MazeCell ** newMaze, uint_fast8_t newCols, uint_fast8_t newRows,  GameManager * newGM, bool newStartSolved, algorithm_t newAlgorithm ); //Provides the AI with whatever info it needs to work.
	protected:
	private:
		bool alreadyVisited( irr::core::position2d< uint_fast8_t > position );
		bool alreadyVisitedDFS( irr::core::position2d< uint_fast8_t > position );
		bool alreadyVisitedIDDFS( irr::core::position2d< uint_fast8_t > position );
		void findSolution();
		void findSolutionDFS( irr::core::position2d< uint_fast8_t > currentPosition );
		void findSolutionDFS( std::vector< irr::core::position2d< uint_fast8_t > > partialSolution, irr::core::position2d< uint_fast8_t > currentPosition );
		void findSolutionIDDFS( irr::core::position2d< uint_fast8_t > currentPosition );
		void findSolutionIDDFS( std::vector< irr::core::position2d< uint_fast8_t > > partialSolution, irr::core::position2d< uint_fast8_t > currentPosition, uint_fast16_t depthLimit );

		algorithm_t algorithm;
		std::vector< irr::core::position2d< uint_fast8_t > > cellsVisited;
		uint_fast8_t cols;
		uint_fast8_t controlsPlayer;
		enum direction_t{ UP, DOWN, LEFT, RIGHT };
		std::vector< irr::core::position2d< uint_fast8_t > > DFSCellsVisited;
		direction_t hand; //Used in Right Hand Rule and Left Hand Rule
		std::vector< irr::core::position2d< uint_fast8_t > > IDDFSCellsVisited;
		uint_fast16_t IDDFSDepthLimit; //For use only when the bots don't know the solution.
		GameManager * gm;
		uint_fast8_t keyImSeeking;
		bool noKeysLeft; //the IDDFS algorithm uses this to determine whether it should really deepen or just start with the max depth like DFS.
		uint_fast32_t lastTimeMoved;
		MazeCell ** maze;
		uint_fast16_t movementDelay; //How long to delay between movements
		//uint_fast8_t numKeysInSolution;
		std::vector< std::vector< irr::core::position2d< uint_fast8_t > > > pathsToLockedCells;
		std::vector< irr::core::position2d< uint_fast8_t > > pathTaken;
		uint_fast8_t rows;
		std::vector< irr::core::position2d< uint_fast8_t > > solution;
		bool solved;
		bool startSolved;
};

#endif // AI_H
