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

#ifndef MAZEMANAGER_H
#define MAZEMANAGER_H

//#include "MainGame.h"
#include "MazeCell.h"
#include "PreprocessorCommands.h"

#include <boost/filesystem.hpp>

#include <irrlicht/irrlicht.h>

class MainGame;

class MazeManager {
	public:
		/** Default constructor */
		MazeManager();
		/** Default destructor */
		virtual ~MazeManager();

		MazeCell** maze;

		bool canGetTo( uint_fast8_t startX, uint_fast8_t startY, uint_fast8_t goalX, uint_fast8_t goalY );
		bool canGetToAllCollectables( uint_fast8_t startX, uint_fast8_t startY );

		void draw( irr::video::IVideoDriver* driver, uint_fast16_t cellWidth, uint_fast16_t cellHeight );
		
		irr::core::stringw getFileTypeExtension();
		irr::core::stringw getFileTypeName();
		bool hideUnseen;
		
		bool loadFromFile();
		bool loadFromFile( boost::filesystem::path src );
		
		void makeCellsVisible( uint_fast8_t x, uint_fast8_t y );
		void makeRandomLevel();
		
		void newMaze( uint_fast8_t newCols, uint_fast8_t newRows );

		void recurseRandom( uint_fast8_t x, uint_fast8_t y, uint_fast16_t depth, uint_fast16_t numSoFar );

		bool saveToFile( boost::filesystem::path dest );

		void setMainGame( MainGame* newGM );

		uint_fast8_t cols;

		MainGame* mainGame;

		uint_fast8_t rows;
	protected:
	private:
		irr::core::stringw fileTypeExtension;
		irr::core::stringw fileTypeName;
};

#endif // MAZEMANAGER_H
