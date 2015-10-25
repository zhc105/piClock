#include "m_string.h"
#include <algorithm>
#include <cassert>

using namespace std;

unsigned char ToHex(unsigned char x) 
{ 
    return  x > 9 ? x + 55 : x + 48; 
}

unsigned char FromHex(unsigned char x) 
{ 
    unsigned char y = 0;
    if (x >= 'A' && x <= 'Z') y = x - 'A' + 10;
    else if (x >= 'a' && x <= 'z') y = x - 'a' + 10;
    else if (x >= '0' && x <= '9') y = x - '0';
    return y;
}

string mem2hex(const char *buf, int len)
{
	static char pUpperHex[20] = "0123456789ABCDEF";

	std::ostringstream s;
	for(; len; --len, ++buf)
		s << pUpperHex[*buf >> 4 & 0xF] << pUpperHex[*buf & 0xF];

	return s.str();
}

void hex2mem(const std::string &hex, char *buf, int buflen)
{
	for (int i = 0; i < (int) hex.length() / 2 && i < buflen; i++)
	{
		buf[i] = (FromHex(hex[i << 1]) << 4) | (FromHex(hex[(i << 1) + 1]));
	}
}

int to_upper(int c)  
{  
	if (islower(c))  
		return c - 32;   
	return c;  
}

int to_lower(int c)
{
	if (isupper(c))
		return c + 32;
	return c;
}

string& str_upcase(string &str)
{
	transform(str.begin(), str.end(), str.begin(), to_upper);
	return str;
}

string& str_lowcase(string &str)
{
	transform(str.begin(), str.end(), str.begin(), to_lower);
	return str;
}

std::string url_escape(const std::string& url)
{
    std::string strTemp = "";
    size_t length = url.length();
    for (size_t i = 0; i < length; i++)
    {
        if (isalnum((unsigned char) url[i]) || 
            (url[i] == '-') || (url[i] == ':') ||
            (url[i] == '_') || (url[i] == '/') || 
            (url[i] == '.') || (url[i] == '@') || 
            (url[i] == '~') || (url[i] == '%'))
		{
            strTemp += url[i];
		}
        else if (url[i] == '?')
		{
            strTemp += url.substr(i);
			break;
		}
        else
        {
            strTemp += '%';
            strTemp += ToHex((unsigned char)url[i] >> 4);
            strTemp += ToHex((unsigned char)url[i] % 16);
        }
    }
    return strTemp;
}

std::string url_encode(const std::string& str)
{
    std::string strTemp = "";
    size_t length = str.length();
    for (size_t i = 0; i < length; i++)
    {
        if (isalnum((unsigned char)str[i]) || 
            (str[i] == '-') ||
            (str[i] == '_') || 
            (str[i] == '.') || 
            (str[i] == '~'))
            strTemp += str[i];
        else if (str[i] == ' ')
            strTemp += "+";
        else
        {
            strTemp += '%';
            strTemp += ToHex((unsigned char)str[i] >> 4);
            strTemp += ToHex((unsigned char)str[i] % 16);
        }
    }
    return strTemp;
}

std::string url_decode(const std::string& str)
{
    std::string strTemp = "";
    size_t length = str.length();
    for (size_t i = 0; i < length; i++)
    {
        if (str[i] == '+') strTemp += ' ';
        else if (str[i] == '%')
        {
            assert(i + 2 < length);
            unsigned char high = FromHex((unsigned char)str[++i]);
            unsigned char low = FromHex((unsigned char)str[++i]);
            strTemp += high*16 + low;
        }
        else strTemp += str[i];
    }
    return strTemp;
}
