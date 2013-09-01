#include "SpellChecker.h"

#include <array>
#include <vector>
#include <map>
#include <iostream>

SpellChecker::SpellChecker()
{
	//ctor
}

SpellChecker::~SpellChecker()
{
	//dtor
}

std::wstring::size_type SpellChecker::DamerauLevenshteinDistance( std::wstring source, std::wstring target ) {
	try {
		if ( source.empty() ) {
			return target.size();
		} else if ( target.empty() ) {
			return source.size();
		}

		std::wstring::size_type score[ source.size() + 2 ][ target.size() + 2 ];

		std::wstring::size_type INF = source.size() + target.size();
		score[ 0 ][ 0 ] = INF;

		for (std::wstring::size_type i = 0; i <= source.size(); ++i) {
			score[ i + 1 ][ 1 ] = i;
			score[ i + 1 ][ 0 ] = INF;
		}

		for (std::wstring::size_type j = 0; j <= target.size(); ++j) {
			score[ 1 ][ j + 1 ] = j;
			score[ 0 ][ j + 1 ] = INF;
		}

		std::map< std::wstring::value_type, std::wstring::size_type > sd;
		std::wstring temp = source + target;
		for( std::wstring::iterator i = temp.begin(); i != temp.end(); ++i ) {
			sd[ *i ] = 0;
		}

		for (std::wstring::size_type i = 1; i <= source.size(); ++i) {
			std::wstring::size_type DB = 0;
			for (std::wstring::size_type j = 1; j <= target.size(); ++j) {
				std::wstring::size_type i1 = sd[ target[ j - 1 ] ];
				std::wstring::size_type j1 = DB;

				if ( source[ i - 1 ] == target[ j - 1 ] ) {
					score[ i + 1 ][ j + 1 ] = score[ i ][ j ];
					DB = j;
				} else {
					score[ i + 1 ][ j + 1 ] = std::min( score[ i ][ j ], std::min( score[ i + 1 ][ j ], score[ i ][ j + 1 ] ) ) + 1;
				}

				score[ i + 1 ][ j + 1 ] = std::min( score[ i + 1 ][ j + 1 ], score[ i1 ][ j1 ] + ( i - i1 - 1 ) + 1 + ( j - j1 - 1 ) );
			}

			sd[ source[ i - 1 ] ] = i;
		}

		return score[ source.size() + 1 ][ target.size() + 1 ];
	} catch( std::exception e ) {
		std::wcout << L"Error in SpellChecker::DamerauLevenshteinDistance(): " << e.what() << std::endl;
		return -1;
	}
}

std::vector< std::wstring >::size_type SpellChecker::indexOfClosestString( std::wstring source, std::vector< std::wstring > targets ) {
	try {
		std::vector< std::wstring >::size_type indexOfMinDistance = -1;
		std::wstring::size_type minDistance = -1;

		for( std::vector< std::wstring >::size_type i = 0; i < targets.size(); ++i ) {
			std::wstring::size_type distance = DamerauLevenshteinDistance( source, targets.at( i ) );
			if( distance < minDistance ) {
				minDistance = distance;
				indexOfMinDistance = i;
			}
		}

		return indexOfMinDistance;
	} catch( std::exception e ) {
		std::wcout << L"Error in SpellChecker::minDistance(): " << e.what() << std::endl;
		return -1;
	}
}
