// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef URL_GURL_H_
#define URL_GURL_H_

#include <iosfwd>
#include <string>

#include "base/strings/string16.h"
#include "base/strings/utf_string_conversions.h"

struct OpenURLParams
{

};

class GURL {
 public:
  // Creates an empty, invalid URL.
   GURL(){};

  // Copy construction is relatively inexpensive, with most of the time going
  // to reallocating the string. It does not re-parse.
   GURL(const GURL& other){
     spec_ = other.spec_;
     inner_url_ = NULL;
   };

  // The narrow version requires the input be UTF-8. Invalid UTF-8 input will
  // result in an invalid URL.
  //
  // The wide version should also take an encoding parameter so we know how to
  // encode the query parameters. It is probably sufficient for the narrow
  // version to assume the query parameter encoding should be the same as the
  // input encoding.
   explicit GURL(const std::string& url_string /*, output_param_encoding*/){
     spec_ = url_string;
     inner_url_ = NULL;
   };
   explicit GURL(const base::string16& url_string /*, output_param_encoding*/){
    spec_ = WideToUTF8(url_string);
    inner_url_ = NULL;
   };

  // Constructor for URLs that have already been parsed and canonicalized. This
  // is used for conversions from KURL, for example. The caller must supply all
  // information associated with the URL, which must be correct and consistent.
  //GURL(const char* canonical_spec, size_t canonical_spec_len,
  //     const url_parse::Parsed& parsed, bool is_valid);
  // Notice that we take the canonical_spec by value so that we can convert
  // from WebURL without copying the string. When we call this constructor
  // we pass in a temporary std::string, which lets the compiler skip the
  // copy and just move the std::string into the function argument. In the
  // implementation, we use swap to move the data into the GURL itself,
  // which means we end up with zero copies.
  //GURL(std::string canonical_spec,
  //     const url_parse::Parsed& parsed, bool is_valid);

   ~GURL(){};

  GURL& operator=(const GURL& other){
    spec_ = other.spec_;
    inner_url_ = other.inner_url_;
    return *this;
  };

  // Returns true when this object represents a valid parsed URL. When not
  // valid, other functions will still succeed, but you will not get canonical
  // data out in the format you may be expecting. Instead, we keep something
  // "reasonable looking" so that the user can see how it's busted if
  // displayed to them.
  bool is_valid() const {
    return spec_.length() > 0;//is_valid_;
  }

  // Returns true if the URL is zero-length. Note that empty URLs are also
  // invalid, and is_valid() will return false for them. This is provided
  // because some users may want to treat the empty case differently.
  bool is_empty() const {
    return spec_.empty();
  }

  // Returns the raw spec, i.e., the full text of the URL, in canonical UTF-8,
  // if the URL is valid. If the URL is not valid, this will assert and return
  // the empty string (for safety in release builds, to keep them from being
  // misused which might be a security problem).
  //
  // The URL will be ASCII except the reference fragment, which may be UTF-8.
  // It is guaranteed to be valid UTF-8.
  //
  // The exception is for empty() URLs (which are !is_valid()) but this will
  // return the empty string without asserting.
  //
  // Used invalid_spec() below to get the unusable spec of an invalid URL. This
  // separation is designed to prevent errors that may cause security problems
  // that could result from the mistaken use of an invalid URL.
  const std::string& spec() const{return spec_;};

  // Returns the potentially invalid spec for a the URL. This spec MUST NOT be
  // modified or sent over the network. It is designed to be displayed in error
  // messages to the user, as the apperance of the spec may explain the error.
  // If the spec is valid, the valid spec will be returned.
  //
  // The returned string is guaranteed to be valid UTF-8.
  const std::string& possibly_invalid_spec() const {
    return spec_;
  }

  // Getter for the raw parsed structure. This allows callers to locate parts
  // of the URL within the spec themselves. Most callers should consider using
  // the individual component getters below.
  //
  // The returned parsed structure will reference into the raw spec, which may
  // or may not be valid. If you are using this to index into the spec, BE
  // SURE YOU ARE USING possibly_invalid_spec() to get the spec, and that you
  // don't do anything "important" with invalid specs.
//   const url_parse::Parsed& parsed_for_possibly_invalid_spec() const {
//     return parsed_;
//   }

  // Defiant equality operator!
  bool operator==(const GURL& other) const {
    return spec_ == other.spec_;
  }
  bool operator!=(const GURL& other) const {
    return spec_ != other.spec_;
  }

  // Allows GURL to used as a key in STL (for example, a std::set or std::map).
  bool operator<(const GURL& other) const {
    return spec_ < other.spec_;
  }

  // Resolves a URL that's possibly relative to this object's URL, and returns
  // it. Absolute URLs are also handled according to the rules of URLs on web
  // pages.
  //
  // It may be impossible to resolve the URLs properly. If the input is not
  // "standard" (SchemeIsStandard() == false) and the input looks relative, we
  // can't resolve it. In these cases, the result will be an empty, invalid
  // GURL.
  //
  // The result may also be a nonempty, invalid URL if the input has some kind
  // of encoding error. In these cases, we will try to construct a "good" URL
  // that may have meaning to the user, but it will be marked invalid.
  //
  // It is an error to resolve a URL relative to an invalid URL. The result
  // will be the empty URL.
  GURL Resolve(const std::string& relative) const{return GURL();};
  GURL Resolve(const base::string16& relative) const{return GURL();};

  // Like Resolve() above but takes a character set encoder which will be used
  // for any query text specified in the input. The charset converter parameter
  // may be NULL, in which case it will be treated as UTF-8.
  //
  // TODO(brettw): These should be replaced with versions that take something
  // more friendly than a raw CharsetConverter (maybe like an ICU character set
  // name).
  /*GURL ResolveWithCharsetConverter(
      const std::string& relative,
      url_canon::CharsetConverter* charset_converter) const;
  GURL ResolveWithCharsetConverter(
      const base::string16& relative,
      url_canon::CharsetConverter* charset_converter) const;*/

  // Creates a new GURL by replacing the current URL's components with the
  // supplied versions. See the Replacements class in url_canon.h for more.
  //
  // These are not particularly quick, so avoid doing mutations when possible.
  // Prefer the 8-bit version when possible.
  //
  // It is an error to replace components of an invalid URL. The result will
  // be the empty URL.
  //
  // Note that we use the more general url_canon::Replacements type to give
  // callers extra flexibility rather than our override.
  /*GURL ReplaceComponents(
      const url_canon::Replacements<char>& replacements) const;
  GURL ReplaceComponents(
      const url_canon::Replacements<base::char16>& replacements) const;*/

  // A helper function that is equivalent to replacing the path with a slash
  // and clearing out everything after that. We sometimes need to know just the
  // scheme and the authority. If this URL is not a standard URL (it doesn't
  // have the regular authority and path sections), then the result will be
  // an empty, invalid GURL. Note that this *does* work for file: URLs, which
  // some callers may want to filter out before calling this.
  //
  // It is an error to get an empty path on an invalid URL. The result
  // will be the empty URL.
  GURL GetWithEmptyPath() const{return GURL();};

  // A helper function to return a GURL containing just the scheme, host,
  // and port from a URL. Equivalent to clearing any username and password,
  // replacing the path with a slash, and clearing everything after that. If
  // this URL is not a standard URL, then the result will be an empty,
  // invalid GURL. If the URL has neither username nor password, this
  // degenerates to GetWithEmptyPath().
  //
  // It is an error to get the origin of an invalid URL. The result
  // will be the empty URL.
  GURL GetOrigin() const{return GURL();}

  // Returns true if the scheme for the current URL is a known "standard"
  // scheme. Standard schemes have an authority and a path section. This
  // includes file: and filesystem:, which some callers may want to filter out
  // explicitly by calling SchemeIsFile[System].
  bool IsStandard() const{return false;}

  // Returns true if the given parameter (should be lower-case ASCII to match
  // the canonicalized scheme) is the scheme for this URL. This call is more
  // efficient than getting the scheme and comparing it because no copies or
  // object constructions are done.
  bool SchemeIs(const char* lower_ascii_scheme) const{return false;}

  // Returns true if the scheme is "http" or "https".
  bool SchemeIsHTTPOrHTTPS() const{return false;}

  // We often need to know if this is a file URL. File URLs are "standard", but
  // are often treated separately by some programs.
  bool SchemeIsFile() const {
    return SchemeIs("file");
  }

  // FileSystem URLs need to be treated differently in some cases.
  bool SchemeIsFileSystem() const {
    return SchemeIs("filesystem");
  }

  // If the scheme indicates a secure connection
  bool SchemeIsSecure() const {
    return SchemeIs("https") || SchemeIs("wss") ||
        (SchemeIsFileSystem() && inner_url() && inner_url()->SchemeIsSecure());
  }

  // Returns true if the hostname is an IP address. Note: this function isn't
  // as cheap as a simple getter because it re-parses the hostname to verify.
  // This currently identifies only IPv4 addresses (bug 822685).
  bool HostIsIPAddress() const{return false;}

  // Getters for various components of the URL. The returned string will be
  // empty if the component is empty or is not present.
  std::string scheme() const {  // Not including the colon. See also SchemeIs.
    return "";//ComponentString(parsed_.scheme);
  }
  std::string username() const {
    return "";//ComponentString(parsed_.username);
  }
  std::string password() const {
    return "";//ComponentString(parsed_.password);
  }
  // Note that this may be a hostname, an IPv4 address, or an IPv6 literal
  // surrounded by square brackets, like "[2001:db8::1]".  To exclude these
  // brackets, use HostNoBrackets() below.
  std::string host() const {
    return "";//ComponentString(parsed_.host);
  }
  std::string port() const {  // Returns -1 if "default"
    return "";//ComponentString(parsed_.port);
  }
  std::string path() const {  // Including first slash following host
    return "";//ComponentString(parsed_.path);
  }
  std::string query() const {  // Stuff following '?'
    return "";//ComponentString(parsed_.query);
  }
  std::string ref() const {  // Stuff following '#'
    return "";//ComponentString(parsed_.ref);
  }

  // Existance querying. These functions will return true if the corresponding
  // URL component exists in this URL. Note that existance is different than
  // being nonempty. http://www.google.com/? has a query that just happens to
  // be empty, and has_query() will return true.
  bool has_scheme() const {
    return false;//parsed_.scheme.len >= 0;
  }
  bool has_username() const {
    return false;//parsed_.username.len >= 0;
  }
  bool has_password() const {
    return false;//parsed_.password.len >= 0;
  }
  bool has_host() const {
    // Note that hosts are special, absense of host means length 0.
    return false;//parsed_.host.len > 0;
  }
  bool has_port() const {
    return false;//parsed_.port.len >= 0;
  }
  bool has_path() const {
    // Note that http://www.google.com/" has a path, the path is "/". This can
    // return false only for invalid or nonstandard URLs.
    return false;//parsed_.path.len >= 0;
  }
  bool has_query() const {
    return false;//parsed_.query.len >= 0;
  }
  bool has_ref() const {
    return false;//parsed_.ref.len >= 0;
  }

  // Returns a parsed version of the port. Can also be any of the special
  // values defined in Parsed for ExtractPort.
  int IntPort() const{return 0;}

  // Returns the port number of the url, or the default port number.
  // If the scheme has no concept of port (or unknown default) returns
  // PORT_UNSPECIFIED.
  int EffectiveIntPort() const{return 0;}

  // Extracts the filename portion of the path and returns it. The filename
  // is everything after the last slash in the path. This may be empty.
  std::string ExtractFileName() const{return "";}

  // Returns the path that should be sent to the server. This is the path,
  // parameter, and query portions of the URL. It is guaranteed to be ASCII.
  std::string PathForRequest() const{return "";}

  // Returns the host, excluding the square brackets surrounding IPv6 address
  // literals.  This can be useful for passing to getaddrinfo().
  std::string HostNoBrackets() const{return "";}

  // Returns true if this URL's host matches or is in the same domain as
  // the given input string. For example if this URL was "www.google.com",
  // this would match "com", "google.com", and "www.google.com
  // (input domain should be lower-case ASCII to match the canonicalized
  // scheme). This call is more efficient than getting the host and check
  // whether host has the specific domain or not because no copies or
  // object constructions are done.
  //
  // If function DomainIs has parameter domain_len, which means the parameter
  // lower_ascii_domain does not gurantee to terminate with NULL character.
  bool DomainIs(const char* lower_ascii_domain, int domain_len) const {return false;}

  // If function DomainIs only has parameter lower_ascii_domain, which means
  // domain string should be terminate with NULL character.
  bool DomainIs(const char* lower_ascii_domain) const {
    return DomainIs(lower_ascii_domain,
                    static_cast<int>(strlen(lower_ascii_domain)));
  }

  // Swaps the contents of this GURL object with the argument without doing
  // any memory allocations.
  void Swap(GURL* other){}

  // Returns a reference to a singleton empty GURL. This object is for callers
  // who return references but don't have anything to return in some cases.
  // This function may be called from any thread.
  //static const GURL& EmptyGURL(){}

  // Returns the inner URL of a nested URL [currently only non-null for
  // filesystem: URLs].
  const GURL* inner_url() const {
    return inner_url_;
  }

  void set_post_data(const std::string& post_data) { post_data_ = post_data;}
  std::string post_data() const{ return post_data_;}

 private:
   void InitializeFromCanonicalSpec(){};

  // Returns the substring of the input identified by the given component.
//   std::string ComponentString(const url_parse::Component& comp) const {
//     if (comp.len <= 0)
//       return std::string();
//     return std::string(spec_, comp.begin, comp.len);
//   }

  // The actual text of the URL, in canonical ASCII form.
  std::string spec_;
  std::string post_data_;

  // Set when the given URL is valid. Otherwise, we may still have a spec and
  // components, but they may not identify valid resources (for example, an
  // invalid port number, invalid characters in the scheme, etc.).
  bool is_valid_;

  // Identified components of the canonical spec.
  //url_parse::Parsed parsed_;

  // Used for nested schemes [currently only filesystem:].
  GURL* inner_url_;

  // TODO bug 684583: Add encoding for query params.
};

namespace url_parse {

  // Deprecated, but WebKit/WebCore/platform/KURLGooglePrivate.h and
  // KURLGoogle.cpp still rely on this type.
  typedef base::char16 UTF16Char;

  // Component ------------------------------------------------------------------

  // Represents a substring for URL parsing.
  struct Component {
    Component() : begin(0), len(-1) {}

    // Normal constructor: takes an offset and a length.
    Component(int b, int l) : begin(b), len(l) {}

    int end() const {
      return begin + len;
    }

    // Returns true if this component is valid, meaning the length is given. Even
    // valid components may be empty to record the fact that they exist.
    bool is_valid() const {
      return (len != -1);
    }

    // Returns true if the given component is specified on false, the component
    // is either empty or invalid.
    bool is_nonempty() const {
      return (len > 0);
    }

    void reset() {
      begin = 0;
      len = -1;
    }

    bool operator==(const Component& other) const {
      return begin == other.begin && len == other.len;
    }

    int begin;  // Byte offset in the string of this component.
    int len;    // Will be -1 if the component is unspecified.
  };

  // Helper that returns a component created with the given begin and ending
  // points. The ending point is non-inclusive.
  inline Component MakeRange(int begin, int end) {
    return Component(begin, end - begin);
  }

  // Parsed ---------------------------------------------------------------------

  // A structure that holds the identified parts of an input URL. This structure
  // does NOT store the URL itself. The caller will have to store the URL text
  // and its corresponding Parsed structure separately.
  //
  // Typical usage would be:
  //
  //    url_parse::Parsed parsed;
  //    url_parse::Component scheme;
  //    if (!url_parse::ExtractScheme(url, url_len, &scheme))
  //      return I_CAN_NOT_FIND_THE_SCHEME_DUDE;
  //
  //    if (IsStandardScheme(url, scheme))  // Not provided by this component
  //      url_parseParseStandardURL(url, url_len, &parsed);
  //    else if (IsFileURL(url, scheme))    // Not provided by this component
  //      url_parse::ParseFileURL(url, url_len, &parsed);
  //    else
  //      url_parse::ParsePathURL(url, url_len, &parsed);
  //
  struct Parsed {
    // Identifies different components.
    enum ComponentType {
      SCHEME,
      USERNAME,
      PASSWORD,
      HOST,
      PORT,
      PATH,
      QUERY,
      REF,
    };

    // The default constructor is sufficient for the components, but inner_parsed_
    // requires special handling.
    Parsed(){};
    Parsed(const Parsed&){};
    Parsed& operator=(const Parsed&){return *this;};
    ~Parsed(){};

    // Returns the length of the URL (the end of the last component).
    //
    // Note that for some invalid, non-canonical URLs, this may not be the length
    // of the string. For example "http://": the parsed structure will only
    // contain an entry for the four-character scheme, and it doesn't know about
    // the "://". For all other last-components, it will return the real length.
    int Length() const { return 0; }

    // Returns the number of characters before the given component if it exists,
    // or where the component would be if it did exist. This will return the
    // string length if the component would be appended to the end.
    //
    // Note that this can get a little funny for the port, query, and ref
    // components which have a delimiter that is not counted as part of the
    // component. The |include_delimiter| flag controls if you want this counted
    // as part of the component or not when the component exists.
    //
    // This example shows the difference between the two flags for two of these
    // delimited components that is present (the port and query) and one that
    // isn't (the reference). The components that this flag affects are marked
    // with a *.
    //                 0         1         2
    //                 012345678901234567890
    // Example input:  http://foo:80/?query
    //              include_delim=true,  ...=false  ("<-" indicates different)
    //      SCHEME: 0                    0
    //    USERNAME: 5                    5
    //    PASSWORD: 5                    5
    //        HOST: 7                    7
    //       *PORT: 10                   11 <-
    //        PATH: 13                   13
    //      *QUERY: 14                   15 <-
    //        *REF: 20                   20
    //
    int CountCharactersBefore(ComponentType type, bool include_delimiter) const{ return 0;};

    // Scheme without the colon: "http://foo"/ would have a scheme of "http".
    // The length will be -1 if no scheme is specified ("foo.com"), or 0 if there
    // is a colon but no scheme (":foo"). Note that the scheme is not guaranteed
    // to start at the beginning of the string if there are preceeding whitespace
    // or control characters.
    Component scheme;

    // Username. Specified in URLs with an @ sign before the host. See |password|
    Component username;

    // Password. The length will be -1 if unspecified, 0 if specified but empty.
    // Not all URLs with a username have a password, as in "http://me@host/".
    // The password is separated form the username with a colon, as in
    // "http://me:secret@host/"
    Component password;

    // Host name.
    Component host;

    // Port number.
    Component port;

    // Path, this is everything following the host name. Length will be -1 if
    // unspecified. This includes the preceeding slash, so the path on
    // http://www.google.com/asdf" is "/asdf". As a result, it is impossible to
    // have a 0 length path, it will be -1 in cases like "http://host?foo".
    // Note that we treat backslashes the same as slashes.
    Component path;

    // Stuff between the ? and the # after the path. This does not include the
    // preceeding ? character. Length will be -1 if unspecified, 0 if there is
    // a question mark but no query string.
    Component query;

    // Indicated by a #, this is everything following the hash sign (not
    // including it). If there are multiple hash signs, we'll use the last one.
    // Length will be -1 if there is no hash sign, or 0 if there is one but
    // nothing follows it.
    Component ref;

    // This is used for nested URL types, currently only filesystem.  If you
    // parse a filesystem URL, the resulting Parsed will have a nested
    // inner_parsed_ to hold the parsed inner URL's component information.
    // For all other url types [including the inner URL], it will be NULL.
    Parsed* inner_parsed() const {
      return inner_parsed_;
    }

    void set_inner_parsed(const Parsed& inner_parsed) {
      if (!inner_parsed_)
        inner_parsed_ = new Parsed(inner_parsed);
      else
        *inner_parsed_ = inner_parsed;
    }

    void clear_inner_parsed() {
      if (inner_parsed_) {
        delete inner_parsed_;
        inner_parsed_ = NULL;
      }
    }

  private:
    Parsed* inner_parsed_;  // This object is owned and managed by this struct.
  };
}
// Stream operator so GURL can be used in assertion statements.
//std::ostream& operator<<(std::ostream& out, const GURL& url);

#endif  // URL_GURL_H_
