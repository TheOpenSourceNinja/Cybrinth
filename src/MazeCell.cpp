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

using namespace std;

MazeCell::MazeCell() {
	top = 'n';
	left = 'n';
	bottom = 'n';
	right = 'n';
	visited = false;
	distanceFromStart = 999;
	id = 0;
	setVisitorColor(BLACK);
	visible = true;
}

void MazeCell::setVisitorColor( irr::video::SColor color ) {
	visitorColor = color;
}

irr::video::SColor MazeCell::getVisitorColor() {
	return visitorColor;
}

MazeCell::~MazeCell() {
	//dtor
}

char MazeCell::getTop() {
	return top;
}

void MazeCell::setTop( char val )  {
	top = val;
}

char MazeCell::getLeft() {
	return left;
}

void MazeCell::setLeft( char val ) {
	left = val;
}

char MazeCell::getBottom() {
	return bottom;
}

void MazeCell::setBottom( char val ) {
	bottom = val;
}

char MazeCell::getRight() {
	return right;
}

void MazeCell::setRight( char val ) {
	right = val;
}

void MazeCell::removeLocks() {
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
}

bool MazeCell::hasLock() {
	if( hasLeftLock() || hasTopLock() ) {
		return true;
	} else {
		return false;
	}
}

bool MazeCell::hasLeftLock() {
	if( left == 'l' ) {
		return true;
	} else {
		return false;
	}
}

bool MazeCell::hasTopLock() {
	if( top == 'l' ) {
		return true;
	} else {
		return false;
	}
}

bool MazeCell::isDeadEnd() {
	bool result = false;
	if ( top == 'n' && left != 'n' && bottom != 'n' && right != 'n' ) {
		//wcout << L"Dead end (top)" << endl;
		result = true;
	} else if ( top != 'n' && left == 'n' && bottom != 'n' && right != 'n' ) {
		//wcout << L"Dead end (left)" << endl;
		result = true;
	} else if ( top != 'n' && left != 'n' && bottom == 'n' && right != 'n' ) {
		//wcout << L"Dead end (bottom)" << endl;
		result = true;
	} else if ( top != 'n' && left != 'n' && bottom != 'n' && right == 'n' ) {
		//wcout << L"Dead end (right)" << endl;
		result = true;
	} else {
		//wcout << L"No dead end here\t" << top << left << bottom << right << endl;
		result = false;
	}
	return result;
}
