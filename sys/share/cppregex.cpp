/* NetHack 5.0  cppregex.cpp */
/* $NHDT-Date: 1596498279 2020/08/03 23:44:39 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.9 $ */
/* Copyright (c) Sean Hunt  2015.                                 */
/* NetHack may be freely redistributed.  See license for details. */

#include <regex>
#include <memory>
#include <cstring>

#if 1 /*JP*/
#include <windows.h>
#define SKIP_BOOLEAN /* already defined */
static std::wstring s2w(const char *s) {
  std::string src = std::string(s);
  auto const dest_size = ::MultiByteToWideChar(CP_ACP, 0U, src.data(), -1, nullptr, 0U);
  std::vector<wchar_t> dest(dest_size, L'\0');
  if (::MultiByteToWideChar(CP_ACP, 0U, src.data(), -1, dest.data(), dest.size()) == 0) {
    return std::wstring(L""); /* should not happen */
  }
  dest.resize(std::char_traits<wchar_t>::length(dest.data()));
  dest.shrink_to_fit();
  return std::wstring(dest.begin(), dest.end());
  }
#endif

extern "C" {
#include "config.h"
#define CPPREGEX_C
#include "nhregex.h"
} // extern "C"


extern "C" { // rest of file

/* nhregex interface documented in sys/share/posixregex.c */

extern const char regex_id[] = "cppregex";

struct nhregex {
#if 0 /*JP*/
    std::unique_ptr<std::regex> re;
#else
    std::unique_ptr<std::wregex> re;
#endif
    std::unique_ptr<std::regex_error> err;
};

struct nhregex *
regex_init(void)
{
    return new nhregex;
}

boolean
regex_compile(const char *s, struct nhregex *re)
{
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

char *
regex_error_desc(struct nhregex *re, char *errbuf)
{
    if (!re) {
        Strcpy(errbuf, "no regexp");
    } else if (!re->err) {
        Strcpy(errbuf, "no explanation");
    } else {
        errbuf[0] = '\0';
        (void) strncat(errbuf, re->err->what(), BUFSZ - 1);
        if (!errbuf[0])
            Strcpy(errbuf, "unspecified regexp error");
    }
    return errbuf;
}

boolean
regex_match(const char *s, struct nhregex *re)
{
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

void
regex_free(struct nhregex *re)
{
    delete re;
}
#undef CPPREGEX_C
} // extern "C"

/*cppregex.cpp*/
