/*
 * Copyright (c) 2013
 *	The President and Fellows of Harvard College.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE UNIVERSITY AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE UNIVERSITY OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef _PROC_H_
#define _PROC_H_

/*
 * Definition of a process.
 *
 * Note: curproc is defined by <current.h>.
 */

#include <spinlock.h>
#include <thread.h> /* required for struct threadarray */
#include "opt-A2.h"

struct addrspace;
struct vnode;
#ifdef UW
struct semaphore;
#endif // UW

#define PROC_EXITED 0
#define PROC_RUNNING 1
#define PROC_NO_PID -1

#define MAX_PID 256

// Begins at 1 because in wait.h, PID 0 is defined in a special way for process groups
// 1 is reserved for the kernel process.
#define MIN_PID 1

/*
 * Array of processes.
 */
#ifndef PROCINLINE
#define PROCINLINE INLINE
#endif

DECLARRAY(proc);
DEFARRAY(proc, PROCINLINE);

int procCount;
int pidLimit;

// procTable to hold all processes
struct array *procTable;

// lock to shield critical sections, such as when a parent calls wait as child calls exit
struct lock *proc_lock;

// Call once during system startup to allocate data structures.
void proctable_bootstrap(void);

// Add process to table, associate it with its parent
int proctable_add_process(struct proc *proc_created, struct proc *proc_parent);

// Switch a process from running to exiting
void proc_exit(struct proc *proc_exited, int exitCode);

// Remove a process from the process table
void proctable_remove_process(struct proc *proc_removed);

// Return a process from the proctable
struct proc* get_proctree(pid_t pid);

#endif /* _PROCTABLE_H_ */


/*
 * Process structure.
 */
struct proc {
	char *p_name;			/* Name of this process */
	struct spinlock p_lock;		/* Lock for this structure */
	struct threadarray p_threads;	/* Threads in this process */

	/* VM */
	struct addrspace *p_addrspace;	/* virtual address space */

	/* VFS */
	struct vnode *p_cwd;		/* current working directory */

#ifdef UW
  /* a vnode to refer to the console device */
  /* this is a quick-and-dirty way to get console writes working */
  /* you will probably need to change this when implementing file-related
     system calls, since each process will need to keep track of all files
     it has opened, not just the console. */
  struct vnode *console;                /* a vnode for the console device */
#endif

	/* add more material here as needed */

  pid_t p_pid; // Process id of current process.
  pid_t p_ppid; // Process id of parent process.
  int p_state; // State of the process, running or exited.
  int p_exitcode; // Exit code.
  struct cv *wait_cv; // parent proc waits on this cv until its child exits.

};

/* This is the process structure for the kernel and for kernel-only threads. */
extern struct proc *kproc;

/* Semaphore used to signal when there are no more processes */
#ifdef UW
extern struct semaphore *no_proc_sem;
#endif // UW

/* Call once during system startup to allocate data structures. */
void proc_bootstrap(void);

/* Create a fresh process for use by runprogram(). */
struct proc *proc_create_runprogram(const char *name);

/* Destroy a process. */
void proc_destroy(struct proc *proc);

/* Attach a thread to a process. Must not already have a process. */
int proc_addthread(struct proc *proc, struct thread *t);

/* Detach a thread from its process. */
void proc_remthread(struct thread *t);

/* Fetch the address space of the current process. */
struct addrspace *curproc_getas(void);

/* Change the address space of the current process, and return the old one. */
struct addrspace *curproc_setas(struct addrspace *);

// Returns the process' exitcode
int get_exitcode(struct proc *proc);

// Returns the process' PID
int get_curpid(struct proc *proc);

// Returns the process' PPID
int get_parent_pid(struct proc *proc);

// Returns the process' state
int getState(struct proc *proc);

// Sets the process' exitcode
void setExitcode(struct proc *proc, int exitcode);

// Sets the process' PID
void setPID(struct proc *proc, int newPID);

// Sets the process' PPID
void setPPID(struct proc *proc, int newPPID);

// Sets the process' state
void setState(struct proc *proc, int newState);
