/***
 * thread.c - cross-platform threading library encapsulator
 * See header file for more information.
 *
 * Copyright (C) 2007 Pradyumna Kannan. All rights reserved.
 */

#include "thread.h"

/****************************** WINDOWS THREADING *****************************/
#ifdef WINDOWS_THREADING

//Headers

#define VC_EXTRALEAN
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <process.h>
#include <stdlib.h>
#include "log.h"

//Detachted Threads
void BeginDThread(DTHREAD_ROUTINE startRoutine, void* parameters)
{
	_beginthread(startRoutine, 0, parameters);
}

void QuitDThread()
{
	_endthread();
}

//Normal Threads
Thread BeginThread(THREAD_ROUTINE startRoutine, void* parameters)
{
	DWORD dummyID;
	return CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)startRoutine,(LPVOID)parameters,0,&dummyID);
}

void QuitThread(int exitcode)
{
	ExitThread((DWORD)exitcode);
}

int Join(Thread t)
{
	DWORD exitcode;
	WaitForSingleObject((HANDLE)t,INFINITE);
	GetExitCodeThread((HANDLE)t, &exitcode);
	CloseHandle((HANDLE)t);
	return (int)exitcode;
}

//Mutexes
void InitMutex(Mutex* m)
{
	*m=malloc(sizeof(CRITICAL_SECTION));
	InitializeCriticalSection((LPCRITICAL_SECTION)(*m));
}

void DeleteMutex(Mutex m)
{
	DeleteCriticalSection((LPCRITICAL_SECTION)m);
	free(m);
}

void LockM(Mutex m)
{
	EnterCriticalSection((LPCRITICAL_SECTION)m);
}

#if((defined(WINDOWS_THREADING) && _WIN32_WINNT >= 0x0400) || POSIX_THREADING)
int TryLockM(Mutex m)
{
	return TryEnterCriticalSection((LPCRITICAL_SECTION)m);
}
#endif

void ReleaseM(Mutex m)
{
	LeaveCriticalSection((LPCRITICAL_SECTION)m);
}

//Miscellaneous
void MilliSleep(unsigned int milliseconds)
{
	Sleep((DWORD)milliseconds);
}

#endif //WINDOWS_THREADING

/******************************* POSIX THREADING ******************************/
#ifdef POSIX_THREADING
#include <stdlib.h>
#include <unistd.h>

//Detached Threads
void BeginDThread(DTHREAD_ROUTINE startRoutine, void* parameters)
{
	Thread dummy;
	pthread_create(&dummy,NULL,startRoutine,parameters);
	pthread_detach(dummy);
}

void QuitDThread()
{
	QuitThread(0);
}

//Normal Threads
Thread BeginThread(THREAD_ROUTINE startRoutine, void* parameters)
{
	Thread ret;
	pthread_create(&ret,NULL,startRoutine,parameters);
	return ret;
}

void QuitThread(int exitcode)
{
	pthread_exit((void*)(exitcode));
}

int Join(Thread t)
{
	int exitcode;
	pthread_join(t,(void*)(&exitcode));
	return exitcode;
}

//Mutexes
void InitMutex(Mutex* m)
{
	pthread_mutexattr_t recursive;
	pthread_mutexattr_init(&recursive);
	pthread_mutexattr_settype(&recursive, PTHREAD_MUTEX_RECURSIVE);
	*m=malloc(sizeof(pthread_mutex_t));
	pthread_mutex_init(*m,&recursive);
}

void DeleteMutex(Mutex m)
{
	pthread_mutex_destroy(m);
	free(m);
}

void LockM(Mutex m)
{
	pthread_mutex_lock(m);
}

int TryLockM(Mutex m)
{
	return pthread_mutex_trylock(m);
}

void ReleaseM(Mutex m)
{
	pthread_mutex_unlock(m);
}

//Miscellaneous
void MilliSleep(unsigned int milliseconds)
{
	usleep(milliseconds*1000);
}

#endif //POSIX_THREADING
