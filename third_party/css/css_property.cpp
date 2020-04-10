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
#include "css_property.h"
#include "css_property_impl.h"

/////////////////////////////////////////////////////////////////////////////
// LU_CSSProperty Construction:

LU_CSSProperty::LU_CSSProperty()
: impl(new LU_CSSProperty_Impl)
{
}

LU_CSSProperty::~LU_CSSProperty()
{
}

/////////////////////////////////////////////////////////////////////////////
// LU_CSSProperty Attributes:

const std::wstring &LU_CSSProperty::get_name() const
{
	return impl->name;
}

const std::wstring &LU_CSSProperty::get_value() const
{
	return impl->value;
}

LU_CSSProperty::Priority LU_CSSProperty::get_priority() const
{
	return impl->priority;
}

/////////////////////////////////////////////////////////////////////////////
// LU_CSSProperty Operations:

void LU_CSSProperty::set_name(const std::wstring &name)
{
	impl->name = name;
}

void LU_CSSProperty::set_value(const std::wstring &value)
{
	impl->value = value;
}

void LU_CSSProperty::set_priority(Priority priority)
{
	impl->priority = priority;
}

bool LU_CSSProperty::operator ==(const LU_CSSProperty &other) const
{
	if (impl->name != other.impl->name)
		return false;
	if (impl->value != other.impl->value)
		return false;
	if (impl->priority != other.impl->priority)
		return false;
	return true;
}

/////////////////////////////////////////////////////////////////////////////
// LU_CSSProperty Implementation:
