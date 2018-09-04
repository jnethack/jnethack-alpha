/* NetHack 3.6  cppregex.cpp */
/* $NHDT-Date: 1524684157 2018/04/25 19:22:37 $  $NHDT-Branch: NetHack-3.6.0 $:$NHDT-Revision: 1.8 $ */
/* Copyright (c) Sean Hunt  2015.                                 */
/* NetHack may be freely redistributed.  See license for details. */

#include <regex>
#include <memory>
#include <windows.h>

/* nhregex interface documented in sys/share/posixregex.c */

#if 1 /*JP*/
static std::wstring s2w(const char *s) {
  std::string src = std::string(s);
  auto const dest_size = ::MultiByteToWideChar(CP_ACP, 0U, src.data(), -1, nullptr, 0U);
  std::vector<wchar_t> dest(dest_size, L'\0');
  if (::MultiByteToWideChar(CP_ACP, 0U, src.data(), -1, dest.data(), dest.size()) == 0) {
    return std::wstring(L"");
  }
  dest.resize(std::char_traits<wchar_t>::length(dest.data()));
  dest.shrink_to_fit();
  return std::wstring(dest.begin(), dest.end());
  }
#endif

extern "C" {
#if 0 /*JP*/
  #include <hack.h>
#endif

  extern const char regex_id[] = "cppregex";

  struct nhregex {
#if 0 /*JP*/
    std::unique_ptr<std::regex> re;
#else
    std::unique_ptr<std::wregex> re;
#endif
    std::unique_ptr<std::regex_error> err;
  };

  struct nhregex *regex_init(void) {
    return new nhregex;
  }

  boolean regex_compile(const char *s, struct nhregex *re) {
    if (!re)
      return FALSE;
    try {
#if 0 /*JP*/
      re->re.reset(new std::regex(s, (std::regex::extended
                                    | std::regex::nosubs
                                    | std::regex::optimize)));
#else
      re->re.reset(new std::wregex(s2w(s), (std::regex::extended
                                     | std::regex::nosubs
                                     | std::regex::optimize)));
#endif
      re->err.reset(nullptr);
      return TRUE;
    } catch (const std::regex_error& err) {
      re->err.reset(new std::regex_error(err));
      re->re.reset(nullptr);
      return FALSE;
    }
  }

  const char *regex_error_desc(struct nhregex *re) {
    if (re->err)
      return re->err->what();
    else
      return nullptr;
  }

  boolean regex_match(const char *s, struct nhregex *re) {
    if (!re->re)
      return false;
    try {
#if 0 /*JP*/
      return regex_search(s, *re->re, std::regex_constants::match_any);
#else
      return regex_search(s2w(s), *re->re, std::regex_constants::match_any);
#endif
    } catch (const std::regex_error& err) {
      return false;
    }
  }

  void regex_free(struct nhregex *re) {
    delete re;
  }
}
