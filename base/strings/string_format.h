/*
**  ClanLib SDK
**  Copyright (c) 1997-2011 The ClanLib Team
**
**  This software is provided 'as-is', without any express or implied
**  warranty.  In no event will the authors be held liable for any damages
**  arising from the use of this software.
**
**  Permission is granted to anyone to use this software for any purpose,
**  including commercial applications, and to alter it and redistribute it
**  freely, subject to the following restrictions:
**
**  1. The origin of this software must not be misrepresented; you must not
**     claim that you wrote the original software. If you use this software
**     in a product, an acknowledgment in the product documentation would be
**     appreciated but is not required.
**  2. Altered source versions must be plainly marked as such, and must not be
**     misrepresented as being the original software.
**  3. This notice may not be removed or altered from any source distribution.
**
**  Note: Some of the libraries ClanLib may link to may have additional
**  requirements or restrictions.
**
**  File Author(s):
**
**    Magnus Norddahl
*/

/// \addtogroup clanCore_Text clanCore Text
/// \{

#pragma once

#include "base/base_export.h"
#include "base/strings/string16.h"

#include <vector>

/// \brief String formatting class.
///
/// \xmlonly !group=Core/Text! !header=core.h! \endxmlonly
class BASE_EXPORT LU_StringFormat
{
/// \name Construction
/// \{

public:

	/// \brief Constructs a TempStringFormat
	///
	/// \param format_string = String Ref
	LU_StringFormat(const string16 &format_string);

	~LU_StringFormat();

/// \}
/// \name Attributes
/// \{

public:
	const string16 &get_result() const;

/// \}
/// \name Operations
/// \{

public:

	/// \brief Set arg
	///
	/// \param index = value
	/// \param text = String Ref
	void set_arg(int index, const string16 &text);

	/// \brief Set arg
	///
	/// \param index = value
	/// \param value = value
	/// \param min_length = value
	void set_arg(int index, int value, int min_length = 0);

	/// \brief Set arg
	///
	/// \param index = value
	/// \param value = value
	/// \param min_length = value
	void set_arg(int index, unsigned int value, int min_length = 0);

	/// \brief Set arg
	///
	/// \param index = value
	/// \param value = value
	/// \param min_length = value
	void set_arg(int index, long unsigned int value, int min_length = 0);

	/// \brief Set arg
	///
	/// \param index = value
	/// \param value = value
	/// \param min_length = value
	void set_arg(int index, long long value, int min_length = 0);

	/// \brief Set arg
	///
	/// \param index = value
	/// \param value = value
	/// \param min_length = value
	void set_arg(int index, unsigned long long value, int min_length = 0);

	/// \brief Set arg
	///
	/// \param index = value
	/// \param value = value
	void set_arg(int index, float value);

	/// \brief Set arg
	///
	/// \param index = value
	/// \param value = value
	void set_arg(int index, double value);

/// \}
/// \name Implementation
/// \{

private:

	/// \brief Create arg
	///
	/// \param index = value
	/// \param start = value
	/// \param length = value
	void create_arg(int index, int start, int length);

	string16 string;

	struct ArgPosition
	{
		ArgPosition() : start(0), length(0) { }
		ArgPosition(int s, int l) : start(s), length(l) {}
		int start;
		int length;
	};

	std::vector<ArgPosition> args;
/// \}
};

inline string16 cl_format(const string16 &format)
{ return format; }

template <class Arg1>
string16 cl_format(const string16 &format, Arg1 arg1)
{ LU_StringFormat f(format); f.set_arg(1, arg1); return f.get_result(); }

template <class Arg1, class Arg2>
string16 cl_format(const string16 &format, Arg1 arg1, Arg2 arg2)
{ LU_StringFormat f(format); f.set_arg(1, arg1); f.set_arg(2, arg2); return f.get_result(); }

template <class Arg1, class Arg2, class Arg3>
string16 cl_format(const string16 &format, Arg1 arg1, Arg2 arg2, Arg3 arg3)
{ LU_StringFormat f(format); f.set_arg(1, arg1); f.set_arg(2, arg2); f.set_arg(3, arg3); return f.get_result(); }

template <class Arg1, class Arg2, class Arg3, class Arg4>
string16 cl_format(const string16 &format, Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4)
{ LU_StringFormat f(format); f.set_arg(1, arg1); f.set_arg(2, arg2); f.set_arg(3, arg3); f.set_arg(4, arg4); return f.get_result(); }

template <class Arg1, class Arg2, class Arg3, class Arg4, class Arg5>
string16 cl_format(const string16 &format, Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5)
{ LU_StringFormat f(format); f.set_arg(1, arg1); f.set_arg(2, arg2); f.set_arg(3, arg3); f.set_arg(4, arg4); f.set_arg(5, arg5); return f.get_result(); }

template <class Arg1, class Arg2, class Arg3, class Arg4, class Arg5, class Arg6>
string16 cl_format(const string16 &format, Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6)
{ LU_StringFormat f(format); f.set_arg(1, arg1); f.set_arg(2, arg2); f.set_arg(3, arg3); f.set_arg(4, arg4); f.set_arg(5, arg5); f.set_arg(6, arg6); return f.get_result(); }

template <class Arg1, class Arg2, class Arg3, class Arg4, class Arg5, class Arg6, class Arg7>
string16 cl_format(const string16 &format, Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6, Arg7 arg7)
{ LU_StringFormat f(format); f.set_arg(1, arg1); f.set_arg(2, arg2); f.set_arg(3, arg3); f.set_arg(4, arg4); f.set_arg(5, arg5); f.set_arg(6, arg6); f.set_arg(7, arg7); return f.get_result(); }

/// \}
