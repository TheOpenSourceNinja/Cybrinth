/**
 * @file
 * @author James Dearing <dearingj@lifetime.oregonstate.edu>
 * 
 * @section LICENSE
 * Copyright © 2012-2017.
 * This file is part of Cybrinth.
 *
 * Cybrinth is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 *
 * Cybrinth is distributed 'as is' in the hope that it will be fun, but WITHOUT ANY WARRANTY; without even the implied warranty of TITLE, MERCHANTABILITY, COMPLETE DESTRUCTION OF EVIL MONSTERS, or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License along with Cybrinth. If not, see <http://www.gnu.org/licenses/>.
 * 
 * @section DESCRIPTION
 * The Player class represents a player in the game.
 */

#ifndef PLAYER_H
#define PLAYER_H

#include <boost/filesystem.hpp>
#include <irrlicht/irrlicht.h>
#include <vector>
#include "Collectable.h"
#include "Object.h"
#include "colors.h"
#include "PreprocessorCommands.h"

//#include "MainGame.h"
class MainGame; //Avoids circular dependency

class Player : public Object {
	public:
		Player();
		virtual ~Player();
		
		void createTexture( irr::IrrlichtDevice* device, uint_fast16_t size );
		
		void draw( irr::IrrlichtDevice* device, uint_fast16_t width, uint_fast16_t height );
		
		uint_fast8_t getItem();
		Collectable::type_t getItemType();
		intmax_t getScoreLastMaze();
		intmax_t getScoreTotal();
		void giveItem( uint_fast8_t item, Collectable::type_t type );
		
		bool hasItem();
		bool hasItem( uint_fast8_t item );
		
		bool isHuman;
		
		uint_fast8_t keysCollectedThisMaze;
		uint_fast8_t keysCollectedLastMaze;
		
		void loadTexture( irr::IrrlichtDevice* device );
		void loadTexture( irr::IrrlichtDevice* device, uint_fast16_t size, irr::core::stringw name );
		//void loadTexture( irr::IrrlichtDevice* device, uint_fast16_t size );
		void loadTexture( irr::IrrlichtDevice* device, uint_fast16_t size, std::vector< boost::filesystem::path > usableFiles );
		
		void moveX( int_fast8_t val );
		void moveY( int_fast8_t val );
		
		void removeItem();
		void reset();
		
		void setMG( MainGame* newMG );
		void setPlayerNumber( uint_fast8_t newNumber );
		void setScore( intmax_t newScore );
		
		uint_fast16_t stepsTakenLastMaze;
		uint_fast16_t stepsTakenThisMaze;
		
		uint_fast32_t timeTakenLastMaze;
		uint_fast32_t timeTakenThisMaze;
	protected:
	private:
		MainGame* mg;
		
		uint_fast8_t heldItem;
		Collectable::type_t heldItemType;
		
		uint_fast8_t playerNumber; //Each player knows what number it is. That knowledge is only needed for setColorBasedOnNum() and loadTexture().
		
		intmax_t scoreLastMaze;
		intmax_t scoreTotal; //Scores can be negative
		
		irr::core::stringw textureFilePath;
};

#endif // PLAYER_H
