#ifndef SPELLCHECKER_H
#define SPELLCHECKER_H

#include "Integers.h"
#include <vector>
#include <string>

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
