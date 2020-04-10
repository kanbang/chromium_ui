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
**    Harry Storbacka
**    Mark Page
*/

#include "css_property.h"
#include "css_selector.h"
#include "css_document_impl.h"

#include "base/strings/string_util.h"
#include "base/strings/utf_string_conversions.h"
#include "base/files/file_path.h"
#include "base/files/memory_mapped_file.h"
#include <cstring>

void LU_CSSDocument_Impl::load(const std::wstring &path, const std::wstring &file_name)
{
  base::FilePath fp(path);
  base::FilePath css_path = fp.Append(file_name);
 
  base::MemoryMappedFile memfile;
  memfile.Initialize(css_path);
	// Load document into a buffer:
	size_t size = memfile.length();
	if (!memfile.data())
		return;

	unsigned char *data_ptr = (unsigned char *) memfile.data();
	whitespace_comments(data_ptr, memfile.length());
	size_t pos = 0;
	while (pos < memfile.length())
	{
		unsigned char ch = data_ptr[pos];
		switch (ch)
		{
		case ' ':
		case '\t':
		case '\r':
		case '\n':
			pos++;
			break;

		case '@': // import
			pos = load_import(data_ptr, pos, memfile.length(), path);
			break;

		default: // ruleset
			pos = load_ruleset(data_ptr, pos, memfile.length());
			break;
		}
	}
}

int LU_CSSDocument_Impl::load_import(unsigned char *data, int pos, int length, const std::wstring &path)
{
	int block_start = 0, block_end = 0;
	pos = load_keyword("@import", data, pos, length);
	block_start = load_until("\"", data, pos, length)+1;
	block_end = load_until("\"", data, block_start, length);

	std::string import_utf8((char *) data+block_start, block_end-block_start);
	std::wstring import_text = trim(UTF8ToWide(import_utf8));

	load(path, import_text);

	return load_until(";", data, block_end+1, length)+1;
}

int LU_CSSDocument_Impl::load_ruleset(unsigned char *data, int pos, int length)
{
	LU_CSSRuleSet ruleset;

	// Find selectors
	while (pos < length)
	{
		int selector_start, selector_end;
		selector_start = pos;
		pos = load_until(",{", data, pos, length);
		selector_end = pos;

		std::string selector_utf8((char *) data+selector_start, selector_end-selector_start);
		std::wstring selector_text = trim(UTF8ToWide(selector_utf8));

		// Split selector into path elements:
		LU_CSSSelector selector;
		while (true)
		{
			std::wstring::size_type split_pos = selector_text.find_first_of(L" \t\r\n");
			if (split_pos == std::wstring::npos)
				break;

			std::wstring element = selector_text.substr(0, split_pos);
			selector.path_elements.push_back(element);
			selector_text = trim(selector_text.substr(split_pos));
		}
		if (selector_text.empty())
      return 0;
		selector.path_elements.push_back(selector_text);
		ruleset.selectors.push_back(selector);

		if (data[pos++] == '{')
			break;
	}

	// Read declaration
	while (pos < length)
	{
		int name_start, name_end;
		name_start = pos;
		pos = load_until(":;}", data, pos, length);
		name_end = pos;
		pos++;

		std::string name_utf8((char *) data+name_start, name_end-name_start);
		std::wstring name_text = trim(UTF8ToWide(name_utf8));

		if (data[pos-1] == ':')
		{
			if (name_text.empty())
				return 0;

			int value_start, value_end;
			value_start = pos;
			pos = load_until("!;}", data, pos, length);
			value_end = pos;

			std::string value_utf8((char *) data+value_start, value_end-value_start);
			std::wstring value_text = trim(UTF8ToWide(value_utf8));
			std::wstring priority_text;
			if (data[pos] == '!')
			{
				pos++;
				int priority_start, priority_end;
				priority_start = pos;
				pos = load_until(";}", data, pos, length);
				priority_end = pos;

				std::string priority_utf8((char *) data+priority_start, priority_end-priority_start);
				priority_text = trim(UTF8ToWide(priority_utf8));
			}

			LU_CSSProperty property;
			property.set_name(name_text);
			property.set_value(value_text);
			if (compare(priority_text, L"important", true) == 0)
				property.set_priority(LU_CSSProperty::priority_important);
			else
				property.set_priority(LU_CSSProperty::priority_normal);
			ruleset.properties.push_back(property);

			if (data[pos++] == '}')
				break;
		}
		else if (data[pos-1] == ';')
		{
			if (!name_text.empty())
				return 0;
		}
		else
		{
			break;
		}
	}

	rulesets.push_back(ruleset);

	return pos;
}

int LU_CSSDocument_Impl::load_keyword(const char *keyword, unsigned char *data, int pos, int length)
{
	int keyword_len = strlen(keyword);
	if (pos+keyword_len > length)
    return 0;
	std::string s((char *) data+pos, keyword_len);
	if (compare(keyword, s, true) != 0)
    return 0;
	return pos + keyword_len;
}

int LU_CSSDocument_Impl::load_until(const char *chars, unsigned char *data, int pos, int length)
{
	bool quotes1 = false;
	bool quotes2 = false;
	int level = 0;
	while (pos < length)
	{
		if (level == 0 && quotes1 == false && quotes2 == false)
		{
			for (int i = 0; chars[i] != 0; i++)
			{
				if (data[pos] == (unsigned char) chars[i])
					return pos;
			}
		}

		switch (data[pos++])
		{
		case '"':
			quotes1 = !quotes1;
			break;
		case '\'':
			quotes2 = !quotes2;
			break;
		case '(':
		case '{':
		case '[':
			if (quotes1 || quotes2)
				break;
			level++;
			break;
		case ')':
		case '}':
		case ']':
			if (quotes1 || quotes2)
				break;
			level--;
			break;
		case '\\':
			pos++;
			break;
		}
	}
	return pos;
}

void LU_CSSDocument_Impl::whitespace_comments(unsigned char *data, unsigned int length)
{
	std::string d((char *) data, length, false);
	std::string::size_type pos = 0;
	while (true)
	{
		std::string::size_type next_pos = d.find_first_of("/\\\"'", pos);
		if (next_pos == std::string::npos || next_pos+1 == length)
			break;
		if (d[next_pos] == '/' && d[next_pos+1] == '*')
		{
			std::string::size_type comment_start = next_pos;
			std::string::size_type comment_end = d.find("*/", comment_start+2);
			if (comment_end == std::string::npos)
				comment_end = length;
			else
				comment_end += 2;
			for (std::string::size_type index = comment_start; index < comment_end; index++)
				d[index] = ' ';
			next_pos = comment_end;
		}
		else if (d[next_pos] == '"' || d[next_pos] == '\'')
		{
			next_pos = d.find(d[next_pos], next_pos+1);
			if (next_pos == std::string::npos)
				break;
			pos = next_pos + 1;
		}
		else if (d[next_pos] == '\\')
		{
			pos = next_pos + 2;
		}
		else
		{
			pos = next_pos + 1;
		}
	}
}

unsigned int LU_CSSDocument_Impl::style_load_until(const wchar_t *chars, const std::wstring &style_text, unsigned int pos)
{
	bool quotes1 = false;
	bool quotes2 = false;
	int level = 0;
	while (pos < style_text.length())
	{
		if (level == 0 && quotes1 == false && quotes2 == false)
		{
			for (int i = 0; chars[i] != 0; i++)
			{
				if (style_text[pos] == chars[i])
					return pos;
			}
		}

		switch (style_text[pos++])
		{
		case '"':
			quotes1 = !quotes1;
			break;
		case '\'':
			quotes2 = !quotes2;
			break;
		case '(':
		case '{':
		case '[':
			if (quotes1 || quotes2)
				break;
			level++;
			break;
		case ')':
		case '}':
		case ']':
			if (quotes1 || quotes2)
				break;
			level--;
			break;
		case '\\':
			pos++;
			break;
		}
	}
	return pos;
}
