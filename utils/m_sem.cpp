#include "m_sem.h"

CMSem::CMSem(const char *name, int value)
{
	_name = name;
	sem_unlink(name);
    _sem = sem_open(name, O_CREAT | O_EXCL, 0664, value);
}

CMSem::~CMSem()
{
	sem_close(_sem);
	sem_unlink(_name.c_str());
}

bool CMSem::get()
{
    if ( sem_wait(_sem) != 0 )
        return false;
    return true;
}

bool CMSem::post()
{
    if ( sem_post(_sem) != 0 )
        return false;
    return true;
}

