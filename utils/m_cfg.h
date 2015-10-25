/*
 *
 *   配置文件解析 1.01
 *
 *	 zhc105 2014-10-31
 *
 *   格式样例:
 *   <root>
 *      test = 123
 *      # sub path
 *      <sub>
 *         t2 = 456
 *         t3 = abc
 *      </sub>
 *   </root>
 *
 */

#ifndef _M_CFG_H_
#define _M_CFG_H_

#include "m_exception.h"
#include <cstdio>
#include <string>
#include <fstream>
#include <vector>
#include <stack>
#include <map>

class CMCfg;
class CMLexer;
class CMToken;

class CMCfg
{
public:
	CMCfg(std::string cfg_file)
	{
		_cfg_file = cfg_file;
	}
	~CMCfg() {};

	bool parse();

private:
	void page();
	void block();
	void match(int type, std::string *key = NULL);

public:
	std::map<std::string, std::string> _page;
	std::map<std::string, std::vector<std::string> > _raw;
	std::string _cfg_file;

private:
	CMLexer *_lex;
	CMToken *_token;
	std::stack<std::string> _prev_path;
	std::string _now_path;
	char _buf[1024];

};

class CMLexer
{
public:
	enum LexerState
	{
		STATE_BASE = 1,
		STATE_BLOCK,
		STATE_BLOCK_START,
		STATE_BLOCK_END,
		STATE_COMMENT,
		STATE_COMMENT_HALF,
		STATE_KEY,
		STATE_END
	};

public:
	CMLexer(std::string cfg_file);
	~CMLexer();

	CMToken* next_token();

private:
	std::fstream fin;

	int _peek;
	CMToken *_token;

};

class CMToken
{
public:
	static const int TOKEN_NONE		   = 0;
	static const int TOKEN_BLOCK_START = 1;
	static const int TOKEN_BLOCK_END   = 2;
	static const int TOKEN_KEY         = 3;
	static const int TOKEN_ASSIGN      = 4;
	static const int TOKEN_EOL         = 5;
	static const int TOKEN_OTHER       = 100;
	static const int TOKEN_EOF         = -1;

	CMToken(int type, std::string key)
	{
		_type = type;
		_key  = key;
	}

public:
	int _type;
	std::string _key;

};

#endif
