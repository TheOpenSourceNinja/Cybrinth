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

#ifndef MAZECELL_H
#define MAZECELL_H

#include <irrlicht.h>
#include <cstdint>

class MazeCell {
	public:
		MazeCell();
		virtual ~MazeCell();
		char getTop();
		void setTop( char val );
		char getLeft();
		void setLeft( char val );
		char getBottom();
		void setBottom( char val );
		char getRight();
		void setRight( char val );
		void removeLocks();
		bool visited; //Used by the maze generation algorithm to ensure there are no loops, by canGetTo() to indicate which cells it's already look at, and during play for the players' own use.
		uint_least16_t distanceFromStart;
		uint_least16_t id;
		bool isDeadEnd();
		irr::video::SColor getVisitorColor();
		void setVisitorColor( irr::video::SColor color );
		bool hasLock();
		bool hasLeftLock();
		bool hasTopLock();
		bool visible;
	protected:
	private:
		char top;
		char left;
		char bottom; //Useless except on maze borders and when checking if it's a dead end. Do not rely on this to be accurate.
		char right; //Ditto.
		irr::video::SColor visitorColor;
};

#endif // MAZECELL_H
