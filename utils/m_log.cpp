#include "m_log.h"
extern "C"
{
	#include "setproctitle.h"
}

#include <ctime>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <cstdarg>
#include <signal.h>

volatile bool stop;
TDebugLog* TDebugLog::_instance = NULL;

TDebugLog::TDebugLog()
{
	_log_level = LOG_DEBUG;
	_log_file = -1;

	_instance = this;
}

TDebugLog::~TDebugLog()
{
}

void sig_handler(int sig)
{
	if (sig == SIGINT)
	{
		stop = true;
	}
}

int TDebugLog::log_open(int level, std::string log_path, 
	std::string log_name, int max_size, int num, key_t shm_key)
{
	_log_path  = log_path;
	_log_name  = log_name;
	_log_level = level;
	_log_size  = max_size;
	_log_num   = num;

	if (_log_file != -1)
		return 1;

	std::string filename = log_path + "/" + log_name + ".log";
	_log_file = open(filename.c_str(), O_CREAT | O_APPEND | O_RDWR, 0644);
	if (_log_file == -1)
		return -1;

	// 创建锁
	std::string lockname = log_name + "_log";
	_lock = new CMSem(lockname.c_str(), 1);
	// 创建共享内存
	_shm_id = shmget(shm_key, BUFFER_SIZE + sizeof(int), IPC_CREAT | 0644);
	if (_shm_id < 0)
	{
		perror("shmget");
		return -2;
	}
	_buf = (log_buffer *) shmat(_shm_id, 0, 0);
	if (_buf == (void *) -1)
	{
		perror("shmat");
		return -3;
	}

	struct sigaction act, old_int, old_usr1;
	sigset_t blk, old;
	// 注册信号
	act.sa_handler = sig_handler;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	sigaction(SIGINT, &act, &old_int);
	sigaction(SIGUSR1, &act, &old_usr1);
	// 阻塞SIGUSR1
	sigaddset(&blk, SIGUSR1);
	sigprocmask(SIG_BLOCK, &blk, &old);
	stop = false;
	// 启动日志进程
	if ((_log_pid = fork()) == 0)
	{
		log_proc();
		exit(0);
	}
	// 恢复信号处理
	sigprocmask(SIG_SETMASK, &old, NULL);
	sigaction(SIGUSR1, &old_usr1, NULL);
	sigaction(SIGINT, &old_int, NULL);

	if (_log_pid < 0)
		return -4;

	return 0;
}

void TDebugLog::log_close()
{
	int status;

	// 发送线程停止信号并等待结束
	kill(_log_pid, SIGINT);
	do
	{
		if (waitpid(_log_pid, &status, 0) == -1)
		{
			perror("waitpid");
			break;
		}
	} while (!WIFEXITED(status) && !WIFSTOPPED(status) && !WIFSIGNALED(status));

	close(_log_file);
	shmdt(_buf);
	shmctl(_shm_id, IPC_RMID, NULL);
	delete _lock;
}	

int TDebugLog::dbg_str(const int& level, const char* level_str, const char* fmt, ...)
{
	if (level > _log_level) {
		return -1;
	}
	
	time_t now;
	time(&now);
	tm *tm_time = localtime(&now);

	char buf[10240];
	int ln;
	ln = snprintf(buf, sizeof(buf), "[%04d-%02d-%02d %02d:%02d:%02d] %s ", 
				tm_time->tm_year + 1900, tm_time->tm_mon + 1, tm_time->tm_mday,
				tm_time->tm_hour, tm_time->tm_min, tm_time->tm_sec, level_str);

	va_list ap;
	va_start(ap, fmt);
	ln += vsnprintf(buf + ln, sizeof(buf) - ln, fmt, ap);
	va_end(ap);

	// 写入日志缓冲区
	_lock->get();
	if (ln < BUFFER_SIZE - _buf->used)
	{
		memcpy(_buf->data + _buf->used, buf, ln);
		_buf->used += ln;
		kill(_log_pid, SIGUSR1); // 发送信号
	}
	_lock->post();

	return 0;
}


int TDebugLog::log_str(const char* fmt, ...)
{
	time_t now;
	time(&now);
	tm *tm_time = localtime(&now);

	char buf[10240];
	int ln;
	ln = snprintf(buf, sizeof(buf), "[%04d-%02d-%02d %02d:%02d:%02d] log ", 
				tm_time->tm_year + 1900, tm_time->tm_mon + 1, tm_time->tm_mday, 
				tm_time->tm_hour, tm_time->tm_min, tm_time->tm_sec);

	va_list ap;
	va_start(ap, fmt);
	ln += vsnprintf(buf + ln, sizeof(buf) - ln, fmt, ap);
	va_end(ap);

	// 写入日志缓冲区
	_lock->get();
	if (ln < BUFFER_SIZE - _buf->used)
	{
		memcpy(_buf->data + _buf->used, buf, ln);
		_buf->used += ln;
		kill(_log_pid, SIGUSR1); // 发送信号
	}
	_lock->post();

	return 0;
}

void TDebugLog::log_proc()
{
	int used;
	char *tmp = new char[TDebugLog::BUFFER_SIZE];
	setproctitle(_log_name.c_str(), "log");

	_lock->get();
	while (!stop || _buf->used > 0)
	{
		/* 等待日志信号到来 */
		if (_buf->used <= 0)
		{
			sigset_t wait;
			sigfillset(&wait);
			sigdelset(&wait, SIGINT);
			sigdelset(&wait, SIGUSR1);

			_lock->post();
			sigsuspend(&wait); // 等待信号
			_lock->get();
		}

		used = _buf->used;
		if (used > 0)
		{
			memcpy(tmp, _buf->data, used); // 拷贝数据到临时缓冲区用于写入文件
			_buf->used = 0;	// 清空日志缓冲
		}
		_lock->post();

		/* 将信息写入日志文件 */
		if (used > 0 && _log_file >= 0)
			write(_log_file, tmp, used);
		
		// 检查日志文件大小
		log_file_check();	

		// 重新获取锁
		_lock->get();
	}
	_lock->post();

	delete [] tmp;
}

void TDebugLog::log_file_check()
{
	// 检查当前日志
	struct stat st;
	std::string nowfile = _log_path + "/" + _log_name + ".log";

	stat(nowfile.c_str(), &st);
	if (st.st_size >= _log_size)
	{
		// 超出日志文件大小，日志滚动
		int page = 1, mtime = TIME_MAX;
		
		// 找出最早修改的日志文件并覆盖
		for (int i = 1; i <= _log_num; i++)
		{
			std::string filename = _log_path + "/" + _log_name + ".log." + to_str<int>(i);
			int res = stat(filename.c_str(), &st);

			if (res < 0)
			{
				if (errno == ENOENT)
				{
					page = i;
					break;
				}
				else
					return;
			}

			if (st.st_mtime < mtime)
			{
				page = i;
				mtime = st.st_mtime;
			}
		}
		std::string filename = _log_path + "/" + _log_name + ".log." + to_str<int>(page);
		close(_log_file);
		rename(nowfile.c_str(), filename.c_str());
		// 重建当前日志
		_log_file = open(nowfile.c_str(), O_CREAT | O_APPEND | O_RDWR, 0644);
	}
}
