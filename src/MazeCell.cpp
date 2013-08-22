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
#include "MazeCell.h"
#include <iostream>
#include "colors.h"


MazeCell::MazeCell() {
	try {
		top = 'n';
		left = 'n';
		bottom = 'n';
		right = 'n';
		visited = false;
		distanceFromStart = 999;
		id = 0;
		setVisitorColor(BLACK);
		visible = true;
	} catch ( std::exception e ) {
		std::wcerr << L"Error in MazeCell::MazeCell(): " << e.what() << std::endl;
	}
}

void MazeCell::setVisitorColor( irr::video::SColor color ) {
	try {
		visitorColor = color;
	} catch ( std::exception e ) {
		std::wcerr << L"Error in MazeCell::setVisitorColor(): " << e.what() << std::endl;
	}
}

irr::video::SColor MazeCell::getVisitorColor() {
	try {
		return visitorColor;
	} catch ( std::exception e ) {
		std::wcerr << L"Error in MazeCell::getVisitorColor(): " << e.what() << std::endl;
		return WHITE;
	}
}

MazeCell::~MazeCell() {
	try {
	} catch ( std::exception e ) {
		std::wcerr << L"Error in MazeCell::~MazeCell(): " << e.what() << std::endl;
	}
}

wchar_t MazeCell::getTop() {
	try {
		return top;
	} catch ( std::exception e ) {
		std::wcerr << L"Error in MazeCell::getTop(): " << e.what() << std::endl;
		return 'n';
	}
}

void MazeCell::setTop( wchar_t val )  {
	try {
		top = val;
	} catch ( std::exception e ) {
		std::wcerr << L"Error in MazeCell::setTop(): " << e.what() << std::endl;
	}
}

wchar_t MazeCell::getLeft() {
	try {
		return left;
	} catch ( std::exception e ) {
		std::wcerr << L"Error in MazeCell::getLeft(): " << e.what() << std::endl;
		return 'n';
	}
}

void MazeCell::setLeft( wchar_t val ) {
	try {
		left = val;
	} catch ( std::exception e ) {
		std::wcerr << L"Error in MazeCell::setLeft(): " << e.what() << std::endl;
	}
}

wchar_t MazeCell::getBottom() {
	try {
		return bottom;
	} catch ( std::exception e ) {
		std::wcerr << L"Error in MazeCell::getBottom(): " << e.what() << std::endl;
		return 'n';
	}
}

void MazeCell::setBottom( wchar_t val ) {
	try {
		bottom = val;
	} catch ( std::exception e ) {
		std::wcerr << L"Error in MazeCell::setBottom(): " << e.what() << std::endl;
	}
}

wchar_t MazeCell::getRight() {
	try {
		return right;
	} catch ( std::exception e ) {
		std::wcerr << L"Error in MazeCell::getRight(): " << e.what() << std::endl;
		return 'n';
	}
}

void MazeCell::setRight( wchar_t val ) {
	try {
		right = val;
	} catch ( std::exception e ) {
		std::wcerr << L"Error in MazeCell::setRight(): " << e.what() << std::endl;
	}
}

void MazeCell::removeLocks() {
	try {
		if( top == 'l' ) {
			top = 'n';
		}

		if( left == 'l' ) {
			left = 'n';
		}

		if( bottom == 'l' ) {
			bottom = 'n';
		}

		if( right == 'l' ) {
			right = 'n';
		}
	} catch ( std::exception e ) {
		std::wcerr << L"Error in MazeCell::removeLocks(): " << e.what() << std::endl;
	}
}

bool MazeCell::hasLock() {
	try {
		if( hasLeftLock() || hasTopLock() ) {
			return true;
		} else {
			return false;
		}
	} catch ( std::exception e ) {
		std::wcerr << L"Error in MazeCell::hasLock(): " << e.what() << std::endl;
		return false;
	}
}

bool MazeCell::hasLeftLock() {
	try {
		if( left == 'l' ) {
			return true;
		} else {
			return false;
		}
	} catch ( std::exception e ) {
		std::wcerr << L"Error in MazeCell::hasLeftLock(): " << e.what() << std::endl;
		return false;
	}
}

bool MazeCell::hasTopLock() {
	try {
		if( top == 'l' ) {
			return true;
		} else {
			return false;
		}
	} catch ( std::exception e ) {
		std::wcerr << L"Error in MazeCell::hasTopLock(): " << e.what() << std::endl;
		return false;
	}
}

bool MazeCell::isDeadEnd() {
	try {
		bool result = false;
		if ( top == 'n' && left != 'n' && bottom != 'n' && right != 'n' ) {
			//wcout << L"Dead end (top)" << std::endl;
			result = true;
		} else if ( top != 'n' && left == 'n' && bottom != 'n' && right != 'n' ) {
			//wcout << L"Dead end (left)" << std::endl;
			result = true;
		} else if ( top != 'n' && left != 'n' && bottom == 'n' && right != 'n' ) {
			//wcout << L"Dead end (bottom)" << std::endl;
			result = true;
		} else if ( top != 'n' && left != 'n' && bottom != 'n' && right == 'n' ) {
			//wcout << L"Dead end (right)" << std::endl;
			result = true;
		} else {
			//wcout << L"No dead end here\t" << top << left << bottom << right << std::endl;
			result = false;
		}
		return result;
	} catch ( std::exception e ) {
		std::wcerr << L"Error in MazeCell::isDeadEnd(): " << e.what() << std::endl;
		return false;
	}
}
