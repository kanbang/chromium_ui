// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/ui/omnibox/autocomplete_input.h"

#include "base/strings/string_util.h"
#include "base/strings/utf_string_conversions.h"
// #include "chrome/browser/external_protocol/external_protocol_handler.h"
// #include "chrome/browser/profiles/profile_io_data.h"
// #include "chrome/common/net/url_fixer_upper.h"
// #include "content/public/common/url_constants.h"
// #include "net/base/net_util.h"
// #include "net/base/registry_controlled_domains/registry_controlled_domain.h"
// #include "url/url_canon_ip.h"

namespace {

void AdjustCursorPositionIfNecessary(size_t num_leading_chars_removed,
                                     size_t* cursor_position) {
  if (*cursor_position == string16::npos)
    return;
  if (num_leading_chars_removed < *cursor_position)
    *cursor_position -= num_leading_chars_removed;
  else
    *cursor_position = 0;
}

}  // namespace

AutocompleteInput::AutocompleteInput()
    : cursor_position_(string16::npos),
      current_page_classification_(AutocompleteInput::INVALID_SPEC),
      type_(INVALID),
      prevent_inline_autocomplete_(false),
      prefer_keyword_(false),
      allow_exact_keyword_match_(true),
      matches_requested_(ALL_MATCHES) {
}

AutocompleteInput::AutocompleteInput(
    const string16& text,
    size_t cursor_position,
    const string16& desired_tld,
    const GURL& current_url,
    AutocompleteInput::PageClassification current_page_classification,
    bool prevent_inline_autocomplete,
    bool prefer_keyword,
    bool allow_exact_keyword_match,
    MatchesRequested matches_requested)
    : cursor_position_(cursor_position),
      current_url_(current_url),
      current_page_classification_(current_page_classification),
      prevent_inline_autocomplete_(prevent_inline_autocomplete),
      prefer_keyword_(prefer_keyword),
      allow_exact_keyword_match_(allow_exact_keyword_match),
      matches_requested_(matches_requested) {
  DCHECK(cursor_position <= text.length() || cursor_position == string16::npos)
      << "Text: '" << text << "', cp: " << cursor_position;
  // None of the providers care about leading white space so we always trim it.
  // Providers that care about trailing white space handle trimming themselves.
  if ((TrimWhitespace(text, TRIM_LEADING, &text_) & TRIM_LEADING) != 0)
    AdjustCursorPositionIfNecessary(text.length() - text_.length(),
                                    &cursor_position_);

  GURL canonicalized_url;
  type_ = Parse(text_, desired_tld, &parts_, &scheme_, &canonicalized_url);

  if (type_ == INVALID)
    return;

  if (((type_ == UNKNOWN) || (type_ == URL)) &&
      canonicalized_url.is_valid() &&
      (!canonicalized_url.IsStandard() || canonicalized_url.SchemeIsFile() ||
       canonicalized_url.SchemeIsFileSystem() ||
       !canonicalized_url.host().empty()))
    canonicalized_url_ = canonicalized_url;

  size_t chars_removed = RemoveForcedQueryStringIfNecessary(type_, &text_);
  AdjustCursorPositionIfNecessary(chars_removed, &cursor_position_);
  if (chars_removed) {
    // Remove spaces between opening question mark and first actual character.
    string16 trimmed_text;
    if ((TrimWhitespace(text_, TRIM_LEADING, &trimmed_text) & TRIM_LEADING) !=
        0) {
      AdjustCursorPositionIfNecessary(text_.length() - trimmed_text.length(),
                                      &cursor_position_);
      text_ = trimmed_text;
    }
  }
}

AutocompleteInput::~AutocompleteInput() {
}

// static
size_t AutocompleteInput::RemoveForcedQueryStringIfNecessary(Type type,
                                                             string16* text) {
  if (type != FORCED_QUERY || text->empty() || (*text)[0] != L'?')
    return 0;
  // Drop the leading '?'.
  text->erase(0, 1);
  return 1;
}

// static
std::string AutocompleteInput::TypeToString(Type type) {
  switch (type) {
    case INVALID:       return "invalid";
    case UNKNOWN:       return "unknown";
    case URL:           return "url";
    case QUERY:         return "query";
    case FORCED_QUERY:  return "forced-query";

    default:
      NOTREACHED();
      return std::string();
  }
}

// static
AutocompleteInput::Type AutocompleteInput::Parse(
    const string16& text,
    const string16& desired_tld,
    url_parse::Parsed* parts,
    string16* scheme,
    GURL* canonicalized_url) {
  const size_t first_non_white = text.find_first_not_of(kWhitespaceUTF16, 0);
  if (first_non_white == string16::npos)
    return INVALID;  // All whitespace.

  if (text.at(first_non_white) == L'?') {
    // If the first non-whitespace character is a '?', we magically treat this
    // as a query.
    return FORCED_QUERY;
  }

  // Ask our parsing back-end to help us understand what the user typed.  We
  // use the URLFixerUpper here because we want to be smart about what we
  // consider a scheme.  For example, we shouldn't consider www.google.com:80
  // to have a scheme.
  url_parse::Parsed local_parts;
  if (!parts)
    parts = &local_parts;
  const string16 parsed_scheme(text);
  if (scheme)
    *scheme = parsed_scheme;
  if (canonicalized_url) {
    //*canonicalized_url = URLFixerUpper::FixupURL(UTF16ToUTF8(text),
    //                                             UTF16ToUTF8(desired_tld));
  }

  if (LowerCaseEqualsASCII(parsed_scheme, "file")) {
    // A user might or might not type a scheme when entering a file URL.  In
    // either case, |parsed_scheme| will tell us that this is a file URL, but
    // |parts->scheme| might be empty, e.g. if the user typed "C:\foo".
    return URL;
  }

  if (LowerCaseEqualsASCII(parsed_scheme, "file")) {
    // This could theoretically be a strange search, but let's check.
    // If it's got an inner_url with a scheme, it's a URL, whether it's valid or
    // not.
    if (parts->inner_parsed() && parts->inner_parsed()->scheme.is_valid())
      return URL;
  }

  // If the user typed a scheme, and it's HTTP or HTTPS, we know how to parse it
  // well enough that we can fall through to the heuristics below.  If it's
  // something else, we can just determine our action based on what we do with
  // any input of this scheme.  In theory we could do better with some schemes
  // (e.g. "ftp" or "view-source") but I'll wait to spend the effort on that
  // until I run into some cases that really need it.
  if (parts->scheme.is_nonempty() &&
      !LowerCaseEqualsASCII(parsed_scheme, "http") &&
      !LowerCaseEqualsASCII(parsed_scheme, "https")) {
    // See if we know how to handle the URL internally.
    //if (ProfileIOData::IsHandledProtocol(UTF16ToASCII(parsed_scheme)))
    //  return URL;

    // There are also some schemes that we convert to other things before they
    // reach the renderer or else the renderer handles internally without
    // reaching the net::URLRequest logic.  We thus won't catch these above, but
    // we should still claim to handle them.
    /*if (LowerCaseEqualsASCII(parsed_scheme, content::kViewSourceScheme) ||
        LowerCaseEqualsASCII(parsed_scheme, content::kJavaScriptScheme) ||
        LowerCaseEqualsASCII(parsed_scheme, chrome::kDataScheme))
      return URL;*/

    // Finally, check and see if the user has explicitly opened this scheme as
    // a URL before, or if the "scheme" is actually a username.  We need to do
    // this last because some schemes (e.g. "javascript") may be treated as
    // "blocked" by the external protocol handler because we don't want pages to
    // open them, but users still can.
    // TODO(viettrungluu): get rid of conversion.
    /*ExternalProtocolHandler::BlockState block_state =
        ExternalProtocolHandler::GetBlockState(UTF16ToUTF8(parsed_scheme));
    switch (block_state) {
      case ExternalProtocolHandler::DONT_BLOCK:
        return URL;

      case ExternalProtocolHandler::BLOCK:
        // If we don't want the user to open the URL, don't let it be navigated
        // to at all.
        return QUERY;

      default: {
        // We don't know about this scheme.  It might be that the user typed a
        // URL of the form "username:password@foo.com".
        const string16 http_scheme_prefix =
            ASCIIToUTF16(std::string(content::kHttpScheme) +
                         content::kStandardSchemeSeparator);
        url_parse::Parsed http_parts;
        string16 http_scheme;
        GURL http_canonicalized_url;
        Type http_type = Parse(http_scheme_prefix + text, desired_tld,
                               &http_parts, &http_scheme,
                               &http_canonicalized_url);
        DCHECK_EQ(std::string(content::kHttpScheme), UTF16ToUTF8(http_scheme));

        if (http_type == URL &&
            http_parts.username.is_nonempty() &&
            http_parts.password.is_nonempty()) {
          // Manually re-jigger the parsed parts to match |text| (without the
          // http scheme added).
          http_parts.scheme.reset();
          url_parse::Component* components[] = {
            &http_parts.username,
            &http_parts.password,
            &http_parts.host,
            &http_parts.port,
            &http_parts.path,
            &http_parts.query,
            &http_parts.ref,
          };
          for (size_t i = 0; i < arraysize(components); ++i) {
            URLFixerUpper::OffsetComponent(
                -static_cast<int>(http_scheme_prefix.length()), components[i]);
          }

          *parts = http_parts;
          if (scheme)
            scheme->clear();
          if (canonicalized_url)
            *canonicalized_url = http_canonicalized_url;

          return http_type;
        }

        // We don't know about this scheme and it doesn't look like the user
        // typed a username and password.  It's likely to be a search operator
        // like "site:" or "link:".  We classify it as UNKNOWN so the user has
        // the option of treating it as a URL if we're wrong.
        // Note that SegmentURL() is smart so we aren't tricked by "c:\foo" or
        // "www.example.com:81" in this case.
        return UNKNOWN;
      }
    }*/

     return UNKNOWN;
  }

  // Either the user didn't type a scheme, in which case we need to distinguish
  // between an HTTP URL and a query, or the scheme is HTTP or HTTPS, in which
  // case we should reject invalid formulations.

  // If we have an empty host it can't be a URL.
  if (!parts->host.is_nonempty())
    return QUERY;

  // No scheme, password, port, path, and no known TLD on the host.
  // This could be:
  // * An "incomplete IP address"; likely a search (see above).
  // * An email-like input like "user@host", where "host" has no known TLD.
  //   It's not clear what the user means here and searching seems reasonable.
  // * A single word "foo"; possibly an intranet site, but more likely a search.
  //   This is ideally an UNKNOWN, and we can let the Alternate Nav URL code
  //   catch our mistakes.
  // * A URL with a valid TLD we don't know about yet.  If e.g. a registrar adds
  //   "xxx" as a TLD, then until we add it to our data file, Chrome won't know
  //   "foo.xxx" is a real URL.  So ideally this is a URL, but we can't really
  //   distinguish this case from:
  // * A "URL-like" string that's not really a URL (like
  //   "browser.tabs.closeButtons" or "java.awt.event.*").  This is ideally a
  //   QUERY.  Since this is indistinguishable from the case above, and this
  //   case is much more likely, claim these are UNKNOWN, which should default
  //   to the right thing and let users correct us on a case-by-case basis.
  return UNKNOWN;
}

// static
void AutocompleteInput::ParseForEmphasizeComponents(
    const string16& text,
    url_parse::Component* scheme,
    url_parse::Component* host) {
  url_parse::Parsed parts;
  string16 scheme_str;
  Parse(text, string16(), &parts, &scheme_str, NULL);

  *scheme = parts.scheme;
  *host = parts.host;

  int after_scheme_and_colon = parts.scheme.end() + 1;
  // For the view-source scheme, we should emphasize the scheme and host of the
  // URL qualified by the view-source prefix.
  if (LowerCaseEqualsASCII(scheme_str, "view:source") &&
      (static_cast<int>(text.length()) > after_scheme_and_colon)) {
    // Obtain the URL prefixed by view-source and parse it.
    string16 real_url(text.substr(after_scheme_and_colon));
    url_parse::Parsed real_parts;
    AutocompleteInput::Parse(real_url, string16(), &real_parts, NULL, NULL);
    if (real_parts.scheme.is_nonempty() || real_parts.host.is_nonempty()) {
      if (real_parts.scheme.is_nonempty()) {
        *scheme = url_parse::Component(
            after_scheme_and_colon + real_parts.scheme.begin,
            real_parts.scheme.len);
      } else {
        scheme->reset();
      }
      if (real_parts.host.is_nonempty()) {
        *host = url_parse::Component(
            after_scheme_and_colon + real_parts.host.begin,
            real_parts.host.len);
      } else {
        host->reset();
      }
    }
  } else if (LowerCaseEqualsASCII(scheme_str, "file") &&
             parts.inner_parsed() && parts.inner_parsed()->scheme.is_valid()) {
    *host = parts.inner_parsed()->host;
  }
}

// static
string16 AutocompleteInput::FormattedStringWithEquivalentMeaning(
    const GURL& url,
    const string16& formatted_url) {
  //if (!net::CanStripTrailingSlash(url))
  //  return formatted_url;
  const string16 url_with_path(formatted_url + char16('/'));
  return (AutocompleteInput::Parse(formatted_url, string16(), NULL, NULL,
                                   NULL) ==
          AutocompleteInput::Parse(url_with_path, string16(), NULL, NULL,
                                   NULL)) ?
      formatted_url : url_with_path;
}

// static
int AutocompleteInput::NumNonHostComponents(const url_parse::Parsed& parts) {
  int num_nonhost_components = 0;
  if (parts.scheme.is_nonempty())
    ++num_nonhost_components;
  if (parts.username.is_nonempty())
    ++num_nonhost_components;
  if (parts.password.is_nonempty())
    ++num_nonhost_components;
  if (parts.port.is_nonempty())
    ++num_nonhost_components;
  if (parts.path.is_nonempty())
    ++num_nonhost_components;
  if (parts.query.is_nonempty())
    ++num_nonhost_components;
  if (parts.ref.is_nonempty())
    ++num_nonhost_components;
  return num_nonhost_components;
}

void AutocompleteInput::UpdateText(const string16& text,
                                   size_t cursor_position,
                                   const url_parse::Parsed& parts) {
  DCHECK(cursor_position <= text.length() || cursor_position == string16::npos)
      << "Text: '" << text << "', cp: " << cursor_position;
  text_ = text;
  cursor_position_ = cursor_position;
  parts_ = parts;
}

void AutocompleteInput::Clear() {
  text_.clear();
  cursor_position_ = string16::npos;
  current_url_ = GURL();
  current_page_classification_ = AutocompleteInput::INVALID_SPEC;
  type_ = INVALID;
  parts_ = url_parse::Parsed();
  scheme_.clear();
  canonicalized_url_ = GURL();
  prevent_inline_autocomplete_ = false;
  prefer_keyword_ = false;
  allow_exact_keyword_match_ = false;
  matches_requested_ = ALL_MATCHES;
}
