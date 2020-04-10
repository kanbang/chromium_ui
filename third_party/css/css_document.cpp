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

#include "css_document.h"
#include "css_property.h"
#include "css_selector.h"
#include "css_ruleset.h"
#include "css_document_impl.h"

#include "base/strings/string_util.h"
#include "base/files/file_path.h"

LU_CSSDocument::LU_CSSDocument()
: impl(new LU_CSSDocument_Impl)
{
}

LU_CSSDocument::~LU_CSSDocument()
{
}

std::vector<LU_CSSProperty> LU_CSSDocument::select(const std::wstring &element)
{
	std::map<std::wstring, std::vector<LU_CSSProperty> >::iterator it_cache;
	it_cache = impl->select_cache.find(element);
	if (it_cache != impl->select_cache.end())
		return it_cache->second;


	std::vector<LU_CSSRuleSet> sets;
	std::vector<int> specificities;

	std::vector<std::wstring> path_elements;
	std::wstring::size_type pos = 0;
	while (true)
	{
		std::wstring::size_type next_whitespace = element.find(' ', pos);
		if (next_whitespace != std::wstring::npos)
		{
			path_elements.push_back(element.substr(pos, next_whitespace - pos));
		}
		else
		{
			path_elements.push_back(element.substr(pos));
			break;
		}
		pos = next_whitespace + 1;
	}

	std::vector<LU_CSSRuleSet>::size_type index_rulesets, size_rulesets;
	size_rulesets = impl->rulesets.size();
	for (index_rulesets = 0; index_rulesets < size_rulesets; index_rulesets++)
	{
		LU_CSSRuleSet &ruleset = impl->rulesets[index_rulesets];
		std::vector<LU_CSSSelector> &selectors = ruleset.selectors;
		bool match = false;
		int specificity = 0;

		std::vector<LU_CSSSelector>::size_type index_selectors, size_selectors;
		size_selectors = selectors.size();
		for (index_selectors = 0; index_selectors < size_selectors; index_selectors++)
		{
			int s = 0;
			bool m = selectors[index_selectors].select(path_elements, s);
			if (m)
			{
				match = true;
				if (s > specificity)
					specificity = s;
			}
		}

		if (match)
		{
			std::vector<int>::size_type index, size;
			size = specificities.size();
			bool inserted = false;
			for (index = 0; index < size; index++)
			{
				if (specificity >= specificities[index])
				{
					specificities.insert(specificities.begin() + index, specificity);
					sets.insert(sets.begin() + index, ruleset);
					inserted = true;
					break;
				}
			}
			if (!inserted)
			{
				specificities.push_back(specificity);
				sets.push_back(ruleset);
			}
		}
	}

	std::vector<LU_CSSProperty> properties;

	// Prioritize properties with the important flag:
	for (size_t i = 0; i < sets.size(); i++)
	{
		for (size_t j = 0; j < sets[i].properties.size(); j++)
		{
			if (sets[i].properties[j].get_priority() == LU_CSSProperty::priority_important)
			{
				properties.push_back(sets[i].properties[j]);
			}
		}
	}

	// Add the remaining properties:
	for (size_t i = 0; i < sets.size(); i++)
	{
		for (size_t j = 0; j < sets[i].properties.size(); j++)
		{
			if (sets[i].properties[j].get_priority() != LU_CSSProperty::priority_important)
			{
				properties.push_back(sets[i].properties[j]);
			}
		}
	}

	impl->select_cache[element] = properties;
	return properties;
}

std::map<std::wstring, std::wstring> LU_CSSDocument::select_map(const std::wstring &element)
{
  std::map<std::wstring, std::wstring> map_element;
  std::vector<LU_CSSProperty> arr_property = select(element);
  for (int i = 0; i < (int)arr_property.size(); i++)
  {
    LU_CSSProperty & property = arr_property[i];
    if(map_element.find(property.get_name()) != map_element.end())
    {
      if(property.get_priority() == LU_CSSProperty::priority_important)
        map_element[property.get_name()]=property.get_value();
    }
    else
      map_element[property.get_name()]=property.get_value();
  }
  return map_element;
}

void LU_CSSDocument::load(const std::wstring &path, const std::wstring &filename)
{
  impl->load(path, filename);
}

std::vector<LU_CSSProperty> LU_CSSDocument::parse_style_properties(const std::wstring &style_text)
{
	std::vector<LU_CSSProperty> properties;
	std::wstring::size_type pos = 0;
	while (pos < style_text.length())
	{
		int name_start, name_end;
		name_start = pos;
		pos = LU_CSSDocument_Impl::style_load_until(L":;", style_text, pos);
		name_end = pos;
		pos++;

		std::wstring name_text = trim(style_text.substr(name_start, name_end-name_start));

		if (pos <= style_text.length() && style_text[pos-1] == L':')
		{
			if (name_text.empty())
				return properties;

			int value_start, value_end;
			value_start = pos;
			pos = LU_CSSDocument_Impl::style_load_until(L"!;", style_text, pos);
			value_end = pos;

			std::wstring value_text = trim(style_text.substr(value_start, value_end-value_start));
			std::wstring priority_text;
			if (style_text[pos] == '!')
			{
				pos++;
				int priority_start, priority_end;
				priority_start = pos;
				pos = LU_CSSDocument_Impl::style_load_until(L";", style_text, pos);
				priority_end = pos;

				priority_text = trim(style_text.substr(priority_start, priority_end-priority_start));
			}

			LU_CSSProperty property;
			property.set_name(name_text);
			property.set_value(value_text);
			if (compare(priority_text, L"important", true) == 0)
				property.set_priority(LU_CSSProperty::priority_important);
			else
				property.set_priority(LU_CSSProperty::priority_normal);
			properties.push_back(property);
		}
		else if (pos > style_text.length() || style_text[pos-1] == L';')
		{
			if (!name_text.empty())
				return properties;
		}
		else
		{
			break;
		}
	}
	return properties;
}
