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
 * The CustomException class was created so that we can throw std::exceptions with custom what() messages.
 */

#include "CustomException.h"

CustomException::CustomException( std::wstring newMessage ) {
	message = newMessage;
}

CustomException::~CustomException() {
	//dtor
}

CustomException::CustomException( const CustomException& other ) {
	message = other.message;
}

std::wstring CustomException::what() {
	return message;
}
