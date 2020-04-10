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

/// \addtogroup clanCore_CSS clanCore CSS
/// \{

#pragma once

#include <string>

#include "base/memory/ref_counted.h"
#include "css_export.h"

class LU_CSSProperty_Impl;

/// \brief Cascading Style Sheet (CSS) property.
///
/// \xmlonly !group=Core/CSS! !header=core.h! \endxmlonly
class CSS_EXPORT LU_CSSProperty
{
/// \name Construction
/// \{

public:
	/// \brief Constructs a CSS property.
	LU_CSSProperty();

	~LU_CSSProperty();

/// \}
/// \name Attributes
/// \{

public:
	/// \brief CSS priorities
	enum Priority
	{
		/// \brief Normal priority.
		priority_normal,

		/// \brief Important priority.
		priority_important
	};

	/// \brief Returns the name of the property.
	const std::wstring &get_name() const;

	/// \brief Returns the value of the property.
	const std::wstring &get_value() const;

	/// \brief Returns the priority of the property.
	Priority get_priority() const;

/// \}
/// \name Operations
/// \{

public:
	/// \brief Sets property name.
	void set_name(const std::wstring &name);

	/// \brief Sets property value.
	void set_value(const std::wstring &value);

	/// \brief Sets property priority.
	void set_priority(Priority priority);

	bool operator ==(const LU_CSSProperty &other) const;

/// \}
/// \name Implementation
/// \{

private:
	scoped_refptr<LU_CSSProperty_Impl> impl;
/// \}
};


class LU_CSSProperty_Impl : public base::RefCountedThreadSafe<LU_CSSProperty_Impl>
{
public:
  LU_CSSProperty_Impl()
    : priority(LU_CSSProperty::priority_normal)
  {
  }

  std::wstring name;
  std::wstring value;
  LU_CSSProperty::Priority priority;

};

/// \}
