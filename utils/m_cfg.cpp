#include "m_cfg.h"
#include "m_string.h"
#include <cstdio>

using namespace std;

bool CMCfg::parse()
{
	bool res = true;

	try
	{
		_lex = new CMLexer(_cfg_file);
		_token = _lex->next_token();

		_now_path = "";
		page();
		if (_token->_type != CMToken::TOKEN_EOF)
		{
			snprintf(_buf, sizeof(_buf), "unexpected token \"%s\".", _token->_key.c_str());
			throw CMException(_buf);
		}
	}
	catch (exception &ex)
	{
		fprintf(stderr, "config file \"%s\" parse error: %s\n", 
			_cfg_file.c_str(), ex.what());
		res = false;
	}
	
	delete _lex;
	return res;
}

void CMCfg::page()
{
	while (_token->_type != CMToken::TOKEN_EOF)
	{
		if (_token->_type == CMToken::TOKEN_KEY)
		{
			string key, value;
			match(CMToken::TOKEN_KEY, &key);
			if (_token->_type == CMToken::TOKEN_ASSIGN)
			{
				match(CMToken::TOKEN_ASSIGN);
				if (_token->_type != CMToken::TOKEN_KEY)
				{
					// page => KEY= page
					value = "";
				}
				else
				{
					// page => KEY=KEY page
					match(CMToken::TOKEN_KEY, &value);
				}
				_page[_now_path + key] = value;
			}
			else
			{
				// page => KEY page  (raw)
				if (_raw.find(_now_path) == _raw.end())
				{
					_raw[_now_path] = vector<string>();
				}
				_raw[_now_path].push_back(key);
			}
		}
		else if (_token->_type == CMToken::TOKEN_BLOCK_START)
		{
			// page => blk page
			block();
		}
		else if (_token->_type == CMToken::TOKEN_EOL)
		{
			// page => EOL page
			match(CMToken::TOKEN_EOL);
		}
		else
		{
			// page => e
			break;
		}
	}
}

void CMCfg::block()
{
	// blk => <xx> page </xx>
	string blk_name, blk_name2;
	
	match(CMToken::TOKEN_BLOCK_START, &blk_name);
	
	_prev_path.push(_now_path);
	_now_path.append(blk_name + "/");
	page();
	_now_path = _prev_path.top();
	_prev_path.pop();
	
	match(CMToken::TOKEN_BLOCK_END, &blk_name2);
	if (blk_name != blk_name2)
	{
		snprintf(_buf, sizeof(_buf), "Block name \"%s\" is not match with \"%s\".",
			blk_name.c_str(), blk_name2.c_str());
		throw CMException(_buf);
	}
}

void CMCfg::match(int type, string *key)
{
	if (_token->_type == type)
	{
		if (key != NULL)
			*key = _token->_key;
		_token = _lex->next_token();
	}
	else
	{
		snprintf(_buf, sizeof(_buf), "unexpected token \"%s\".", _token->_key.c_str());
		throw CMException(_buf);
	}
}

CMLexer::CMLexer(std::string cfg_file)
{
	_peek  = ' ';
	_token = new CMToken(CMToken::TOKEN_OTHER, "");
	fin.open(cfg_file.c_str(), ios::in);
	if (!fin)
		throw CMException("file not found");
}

CMLexer::~CMLexer()
{
	delete _token;
	fin.close();
}

CMToken* CMLexer::next_token()
{
	string key = "";
	int state = STATE_BASE;
	bool comment_sign;

	_token->_type = CMToken::TOKEN_NONE;

	while (state != STATE_END)
	{
		switch (state)
		{
		case STATE_BASE:
			if (_peek == ' ' || _peek == '\t' || _peek == '\r')
				break; // 忽略空白字符
			if (_peek == '=')
			{
				_token->_type = CMToken::TOKEN_ASSIGN;
				_token->_key  = "=";
				state = STATE_END;
			}
			else if (_peek == '\n')
			{
				// end of line
				_token->_type = CMToken::TOKEN_EOL;
				_token->_key  = "end-of-line";
				state = STATE_END;
			}
			else if (_peek == EOF)
			{
				// end of file
				_token->_type = CMToken::TOKEN_EOF;
				_token->_key  = "end-of-file";
				state = STATE_END;
			}
			else if (_peek == '<')
			{
				key.clear();
				state = STATE_BLOCK;
			}
			else if (_peek == '#')
				state = STATE_COMMENT;
			else if (_peek == '/')
				state = STATE_COMMENT_HALF;
			else
			{
				key.clear();
				state = STATE_KEY;
				continue;
			}
			break;
		case STATE_BLOCK:
			if (_peek != '/')
			{
				state = STATE_BLOCK_START;
				continue;
			}
			else
				state = STATE_BLOCK_END;
			break;
		case STATE_BLOCK_START:
			if (_peek == '>')
			{
				_token->_type = CMToken::TOKEN_BLOCK_START;
				_token->_key  = key;
				state = STATE_END;
			}
			else
				key += _peek;
			break;
		case STATE_BLOCK_END:
			if (_peek == '>')
			{
				_token->_type = CMToken::TOKEN_BLOCK_END;
				_token->_key  = key;
				state = STATE_END;
			}
			else
				key += _peek;
			break;
		case STATE_COMMENT:
			if (_peek == '\n' || _peek == EOF)
			{
				if (_token->_type == CMToken::TOKEN_NONE)
					state = STATE_BASE;
				else
					state = STATE_END;
			}
			break;
		case STATE_COMMENT_HALF:
			if (_peek != '/')
			{
				key.clear();
				key += '/';
				state = STATE_KEY;
				continue;
			}
			else
				state = STATE_COMMENT;
			break;
		case STATE_KEY:
			comment_sign = false;
			if (_peek == '#')
			{
				comment_sign = true;
			}
			else if (_peek == '/')
			{
				if ((_peek = fin.get()) != '/')
					key += '/';
				else
					comment_sign = true;
			}
			
			if (comment_sign)
			{
				_token->_type = CMToken::TOKEN_KEY;
				_token->_key  = trim(key);
				state = STATE_COMMENT;
			}
			else if (_peek == '\n' || _peek == '=' || _peek == EOF)
			{
				_token->_type = CMToken::TOKEN_KEY;
				_token->_key  = trim(key);
				state = STATE_END;
				continue;
			}
			else
				key += _peek;
			break;
		}
	
		_peek = fin.get();
	}

	return _token;
}
