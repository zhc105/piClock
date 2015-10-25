#ifndef _M_LOG_H_
#define _M_LOG_H_

#include "m_sem.h"
#include "m_string.h"
#include <sys/ipc.h>
#include <sys/shm.h>

enum LOG_LEVEL
{
    LOG_EMERG = 0, //system is unusable
	LOG_ALERT = 1, // action must be taken immediately
	LOG_CRIT  = 2, // critical conditions
	LOG_ERR = 3, // error conditions
	LOG_WARNING = 4, // warning conditions
	LOG_NOTICE = 5, //normal but significant condition
	LOG_INFO = 6, //informational
	LOG_DEBUG = 7, //debug-level messages
	LOG_TRACE = 8, //trace
};

#pragma pack(1)
typedef struct _log_buffer
{
	int used;
	char data[1];
} log_buffer;
#pragma pack()

class TDebugLog
{
public:
	static const int BUFFER_SIZE = 10485760;
	static const int TIME_MAX = 0x7FFFFFFF;

	TDebugLog();
	~TDebugLog();
	static TDebugLog* instance() {
		return _instance;
	}
	int log_open(int level, std::string log_path, std::string log_name, 
		int max_size, int num, key_t shm_key);
	void log_close();
	
	int log_str(const char* fmt, ...);
	int dbg_str(const int& level, const char* level_str, const char* fmt, ...);
	
	void log_proc();
	void  log_file_check();

public:
	std::string _log_path;
	std::string _log_name;
	int _log_size;
	int _log_num;

	int _log_file;
	log_buffer *_buf;

private:
	int _log_level;
	int _log_pid;
	int _shm_id;
	CMSem *_lock;

	static TDebugLog *_instance;

};

#define LOG_INIT TDebugLog::init
#define LOG_FINI TDebugLog::fini
#define LOG_OPEN TDebugLog::instance()->log_open

#define TRC(fmt, ...) TDebugLog::instance()->dbg_str(LOG_TRACE, "trace", fmt, ##__VA_ARGS__)
#define DBG(fmt, ...) TDebugLog::instance()->dbg_str(LOG_DEBUG, "debug", fmt, ##__VA_ARGS__)
#define INF(fmt, ...) TDebugLog::instance()->dbg_str(LOG_INFO, "info", fmt, ##__VA_ARGS__)
#define NTC(fmt, ...) TDebugLog::instance()->dbg_str(LOG_NOTICE, "notice", fmt, ##__VA_ARGS__)
#define WRN(fmt, ...) TDebugLog::instance()->dbg_str(LOG_WARNING, "warning", fmt, ##__VA_ARGS__)
#define ERR(fmt, ...) TDebugLog::instance()->dbg_str(LOG_ERR, "error", fmt, ##__VA_ARGS__)
#define CRT(fmt, ...) TDebugLog::instance()->dbg_str(LOG_CRIT, "critical", fmt, ##__VA_ARGS__)
#define ALT(fmt, ...) TDebugLog::instance()->dbg_str(LOG_ALERT, "alert", fmt, ##__VA_ARGS__)
#define EMG(fmt, ...) TDebugLog::instance()->dbg_str(LOG_EMERG, "emerg", fmt, ##__VA_ARGS__)
#define LOG(fmt, ...) TDebugLog::instance()->log_str(fmt, ##__VA_ARGS__)

#endif

