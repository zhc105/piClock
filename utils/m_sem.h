#ifndef _M_SEM_H_
#define _M_SEM_H_

#include <semaphore.h>
#include <fcntl.h>
#include <string>

class CMSem
{
private:
	sem_t *_sem;
	std::string _name;

public:
	CMSem(const char* name, int value);
	~CMSem();
	bool get();
	bool post();

};

#endif

