/*

_____             _       _   _ ___________ 
|  __ \          | |     | \ | |  ___|_   _|
| |__) |___ _   _| | __ _|  \| | |__   | |  
|  ___/ _` | | | | |/ _` | . ` |  __|  | |  
| |  | (_| | |_| | | (_| | |\  | |___  | |  
|_|   \__,_|\__,_|_|\__,_|_| \_|_____| |_|  

  PaulaNET - Amiga WiFi via the Floppy Port
  Using Paula's DMA for what she never intended
  Copyright (C) 2026 RobSmithDev

*/
#include "l_option.h"
#include <proto/exec.h>
#define NO_INLINE_STDARG
#include <proto/intuition.h>
#undef NO_INLINE_STDARG
#include <proto/gadtools.h>
#include <proto/graphics.h>
#include <proto/dos.h>
#include <intuition/intuition.h>
#include <intuition/gadgetclass.h>
#include <libraries/gadtools.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>

__attribute__((externally_visible)) struct ExecBase *SysBase;
__attribute__((externally_visible)) struct IntuitionBase *IntuitionBase = NULL;
__attribute__((externally_visible)) struct DosLibrary *DOSBase = NULL;
__attribute__((externally_visible)) struct UtilityBase *UtilityBase = NULL;
__attribute__((externally_visible)) struct GfxBase *GfxBase = NULL;
__attribute__((externally_visible)) struct Library *GadToolsBase = NULL;

#include "paulanetstruct.h"
#include "paulanetio.h"
#include "maindlg.h"


#if OPTION_USE_CLIB2
#include <stdlib_headers.h>
#endif


#if OPTION_USE_CLIB2
// Declare all clib2 constructors and destructors
extern int __ctor_stdlib_memory_init(void);
extern int __ctor_stdlib_program_name_init(void);
extern int __ctor_stdio_init(void);
extern int __ctor_stdio_file_init(void);
extern int __ctor_math_init(void);
extern int __ctor_socket_init(void);
extern int __ctor_arg_init(void);
extern int __ctor_rexxvars_init(void);
extern int __ctor_dirent_init(void);
extern int __ctor_locale_init(void);
extern int __ctor_clock_init(void);
extern int __ctor_unistd_init(void);
extern int __ctor_timer_init(void);
extern int __ctor_usergroup_init(void);

extern int __dtor_usergroup_exit();
extern int __dtor_timer_exit();
extern int __dtor_unistd_exit();
extern int __dtor_locale_exit();
extern int __dtor_dirent_exit();
extern int __dtor_rexxvars_exit();
extern int __dtor_socket_exit();
extern int __dtor_math_exit();
extern int __dtor_workbench_exit();
extern int __dtor_stdio_exit();
extern int __dtor_stdlib_program_name_exit();
extern int __dtor_stdlib_memory_exit();
extern int __dtor___wildcard_expand_exit();
extern int __dtor_alloca_exit();
extern int __dtor___setenv_exit();
extern int __dtor___chdir_exit();
#endif

#if OPTION_USE_CLIB2
// Calls all constructors
static VOID fcntCallCtor(void)
{	
	__ctor_stdlib_memory_init();
	__ctor_stdlib_program_name_init();
	__ctor_stdio_init();
	__ctor_stdio_file_init();
	__ctor_math_init();
	__ctor_socket_init();
	__ctor_arg_init();
	__ctor_rexxvars_init();
	__ctor_dirent_init();
	__ctor_locale_init();
	__ctor_clock_init();
	__ctor_unistd_init();
	__ctor_timer_init();
	__ctor_usergroup_init();
}

// Calls all destructors
static VOID fcntCallDtor(void)
{	
	__dtor___wildcard_expand_exit();
	__dtor_alloca_exit();
	__dtor___setenv_exit();
	__dtor___chdir_exit();
	__dtor_usergroup_exit();
	__dtor_timer_exit();
	__dtor_unistd_exit();
	__dtor_locale_exit();
	__dtor_dirent_exit();
	__dtor_rexxvars_exit();
	__dtor_socket_exit();
	__dtor_math_exit();
	__dtor_workbench_exit();
	__dtor_stdio_exit();
	__dtor_stdlib_program_name_exit();
	__dtor_stdlib_memory_exit();
}
#endif


// clean up
void cleanUP(void) {
	if (IntuitionBase != NULL) CloseLibrary((struct Library *)IntuitionBase);
	if (DOSBase) CloseLibrary((struct Library *)DOSBase);
	if (UtilityBase) CloseLibrary((struct Library *)UtilityBase);
	if (GfxBase) CloseLibrary((struct Library *)GfxBase);	
}
#include <devices/trackdisk.h>


// Entry
int main(int argc, char **argv) {
	SysBase = *((struct ExecBase**)4UL);
	struct Process *proc;
	struct WBStartup *wbmsg = NULL;	
	proc = (struct Process *)FindTask(NULL); // We check if our program has been started by the workbench or by the shell

	if (proc->pr_CLI == NULL) { // We have been launched by workbench: we wait for the startup message		
		WaitPort(&proc->pr_MsgPort);
		wbmsg = (struct WBStartup *)GetMsg(&proc->pr_MsgPort);
	}

#if OPTION_USE_CLIB2
	__WBenchMsg = wbmsg; // this variable is used in __ctor_arg_init()
	__exit_blocked = FALSE; // this will push exit() and similar functions to longjmp to target set by setjmp()
#endif

	// We open the libraries (required since we are linking with alib; please see https://github.com/jyoberle/alib for details)
	IntuitionBase = (struct IntuitionBase *)OpenLibrary((CONST_STRPTR)"intuition.library",0L);
	DOSBase = (struct DosLibrary *)OpenLibrary((CONST_STRPTR)"dos.library",0L);
	UtilityBase = (struct UtilityBase *)OpenLibrary((CONST_STRPTR)"utility.library",0L);
	GfxBase = (struct GfxBase *)OpenLibrary("graphics.library",0);
	GadToolsBase = OpenLibrary("gadtools.library", 37); 

	if(IntuitionBase && DOSBase && UtilityBase && GfxBase && GadToolsBase) {
#if OPTION_USE_CLIB2
		__UtilityBase = (struct Library *)UtilityBase; // we need to add this because of macro DECLARE_UTILITYBASE()

		if(setjmp(__exit_jmp_buf) != 0) goto out; // target for exit

		fcntCallCtor(); // call the constructors
#endif
#if OPTION_USE_CLIB2
		out:
		fcntCallDtor(); // call the destructors
#endif
	}
  
	APTR t = paulaNetFindAndOpen(SysBase, DOSBase);

	if (t) {
		runSettingsWindow(t);
		paulaNetClose(t);
	} else {
		struct EasyStruct es = { sizeof(struct EasyStruct), 0, "PaulaNET","The PaulaNET device was not detected.","OK"};
		EasyRequest(NULL, &es, NULL);		
	}

	cleanUP();

	if(wbmsg != NULL) {		
		Forbid();
		ReplyMsg((struct Message *)wbmsg);
	}
}

__attribute__((used)) __attribute__((section(".text.unlikely"))) void _start(int argc, char **argv) {
	main(argc,argv);
}