#ifndef _M_EXCEPTION_H_
#define _M_EXCEPTION_H_

#include <cstdlib>
#include <stdexcept>
#include <execinfo.h>

class CMException : public std::exception
{
public:
	CMException(const std::string& s, bool trace = true) : _err_str(s)
	{
		if (trace)
		{
			void *array[64];
			int nSize = backtrace(array, 64);
			char **symbols = backtrace_symbols(array, nSize);
			for (int i = 0; i < nSize; i++)
				_bt_str.append(symbols[i]).append("\n");
			free(symbols);
		}
	}
	virtual ~CMException() throw() {}
	virtual const char* what() const throw() {return _err_str.c_str();}
	virtual const char* where() const throw() {return _bt_str.c_str();}

protected:
	std::string _bt_str;
	std::string _err_str;

};
#endif
