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

#pragma once
#include <string>

#include "base/memory/ref_counted.h"

class LU_CSSToken;
class LU_IODevice;

class LU_CSSTokenizer_Impl : public base::RefCountedThreadSafe<LU_CSSTokenizer_Impl>
{
public:
	LU_CSSTokenizer_Impl(LU_IODevice &device);
	LU_CSSTokenizer_Impl(const std::wstring &text);
	void read(LU_CSSToken &out_token);
	void peek(LU_CSSToken &out_token);

private:
	void read_atkeyword(LU_CSSToken &out_token);
	void read_hash(LU_CSSToken &out_token);
	void read_cdo(LU_CSSToken &out_token);
	void read_cdc(LU_CSSToken &out_token);
	void read_comment(LU_CSSToken &out_token);
	void read_uri(LU_CSSToken &out_token);
	void read_function(LU_CSSToken &out_token);
	void read_whitespace(LU_CSSToken &out_token);
	void read_includes(LU_CSSToken &out_token);
	void read_dashmatch(LU_CSSToken &out_token);
	void read_number_type(LU_CSSToken &out_token);
	size_t read_ident(size_t p, std::wstring &out_ident);
	size_t read_name(size_t p, std::wstring &out_ident);
	inline size_t read_nmstart(size_t p, wchar_t &out_c);
	inline size_t read_nmchar(size_t p, wchar_t &out_c);
	size_t read_string(size_t p, std::wstring &out_str, wchar_t str_char) const;
	size_t read_invalid(size_t p) const;
	size_t read_uri_nonquoted_string(size_t p, std::wstring &out_str) const;
	inline static bool is_whitespace(wchar_t c);

	std::wstring doc;
	size_t pos;
};
