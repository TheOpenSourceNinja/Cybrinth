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

#ifndef INTEGERS_H_INCLUDED
#define INTEGERS_H_INCLUDED

#if HAVE_STDINT_H
	#include <cstdint>
#endif
#if !defined UINT8_MAX && !defined uint8_t && !defined __uint8_t_defined
	//! 8 bit unsigned variable.
	typedef unsigned char	uint8_t;
	#define UINT8_MAX 255
	#define __uint8_t_defined
#endif

#if !defined INT8_MAX && !defined int8_t && !defined __int8_t_defined
	//! 8 bit signed variable.
	typedef signed char	int8_t;
	#define INT8_MAX 127
	#define __int8_t_defined
#endif

#if !defined UINT16_MAX && !defined uint16_t && !defined __uint16_t_defined
	//! 16 bit unsigned variable.
	typedef unsigned short int	uint16_t;
	#define UINT16_MAX 65535
	#define __uint16_t_defined
#endif

#if !defined INT16_MAX && !defined int16_t && !defined __int16_t_defined
	//! 16 bit signed variable.
	typedef signed short int	int16_t;
	#define INT16_MAX 32767
	#define __int16_t_defined
#endif

#if !defined UINT32_MAX && !defined uint32_t && !defined __uint32_t_defined
	//! 32 bit unsigned variable.
	typedef unsigned long int	uint32_t;
	#define UINT32_MAX 4294967295
	#define __uint32_t_defined
#endif

#if !defined INT32_MAX && !defined int32_t && !defined __int32_t_defined
	//! 32 bit signed variable.
	typedef signed long int	int32_t;
	#define INT32_MAX 2147483647
	#define __int32_t_defined
#endif

#if !defined UINT64_MAX && !defined uint64_t && !defined __uint64_t_defined
	//! 64 bit unsigned variable.
	typedef unsigned long long int	uint64_t;
	#define UINT64_MAX 18446744073709551615
	#define __uint64_t_defined
#endif

#if !defined INT64_MAX && !defined int64_t && !defined __int64_t_defined
	//! 64 bit signed variable.
	typedef signed long long int	int64_t;
	#define INT64_MAX 9223372036854775807
	#define __int64_t_defined
#endif

#endif // INTEGERS_H_INCLUDED
