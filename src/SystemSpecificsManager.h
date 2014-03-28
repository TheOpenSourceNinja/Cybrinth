// *** ADDED BY HEADER FIXUP ***
#include <string>
// *** END ***
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

#ifndef SYSTEMSPECIFICSMANAGER_H
#define SYSTEMSPECIFICSMANAGER_H

#include <filesystem.hpp>
#ifdef HAVE_VECTOR
#include <vector>
#endif //HAVE_VECTOR

#include "PreprocessorCommands.h"
#include "StringConverter.h"

class SystemSpecificsManager
{
	public:
		SystemSpecificsManager();
		virtual ~SystemSpecificsManager();
		std::vector< boost::filesystem::path > getFontFolders();
		std::vector< boost::filesystem::path > getConfigFolders();
	protected:
		std::wstring getEnvironmentVariable( std::string name );
	private:
		StringConverter sc;
};

#endif // SYSTEMSPECIFICSMANAGER_H
