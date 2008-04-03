
//         Copyright E�in O'Callaghan 2006 - 2007.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef GLOBAL_STRING_CONV
#define GLOBAL_STRING_CONV

#include <string>
#include <boost/format.hpp>
#include <boost/array.hpp>
#include "unicode.hpp"

namespace hal
{

inline std::string to_utf8(const std::wstring& wstr) 
{ 
	std::string ustr;

	unicode::transcode<unicode::wchar_encoding, unicode::utf8>(
	   wstr.begin(),
	   wstr.end(),
	   std::insert_iterator<std::string>(ustr, ustr.end())
	);

	return ustr;
}

inline std::wstring from_utf8(const std::string& ustr) 
{ 
	std::wstring wstr;
	
	unicode::transcode<unicode::utf8, unicode::wchar_encoding>(
	   ustr.begin(),
	   ustr.end(),
	   std::insert_iterator<std::wstring>(wstr, wstr.end())
	);

	return wstr; 
}

inline std::wstring from_ascii(const std::string& s)
{
	std::wstring ret;

	for (const char *i = &s[0], *end = &s[0] + s.size(); i < end;)
	{
		wchar_t c = '.';
		int result = std::mbtowc(&c, i, end - i);
		if (result > 0) i += result;
		else ++i;
		ret += c;
	}

	return ret;
}

inline std::wstring from_utf8_safe(const std::string& s)
{
	try
	{
		return from_utf8(s);
	}
	catch (std::exception&)
	{
		return from_ascii(s);
	}
}

template<typename str_t>
inline std::wstring to_wstr_shim(str_t& s)
{
	return s;
}

template<>
inline std::wstring to_wstr_shim<const wchar_t *const>(const wchar_t *const& s)
{
	return std::wstring(s);
}

template<>
inline std::wstring to_wstr_shim<const wchar_t *>(const wchar_t *& s)
{
	return std::wstring(s);
}

template<>
inline std::wstring to_wstr_shim<const char *>(const char *& s)
{
	return from_utf8_safe(s);
}

template<>
inline std::wstring to_wstr_shim<const char *const>(const char *const& s)
{
	return from_utf8_safe(s);
}

template<>
inline std::wstring to_wstr_shim<const std::string>(const std::string& s)
{
	return from_utf8_safe(s);
}

template<>
inline std::wstring to_wstr_shim<std::string>(std::string& s)
{
	return from_utf8_safe(s);
}

template<>
inline std::wstring to_wstr_shim<const boost::wformat>(const boost::wformat& f)
{
	return f.str();
}

template<>
inline std::wstring to_wstr_shim<boost::wformat>(boost::wformat& f)
{
	return f.str();
}

template<>
inline std::wstring to_wstr_shim<const boost::format>(const boost::format& f)
{
	return from_utf8_safe(f.str());
}

template<>
inline std::wstring to_wstr_shim<boost::format>(boost::format& f)
{
	return from_utf8_safe(f.str());
}

} // namespace aux

#endif // GLOBAL_STRING_CONV