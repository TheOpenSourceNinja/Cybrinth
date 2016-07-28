/**
 * @file
 * @author James Dearing <dearingj@lifetime.oregonstate.edu>
 * 
 * @section LICENSE
 * Copyright © 2012-2016.
 * This file is part of Cybrinth.
 *
 * Cybrinth is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 *
 * Cybrinth is distributed 'as is' in the hope that it will be fun, but WITHOUT ANY WARRANTY; without even the implied warranty of TITLE, MERCHANTABILITY, COMPLETE DESTRUCTION OF EVIL MONSTERS, or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License along with Cybrinth. If not, see <http://www.gnu.org/licenses/>.
 * 
 * @section DESCRIPTION
 * The SystemSpecificsManager class is responsible for anything that varies from one operating system to another, such as determining which folders are likely to contain fonts.
 */

#ifndef SYSTEMSPECIFICSMANAGER_H
#define SYSTEMSPECIFICSMANAGER_H

#include <boost/filesystem.hpp>
#ifdef HAVE_VECTOR
	#include <vector>
#endif //HAVE_VECTOR

#include "PreprocessorCommands.h"
#include "StringConverter.h"
#include <string>

class SystemSpecificsManager {
	public:
		SystemSpecificsManager();
		virtual ~SystemSpecificsManager();
		std::vector< boost::filesystem::path > getDataFolders();
		std::vector< boost::filesystem::path > getFontFolders();
		std::vector< boost::filesystem::path > getImageFolders();
		std::vector< boost::filesystem::path > getMusicFolders();
		std::vector< boost::filesystem::path > getConfigFolders(); //Order: Increasing specificity of who can read/write to the folder. I.e. start with system-wide stuff, end with stuff specific to the current user.
		bool canBeUsedAsFolder( boost::filesystem::path folder );
	protected:
		std::wstring getEnvironmentVariable( std::string name );
		std::wstring getEnvironmentVariable( std::wstring name );
	private:
		StringConverter sc;
};

#endif // SYSTEMSPECIFICSMANAGER_H
