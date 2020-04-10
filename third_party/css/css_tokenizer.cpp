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

#include "css_tokenizer.h"
#include "css_token.h"
#include "css_tokenizer_impl.h"

LU_CSSTokenizer::LU_CSSTokenizer(LU_IODevice &device)
: impl(new LU_CSSTokenizer_Impl(device))
{
}

LU_CSSTokenizer::LU_CSSTokenizer(const std::wstring &text)
: impl(new LU_CSSTokenizer_Impl(text))
{
}

void LU_CSSTokenizer::read(LU_CSSToken &token, bool eat_whitespace, bool eat_comments)
{
	do
	{
		impl->read(token);
		if (eat_comments && (token.type == LU_CSSToken::type_whitespace || token.type == LU_CSSToken::type_comment))
		{
			if (token.type == LU_CSSToken::type_comment)
			{
				token.type = LU_CSSToken::type_whitespace;
				token.value = L" ";
			}
			LU_CSSToken next_token;
			while (true)
			{
				impl->peek(next_token);
				if (next_token.type != LU_CSSToken::type_whitespace && next_token.type != LU_CSSToken::type_comment)
					break;
				impl->read(next_token);
				if (next_token.type == LU_CSSToken::type_comment)
				{
					next_token.type = LU_CSSToken::type_whitespace;
					next_token.value = L" ";
				}
				token.value += next_token.value;
			}
		}
	} while((eat_whitespace && token.type == LU_CSSToken::type_whitespace));
}
