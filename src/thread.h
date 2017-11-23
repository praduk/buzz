/***
* thread.h - a very simple cross-platform threading library encapsulator
*          - with some miscellaneous useful stuff...
*
* Supported Operating Systems:
* Win 98, Win Me, Win NT, Win 2000, Win XP, Win Vista and POSIX
*
* Supported Processors:
* x86, x86-64
*
* Starting function prototypes for threads:
* THREAD_PROC_RET startRoutine(void* params)
* DTHREAD_PROC_RET startRoutine(void* params) //detached thread
*
* Datatypes:
* Thread
* Mutex    //Blocking Mutex - slow, but blocking
* SpinLock //Spinning Mutex - fast, but hogs CPU
*
* Functions/Macros:
*
* Detached Threading ---
* void BeginDThread(DTHREAD_ROUTINE startRoutine, void* parameters) //begins detached thread
* void QuitDThread() //exits from a detached thread
*
* Attached Threading ---
* Thread BeginThread(THREAD_ROUTINE startRoutine, void* parameters)
* void QuitThread(int exitcode)
* int Join(Thread t) (must be called to clean up a thread)
*                          (returns the exit code)
*
* Blocking Mutex ---
* void InitMutex(Mutex* m)
* void DeleteMutex(Mutex m)
* void LockM(Mutex m)
* bool TryLockM(Mutex m) (returns non-zero if lock succeeded) POSIX or WIN2000+ only
* void ReleaseM(Mutex m)
*
* Spinning Mutex ---
* void ResetSpinLock(SpinLock s)
* long IsLocked(SpinLock s)
* void SetLocked(s,boolean)
* void Lock(SpinLock s)
* void TryLock(SpinLock s) (returns non-zero if lock succeeded)
* void Release(SpinLock s)
*
* Sleeping ---
* void MilliSleep(unsigned int milliseconds)
*
* Computed Gotos ---
* void JMP(void* code_address); will change the instruction pointer to code_address
* void GETADDR(void** code_address_p); will store the instruction pointer to *code_address_p
*
* Copyright (C) 2007 Pradyumna Kannan. All rights reserved.
****/


#ifndef __THREAD__
#define __THREAD__
#define __THREAD_VER 0007

#if !(defined _WIN32) && !(defined _WIN64) && !(defined _POSIX)
#error You must compile to a supported platform: Win 98, Win Me, Win NT, Win 2000, Win XP, Win Vista, and POSIX (UNIX)
#endif

#ifndef INLINE
	#ifdef _MSC_VER
		#define INLINE __forceinline
	#elif defined(__GNUC__)
		#define INLINE __inline__ __attribute__((always_inline))
	#else
		#define INLINE inline
	#endif
#endif

/************************ WINDOWS THREADING ************************/

#if (defined(_WIN32) || defined(_WIN64))
#define WINDOWS_THREADING

#if defined(_MSC_VER) && !defined(_MT)
#error You must use a multi-threaded runtime library
#endif

#ifdef _MSC_VER
	#pragma message("MSC compatible compiler detected -- turning off warning 4716")
	#pragma warning( disable : 4716 )
	#define THREAD_PROC_RET unsigned int __stdcall
	#define DTHREAD_PROC_RET void __cdecl
	typedef void (__cdecl *DTHREAD_ROUTINE)(void*);
	typedef unsigned int (__stdcall *THREAD_ROUTINE)(void*);
#else
	#define DTHREAD_PROC_RET void
	#define THREAD_PROC_RET unsigned int
	typedef void (*DTHREAD_ROUTINE)(void*);
	typedef unsigned int (*THREAD_ROUTINE)(void*);
#endif

typedef void* volatile Thread;
typedef void* volatile Mutex;

#endif //Windows Threading

/************************ POSIX THREADING ************************/

#if defined(_POSIX) && !(defined(_WIN32) || defined(_WIN64))
#define POSIX_THREADING

//Headers
#ifdef __GNUC__
	#define _GNU_SOURCE
#endif
#include <pthread.h>

#define THREAD_PROC_RET void*
#define DTHREAD_PROC_RET THREAD_PROC_RET
typedef DTHREAD_PROC_RET (*DTHREAD_ROUTINE)(void*);
typedef THREAD_PROC_RET (*THREAD_ROUTINE)(void*);

typedef volatile pthread_t Thread;
typedef pthread_mutex_t* volatile Mutex;

#endif //POSIX Threads (PThreads)

/************************ SPIN LOCKS ************************/

#if defined(__GNUC__)
	typedef volatile int SpinLock[1];
	typedef volatile int* const SpinLock_P;
	static INLINE int volatile LockedExchange(SpinLock_P Target, const int Value)
	{
		int ret = Value;
		__asm__
		(
			"xchgl %[ret], %[Target]"
			: [ret] "+r" (ret)
			: [Target] "m" (*Target)
			: "memory"
		);
		return ret;
	}
#elif defined(_MSC_VER)
	typedef volatile long SpinLock[1];
	typedef volatile long* const SpinLock_P;
	#include <intrin.h>
	#pragma intrinsic (_InterlockedExchange) 
	#define LockedExchange(Target,Value) _InterlockedExchange(Target,Value)
#else
	#error Unspported Compiler
#endif

#define IsLocked(s) ((s)[0])
#define SetLocked(s,boolean) ((s)[0]=(boolean))
#define ResetSpinLock(s) SetLocked(s,0)
static INLINE void volatile Release(SpinLock_P s) {SetLocked(s,0);}
static INLINE int volatile TryLock(SpinLock_P s) {return !(IsLocked(s) || LockedExchange(s,1));} 
static INLINE void volatile Lock(SpinLock_P s) {while(IsLocked(s) || LockedExchange(s,1));}

/************************ FUNCTION PROTOTYPES ************************/

/********** Detached Threads ***********/

/**
 * BeginDthread starts a new detached thread
 *
 * @param startRoutine - this routine will be called when the new thread starts
 * @param parameters   - the parameters to send to the routine
 */
void BeginDThread(DTHREAD_ROUTINE startRoutine, void* parameters);

/**
 * When QuitDThread called from a running detached thread, the thread quits
 */
void QuitDThread();


/********** Attached Threads ***********/

/**
 * BeginThread starts a new thread
 *
 * @param startRoutine - this routine will be called when the new thread starts
 * @param parameters   - the parameters to send to the routine
 * @returns the new thread created
 */
Thread BeginThread(THREAD_ROUTINE startRoutine, void* parameters);

/**
 * When QuitThread called from a running thread, the thread quits
 */
void QuitThread(int exitcode);

/**
 * Waits for a thread to finish then cleans up memory used by the thread
 * @param t - the thread to wait for
 * @returns the thread's exit code
 */
int Join(Thread t);

/********** Mutexes ***********/

void InitMutex(Mutex* m);
void DeleteMutex(Mutex m);
void LockM(Mutex m);
#if((defined(WINDOWS_THREADING) && _WIN32_WINNT >= 0x0400) || defined(POSIX_THREADING))
	int  TryLockM(Mutex m); //Returns non-zero if lock is successful
#endif
void ReleaseM(Mutex m);

/********** Miscellaneous ***********/

void MilliSleep(unsigned int milliseconds);

#endif //__THREAD__
