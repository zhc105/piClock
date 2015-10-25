#ifndef _M_STRING_H_
#define _M_STRING_H_

#include <string>
#include <sstream>

template<typename T> 
	inline std::string to_str(const T& t)
	{
		std::ostringstream s;
		s << t;
		return s.str();
	}

template<typename T> 
	inline T from_str(const std::string& s)
	{
		std::istringstream is(s);
		T t;
		is >> t;
		return t;
	}

inline std::string& ltrim(std::string &s)
{
	int pos, len = s.length();
	for (pos = 0; pos < len; pos++)
		if (s[pos] != ' ' && s[pos] != '\t' && s[pos] != '\r' && s[pos] != '\n')
			break;
	s = s.substr(pos);
	return s;
}

inline std::string& rtrim(std::string &s)
{
	int pos;
	for (pos = s.length() - 1; pos >= 0; --pos)
		if (s[pos] != ' ' && s[pos] != '\t' && s[pos] != '\r' && s[pos] != '\n')
			break;
	s = s.substr(0, pos + 1);
	return s;
}

inline std::string& trim(std::string &s)
{
	return ltrim(rtrim(s));
}

std::string mem2hex(const char *buf, int len);
void hex2mem(const std::string &hex, char *buf, int buflen);

std::string& str_upcase(std::string &str);
std::string& str_lowcase(std::string &str);

std::string url_escape(const std::string &url);
std::string url_encode(const std::string &str);
std::string url_decode(const std::string &str);

#endif
