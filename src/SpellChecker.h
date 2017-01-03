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
 * The SpellChecker class is just what its name says: a spelling checker. Give it a list of words to use as the dictionary, and one word to check against the dictionary.
 */

#ifndef SPELLCHECKER_H
#define SPELLCHECKER_H

#include "Integers.h"
#include "PreprocessorCommands.h"
#ifdef HAVE_VECTOR
#include <vector>
#endif //HAVE_VECTOR
#ifdef HAVE_STRING
#include <string>
#endif //HAVE_STRING


//Spell checker: A class used for general ad-hackery.
class SpellChecker {
	public:
		SpellChecker();
		virtual ~SpellChecker();
		std::wstring::size_type DamerauLevenshteinDistance( std::wstring source, std::wstring target );
		std::vector< std::wstring >::size_type indexOfClosestString( std::wstring source, std::vector< std::wstring > targets );
	protected:
	private:
};

#endif // SPELLCHECKER_H
