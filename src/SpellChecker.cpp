#include "SpellChecker.h"

#include <array>
#ifdef HAVE_VECTOR
#include <vector>
#endif //HAVE_VECTOR
#ifdef HAVE_MAP
#include <map>
#endif //HAVE_MAP
#ifdef HAVE_IOSTREAM
#include <iostream>
#endif //HAVE_IOSTREAM

SpellChecker::SpellChecker() {
	//ctor
}

SpellChecker::~SpellChecker() {
	//dtor
}

std::wstring::size_type SpellChecker::DamerauLevenshteinDistance( std::wstring source, std::wstring target ) {
	try {
		//Got this from https://en.wikipedia.org/w/index.php?title=Damerau%E2%80%93Levenshtein_distance&oldid=567829730
		//It's based on the C# example there
		if ( source.empty() ) {
			return target.size();
		} else if ( target.empty() ) {
			return source.size();
		}
		
		std::wstring::size_type score[ source.size() + 2 ][ target.size() + 2 ]; //Where do these +2s come from? Ask Wikipedia.
		
		{
			std::wstring::size_type infinity = source.size() + target.size(); //Okay, so this isn't really infinity. It's close enough.
			score[ 0 ][ 0 ] = infinity;

			for ( decltype( source.size() ) s = 0; s <= source.size(); ++s ) {
				score[ s + 1 ][ 1 ] = s;
				score[ s + 1 ][ 0 ] = infinity;
			}

			for ( decltype( target.size() ) t = 0; t <= target.size(); ++t ) {
				score[ 1 ][ t + 1 ] = t;
				score[ 0 ][ t + 1 ] = infinity;
			}
		}
		
		{
			std::map< std::wstring::value_type, std::wstring::size_type > SortedDictionary;
			{
				std::wstring temp = source + target;
				for( auto i = temp.begin(); i != temp.end(); ++i ) {
					SortedDictionary[ *i ] = 0;
				}
			}

			for ( decltype( source.size() ) s = 1; s <= source.size(); ++s ) {
				std::wstring::size_type DB = 0;
				for ( decltype( target.size() ) t = 1; t <= target.size(); ++t ) {
					auto s1 = SortedDictionary[ target[ t - 1 ] ];
					auto t1 = DB;

					if ( source[ s - 1 ] == target[ t - 1 ] ) {
						score[ s + 1 ][ t + 1 ] = score[ s ][ t ];
						DB = t;
					} else {
						score[ s + 1 ][ t + 1 ] = std::min( score[ s ][ t ], std::min( score[ s + 1 ][ t ], score[ s ][ t + 1 ] ) ) + 1;
					}

					score[ s + 1 ][ t + 1 ] = std::min( score[ s + 1 ][ t + 1 ], score[ s1 ][ t1 ] + ( s - s1 - 1 ) + 1 + ( t - t1 - 1 ) );
				}

				SortedDictionary[ source[ s - 1 ] ] = s;
			}
		}

		return score[ source.size() + 1 ][ target.size() + 1 ];
	} catch( std::exception &e ) {
		std::wcout << L"Error in SpellChecker::DamerauLevenshteinDistance(): " << e.what() << std::endl;
		return -1;
	}
}

std::vector< std::wstring >::size_type SpellChecker::indexOfClosestString( std::wstring source, std::vector< std::wstring > targets ) {
	try {
		auto indexOfMinDistance = targets.max_size();
		auto minDistance = source.max_size();

		for( decltype( targets.size() ) i = 0; i < targets.size(); ++i ) {
			auto distance = DamerauLevenshteinDistance( source, targets.at( i ) );
			if( distance < minDistance ) {
				minDistance = distance;
				indexOfMinDistance = i;
				
				if( minDistance == 0 ) { //We've found an exact match
					break;
				}
			}
		}

		return indexOfMinDistance;
	} catch( std::exception &e ) {
		std::wcout << L"Error in SpellChecker::minDistance(): " << e.what() << std::endl;
		return -1;
	}
}
