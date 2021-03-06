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
 * The MazeCell class represents one cell in the maze.
 */
 
#include "MazeCell.h"
#ifdef HAVE_IOSTREAM
#include <iostream>
#endif //HAVE_IOSTREAM
#include "colors.h"


MazeCell::MazeCell() {
	try {
		top = NONE;
		left = NONE;
		bottom = NONE;
		right = NONE;
		originalTop = NONE;
		originalLeft = NONE;
		visited = false;
		distanceFromStart = 999;
		id = 0;
		setVisitorColor(BLACK);
		bool startVisible = false;
		topVisible = startVisible;
		leftVisible = startVisible;
		rightVisible = startVisible;
		bottomVisible = startVisible;
	} catch ( std::exception &e ) {
		std::wcerr << L"Error in MazeCell::MazeCell(): " << e.what() << std::endl;
	}
}

void MazeCell::setVisitorColor( irr::video::SColor color ) {
	try {
		visitorColor = color;
	} catch ( std::exception &e ) {
		std::wcerr << L"Error in MazeCell::setVisitorColor(): " << e.what() << std::endl;
	}
}

irr::video::SColor MazeCell::getVisitorColor() const {
	try {
		return visitorColor;
	} catch ( std::exception &e ) {
		std::wcerr << L"Error in MazeCell::getVisitorColor(): " << e.what() << std::endl;
		return WHITE;
	}
}

MazeCell::~MazeCell() {
	try {
	} catch ( std::exception &e ) {
		std::wcerr << L"Error in MazeCell::~MazeCell(): " << e.what() << std::endl;
	}
}

MazeCell::border_t MazeCell::getTop() const {
	try {
		return top;
	} catch ( std::exception &e ) {
		std::wcerr << L"Error in MazeCell::getTop(): " << e.what() << std::endl;
		return NONE;
	}
}

MazeCell::border_t MazeCell::getOriginalTop() const {
	try {
		return originalTop;
	} catch ( std::exception &e ) {
		std::wcerr << L"Error in MazeCell::getOriginalTop(): " << e.what() << std::endl;
		return NONE;
	}
}

void MazeCell::setTop( border_t val )  {
	try {
		top = val;
	} catch ( std::exception &e ) {
		std::wcerr << L"Error in MazeCell::setTop(): " << e.what() << std::endl;
	}
}

void MazeCell::setOriginalTop( border_t val ) {
	try {
		originalTop = val;
		setTop( val );
	} catch ( std::exception &e ) {
		std::wcerr << L"Error in MazeCell::setOriginalTop(): " << e.what() << std::endl;
	}
}

MazeCell::border_t MazeCell::getOriginalLeft() const {
	try {
		return originalLeft;
	} catch ( std::exception &e ) {
		std::wcerr << L"Error in MazeCell::getOriginalLeft(): " << e.what() << std::endl;
		return NONE;
	}
}

MazeCell::border_t MazeCell::getLeft() const {
	try {
		return left;
	} catch ( std::exception &e ) {
		std::wcerr << L"Error in MazeCell::getLeft(): " << e.what() << std::endl;
		return NONE;
	}
}

void MazeCell::setLeft( border_t val ) {
	try {
		left = val;
	} catch ( std::exception &e ) {
		std::wcerr << L"Error in MazeCell::setLeft(): " << e.what() << std::endl;
	}
}

void MazeCell::setOriginalLeft( border_t val ) {
	try {
		originalLeft = val;
		setLeft( val );
	} catch ( std::exception &e ) {
		std::wcerr << L"Error in MazeCell::setOriginalLeft(): " << e.what() << std::endl;
	}
}

MazeCell::border_t MazeCell::getBottom() const {
	try {
		return bottom;
	} catch ( std::exception &e ) {
		std::wcerr << L"Error in MazeCell::getBottom(): " << e.what() << std::endl;
		return NONE;
	}
}

void MazeCell::setOriginalBottom( border_t val ) {
	try {
		bottom = val;
	} catch ( std::exception &e ) {
		std::wcerr << L"Error in MazeCell::setBottom(): " << e.what() << std::endl;
	}
}

MazeCell::border_t MazeCell::getRight() const {
	try {
		return right;
	} catch ( std::exception &e ) {
		std::wcerr << L"Error in MazeCell::getRight(): " << e.what() << std::endl;
		return NONE;
	}
}

void MazeCell::setOriginalRight( border_t val ) {
	try {
		right = val;
	} catch ( std::exception &e ) {
		std::wcerr << L"Error in MazeCell::setRight(): " << e.what() << std::endl;
	}
}

void MazeCell::removeLocks() {
	try {
		if( top == LOCK ) {
			top = NONE;
		}

		if( left == LOCK ) {
			left = NONE;
		}

		if( bottom == LOCK ) {
			bottom = NONE;
		}

		if( right == LOCK ) {
			right = NONE;
		}
	} catch ( std::exception &e ) {
		std::wcerr << L"Error in MazeCell::removeLocks(): " << e.what() << std::endl;
	}
}

bool MazeCell::hasLock() {
	try {
		if( hasLeftLock() or hasTopLock() ) {
			return true;
		} else {
			return false;
		}
	} catch ( std::exception &e ) {
		std::wcerr << L"Error in MazeCell::hasLock(): " << e.what() << std::endl;
		return false;
	}
}

bool MazeCell::hasLeftLock() const {
	try {
		if( left == LOCK ) {
			return true;
		} else {
			return false;
		}
	} catch ( std::exception &e ) {
		std::wcerr << L"Error in MazeCell::hasLeftLock(): " << e.what() << std::endl;
		return false;
	}
}

bool MazeCell::hasTopLock() const {
	try {
		if( top == LOCK ) {
			return true;
		} else {
			return false;
		}
	} catch ( std::exception &e ) {
		std::wcerr << L"Error in MazeCell::hasTopLock(): " << e.what() << std::endl;
		return false;
	}
}

bool MazeCell::isDeadEnd() const {
	try {
		bool result = false;
		if ( top == NONE and left not_eq NONE and bottom not_eq NONE and right not_eq NONE ) {
			//wcout << L"Dead end (top)" << std::endl;
			result = true;
		} else if ( top not_eq NONE and left == NONE and bottom not_eq NONE and right not_eq NONE ) {
			//wcout << L"Dead end (left)" << std::endl;
			result = true;
		} else if ( top not_eq NONE and left not_eq NONE and bottom == NONE and right not_eq NONE ) {
			//wcout << L"Dead end (bottom)" << std::endl;
			result = true;
		} else if ( top not_eq NONE and left not_eq NONE and bottom not_eq NONE and right == NONE ) {
			//wcout << L"Dead end (right)" << std::endl;
			result = true;
		} else {
			//wcout << L"No dead end here\t" << top << left << bottom << right << std::endl;
			result = false;
		}
		return result;
	} catch ( std::exception &e ) {
		std::wcerr << L"Error in MazeCell::isDeadEnd(): " << e.what() << std::endl;
		return false;
	}
}
