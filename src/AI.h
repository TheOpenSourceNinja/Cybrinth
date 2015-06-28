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
 * The AI class is responsible for controlling computer players. It contains the maze-solving algorithms.
 */

#ifndef AI_H
#define AI_H

#ifdef WINDOWS
    #include <irrlicht.h>
#else
    #include <irrlicht/irrlicht.h>
#endif
#ifdef HAVE_STRING
	#include <string>
#endif //HAVE_STRING
#ifdef HAVE_VECTOR
	#include <vector>
#endif //HAVE_VECTOR
#include "Collectable.h"
#include "MazeCell.h"
#include "PreprocessorCommands.h"

//#include "MainGame.h"
class MainGame; //Avoids circular dependency

class AI {
	public:
		/**
		 * Constructor: calls setup() with default values.
		 */
		AI();
		/**
		 * Destructor: Does nothing.
		 */
		virtual ~AI();

		enum algorithm_t : uint_fast8_t { DEPTH_FIRST_SEARCH, ITERATIVE_DEEPENING_DEPTH_FIRST_SEARCH, RIGHT_HAND_RULE, LEFT_HAND_RULE, DIJKSTRA, ALGORITHM_DO_NOT_USE };

		/**
		 * Returns the algorithm most closely matching a given string
		 */
		algorithm_t algorithmFromString( std::wstring input );
		/**
		 * Makes the bot 'forget' that it has visited certain maze cells, specifically those in pathsToLockedCells.
		 */
		void allKeysFound();
		/**
		 * @return A Boolean indicating whether this bot is currently at the goal.
		 */
		bool atGoal();

		/**
		 * Used by MainGame to see if it's time to call this bot's move() function.
		 * @return a Boolean indicating whether enough time has passed to move again.
		 */
		bool doneWaiting();

		/**
		 * Used by MainGame to see which player this bot controls.
		 * @return the number of the player controlled by this AI.
		 */
		uint_fast8_t getPlayer();

		/**
		 * Called by MainGame to tell this bot that a key has been found.
		 * @param key: the number of the key which has been found.
		 */
		void keyFound( uint_fast8_t s );

		/**
		 * Called by MainGame. Moves the player. If the maze is pre-solved, we just follow the solution. If the maze is not pre-solved, this is where the non-pre-solved versions of each algorithm are implemented.
		 */
		void move(); //Needs to call MainGame's movePlayerOnX and movePlayerOnY functions.

		/**
		 * Cleans up between mazes. Called by MainGame and by setup().
		 */
		void reset();

		/**
		 * Sets which player this bot controls.
		 * @param newPlayer: which player to control.
		 */
		void setPlayer( uint_fast8_t newPlayer );
		/**
		 * Anything that needs to be passed from MainGame and used by this bot, such as pointers to various stuff, gets passed here.
		 * @param newMaze: A pointer to a two-dimensional array of maze cells
		 * @param newCols: The width of the maze
		 * @param newRols: The height of the maze
		 * @param newGM: A pointer to the MainGame
		 * @param newStartSolved: A Boolean indicating whether this bot should pre-solve the maze.
		 * @param newAlgorithm: Which algorithm to solve the maze with.
		 * @param newMovementDelay: The amount of time to wait between moves.
		 */
		//void setup( MazeCell ** newMaze, uint_fast8_t newCols, uint_fast8_t newRows, MainGame * newGM, bool newStartSolved, algorithm_t newAlgorithm, uint_fast16_t newMovementDelay );
		void setup( MainGame * newGM, bool newStartSolved, algorithm_t newAlgorithm, uint_fast16_t newMovementDelay );
		/**
		 * Given an algorithm, returns a string representation of the algorithm
		 */
		std::wstring stringFromAlgorithm( algorithm_t input );
	protected:
	private:
		algorithm_t algorithm;
		/**
		 * Used to keep track of whether a given position has really been visited.
		 * @param position: The position in question.
		 * @return a Boolean indicating whether this bot has already visited the given position.
		 */
		bool alreadyVisited( irr::core::position2d< uint_fast8_t > position );
		/**
		 * Used when pre-solving the maze to keep track of whether we've already 'visited' a given position. Separate from alreadyVisited() because we have not actually visited anywhere yet.
		 * @param position: The position in question.
		 * @return a Boolean indicating whether this bot has already 'visited' the given position.
		 */
		bool alreadyVisitedPretend( irr::core::position2d< uint_fast8_t > position );

		std::vector< irr::core::position2d< uint_fast8_t > > cellsVisited; ///< A vector listing all the cells that have been visited so far.
		uint_fast8_t controlsPlayer; ///< The number of the player controlled by this bot.

		enum direction_t : uint_fast8_t { UP, DOWN, LEFT, RIGHT }; ///< Directions, duh.

		std::vector< std::vector< uint_fast16_t > > DijkstraDistance;

		/**
		 * A wrapper for the other version of this function.
		 * @param x: the X coordinate of the cell in question.
		 * @param y: the Y coordinate of the cell in question.
		 * @return A Boolean indicating whether the player can move up/down through this maze cell's top.
		 */
		bool effectivelyNoTopWall( uint_fast8_t x, uint_fast8_t y );
		/**
		 * Determines whether the player can move up/down through this maze cell's top.
		 * @param x: the X coordinate of the cell in question.
		 * @param y: the Y coordinate of the cell in question.
		 * @param canDissolveWalls: A Boolean indicating whether this function should ignore soluble walls (not all walls are soluble).
		 * @return A Boolean indicating whether the player can move up/down through this maze cell's top.
		 */
		bool effectivelyNoTopWall( uint_fast8_t x, uint_fast8_t y, bool canDissolveWalls );
		/**
		 * A wrapper for the other version of this function.
		 * @param x: the X coordinate of the cell in question.
		 * @param y: the Y coordinate of the cell in question.
		 * @return A Boolean indicating whether the player can move left/right through this maze cell's left.
		 */
		bool effectivelyNoLeftWall( uint_fast8_t x, uint_fast8_t y );
		/**
		 * Determines whether the player can move left/right through this maze cell's left.
		 * @param x: the X coordinate of the cell in question.
		 * @param y: the Y coordinate of the cell in question.
		 * @param canDissolveWalls: A Boolean indicating whether this function should ignore soluble walls (not all walls are soluble).
		 * @return A Boolean indicating whether the player can move left/right through this maze cell's left.
		 */
		bool effectivelyNoLeftWall( uint_fast8_t x, uint_fast8_t y, bool canDissolveWalls );

		/**
		 * Calls other solution-finding functions depending on the algorithm.
		 */
		void findSolution();
		/**
		 * Finds a solution using Depth-First Search. Works by calling findSolutionIDDFS() with the max depth possible.
		 * @param startPosition: the position from which to start searching.
		 */
		void findSolutionDFS( irr::core::position2d< uint_fast8_t > startPosition );

		/**
		 * Finds a solution using Dijkstra's algorithm
		 * @param startPosition: the position from which to start searching.
		 */
		void findSolutionDijkstra( irr::core::position2d< uint_fast8_t > startPosition );

		/**
		 * Finds a solution using the Iterative Deepening Depth-First Search algorithm. Does so by calling the other version of findSolutionIDDFS() over and over again with increasing depth limits.
		 */
		void findSolutionIDDFS( irr::core::position2d< uint_fast8_t > startPosition );
		/**
		 * Finds a solution using the Iterative Deepening Depth-First Search algorithm. Does so by calling the other version of findSolutionIDDFS() over and over again with increasing depth limits.
		 */
		void findSolutionIDDFS( std::vector< irr::core::position2d< uint_fast8_t > > partialSolution, irr::core::position2d< uint_fast8_t > startPosition, uint_fast16_t depthLimit, bool canDissolveWalls );

		MainGame * mg; ///< A pointer to the MainGame.

		direction_t hand; ///< Used in Right Hand Rule and Left Hand Rule

		std::vector< irr::core::position2d< uint_fast8_t > > IDDFSDeadEnds;
		uint_fast16_t IDDFSDepthLimit; ///< For use only when the bots don't know the solution.
		bool IDDFSIsDeadEnd( irr::core::position2d< uint_fast8_t > position );

		uint_fast8_t keyImSeeking; ///< When a key is found, see if it's this one. If so, look for a new key. Not used if we don't pre-solve the maze.

		uint_fast32_t lastTimeMoved; ///< The last time this bot moved.

		uint_fast16_t movementDelay; ///< How long to delay between movements

		bool noKeysLeft; ///< the IDDFS algorithm uses this to determine whether it should really deepen or just start with the max depth like DFS.
		/**
		 * Used by those algorithms which do not yet account for the fact that maze walls can be destroyed. Consider this function deprecated, as it will be removed if all algorithms are updated such that they no longer need it.
		 * @param x: the X coordinate of the maze cell in question.
		 * @param y: the Y coordinate of the maze cell in question.
		 */
		bool noOriginalLeftWall( uint_fast8_t x, uint_fast8_t y );
		/**
		 * Used by those algorithms which do not yet account for the fact that maze walls can be destroyed. Consider this function deprecated, as it will be removed if all algorithms are updated such that they no longer need it.
		 * @param x: the X coordinate of the maze cell in question.
		 * @param y: the Y coordinate of the maze cell in question.
		 */
		bool noOriginalTopWall( uint_fast8_t x, uint_fast8_t y );

		std::vector< std::vector< irr::core::position2d< uint_fast8_t > > > pathsToLockedCells; ///< The paths from the current position to each locked cell found so far.
		std::vector< irr::core::position2d< uint_fast8_t > > pathTaken; ///< The path taken so far. Some algorithms use this to backtrack.
		std::vector< irr::core::position2d< uint_fast8_t > > pretendCellsVisited; ///< Cells 'visited' while pre-solving the maze.
		std::vector< irr::core::position2d< uint_fast8_t > > pretendCellsUnvisited; ///< Cells 'unvisited' while pre-solving the maze. Used by Dijkstra's algorithm (more convenient than pretendCellsVisited)

		std::vector< irr::core::position2d< uint_fast8_t > > solution; ///< A list of cells to visit in order to get from start to finish.
		bool solved; ///< Indicates whether the maze has been pre-solved.
		bool startSolved; ///< Indicates whether the maze should be pre-solved.
};

#endif // AI_H
