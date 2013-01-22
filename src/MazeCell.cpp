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
	top = '0';
	left = '0';
	bottom = '0';
	right = '0';
	visited = false;
	distanceFromStart = 999;
	id = 0;
	setVisitorColor(BLACK);
}

void MazeCell::setVisitorColor( video::SColor color ) {
	visitorColor = color;
}

video::SColor MazeCell::getVisitorColor() {
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
	if( top == 'l' || top == 'L' ) {
		top = '0';
	}

	if( left == 'l' || left == 'L' ) {
		left = '0';
	}

	if( bottom == 'l' || bottom == 'L' ) {
		bottom = '0';
	}

	if( right == 'l' || right == 'L' ) {
		right = '0';
	}
}

bool MazeCell::hasLock() {
	if( top == 'l' || left == 'l' ) {
		return true;
	} else {
		return false;
	}
}

bool MazeCell::isDeadEnd() {
	bool result = false;
	if ( top == '0' && left != '0' && bottom != '0' && right != '0' ) {
		//wcout << L"Dead end (top)" << endl;
		result = true;
	} else if ( top != '0' && left == '0' && bottom != '0' && right != '0' ) {
		//wcout << L"Dead end (left)" << endl;
		result = true;
	} else if ( top != '0' && left != '0' && bottom == '0' && right != '0' ) {
		//wcout << L"Dead end (bottom)" << endl;
		result = true;
	} else if ( top != '0' && left != '0' && bottom != '0' && right == '0' ) {
		//wcout << L"Dead end (right)" << endl;
		result = true;
	} else {
		//wcout << L"No dead end here\t" << top << left << bottom << right << endl;
		result = false;
	}
	return result;
}
