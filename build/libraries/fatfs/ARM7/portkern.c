/*
* portrtfs.c
*
* ERTFS Porting layer. The user must port some or all of the routines 
* in this file to his RTOS and hardware environment according to the 
* rules set forth in the ERTFS porting specification.
*  
*   Copyright EBS Inc , 1993-2003
*   All rights reserved.
*   This code may not be redistributed in source or linkable object form
*   without the consent of its author.
*
*   Generic RTOS porting layer template.
*
* This file contains the RTOS/KERNEL porting layer as described in
* the porting guide. If you are porting to a real time operating
* system environment you must modify most of the routines in this
* file for your RTOS. If you are not porting to an RTOS but are rather
* bringing ERTFS up in polling mode then read the following section.
*
*
*   If you are using POLLED mode then this file is actually quite portable.
*   Several reference ports are  provided for x86 environments. To port 
*   it to a non supported environment you must provide the following
*   functions and macros.
* 	unsigned long rtfs_port_get_ticks(); 
*   #define  MILLISECONDS_PER_TICK 
*
*
* And depending on what drivers are required one or more of the following
* 
* void hook_82365_pcmcia_interrupt(int irq)
* void hook_ide_interrupt(int irq, int controller_number)
* void hook_ide_interrupt(int irq, int controller_number)
* void hook_floppy_interrupt(int irq)
* 
*/

#include <twl.h>		/* ctr modified */
//#include <ctr/rtc/rtc.h> /* ctr modified */
//#include <kernel.h>
#include <rtfs.h>
#include <portconf.h>	/* For included devices */

/*--- ctr modified ---*/
#include <rtfsconf.h>
#if (RTFS_DEBUG_PRINT_ON == 1)
	#if (CTR_DEF_ENVIRONMENT_DSEMU == 1)
		#define PRINTDEBUG	osTPrintf
	#else
		#include <ctr/vlink.h>
		#define PRINTDEBUG	vlink_dos_printf
	#endif
#else
	#define PRINTDEBUG	i_sdmc_no_print
	static void i_sdmc_no_print( const char *fmt, ... );
	static void i_sdmc_no_print( const char *fmt, ... ){ return; }
#endif
/*--------------------*/

/* This routine takes no arguments and returns an unsigned long. The routine
   must return a tick count from the system clock. The macro named
   MILLISECONDS_PER_TICK must be defined in such a way so that it returns
   the rate at which the tick increases in milliseconds per tick */
/* This routine is declared as static to emphasize that its use is local 
   to the portkern.c file only */

#if (TARGET_OS_CTR == 1)
#define  MILLISECONDS_PER_TICK 1 /* Change this to your environment */
#else
#define  MILLISECONDS_PER_TICK 33520 /* Change this to your environment */
#endif

static dword rtfs_port_get_ticks(void)
{
#if (TARGET_OS_CTR == 1)
    SYSTIM systim;	/* ctr modified */
    systim = get_tim( &systim);
    PRINTDEBUG( "fatfs : rtfs_port_get_ticks(%d).\n", systim);
    return( (dword)(systim)); /* Return the real periodic clock tick value here */
#else
    OSTick systim;
    systim = OS_GetTick();
    return( (dword)(systim));
#endif
}

/* 
This routine takes no arguments and returns an unsigned long. The routine
must allocate and initialize a Mutex, setting it to the "not owned" state. It
must return an unsigned long value that will be used as a handle. ERTFS will
not interpret the value of the return value. The handle will only used as 
an argument to the rtfs_port_claim_mutex() and rtfs_port_release_mutex()
calls. The handle may be used as an index into a table or it may be cast
internally to an RTOS specific pointer. If the mutex allocation function 
fails this routine must return 0 and the ERTFS calling function will return
failure.
*/
/*メモ：apicnfig.cの定義「NDRIVES」の数 + 1個ぶんの回数呼ばれる*/
dword rtfs_port_alloc_mutex(void)
{	/* ctr modified */
	/* POLLED mode does not require mutexes */
	/* Otherwise implement it */
#if (TARGET_OS_CTR == 1)
//    ID		mytskid;
    T_CMTX	cmtx;
    ER_ID	mtxid;

//    get_tid( &mytskid);

    // setup mutex generate info.
	cmtx.mtxatr	= TA_INHERIT;  // set attribution : inherit priority
    cmtx.ceilpri= TMAX_MPRI;   // set max priority : no effect when mtxatr is without TA_CEILING

    mtxid = acre_mtx( &cmtx);	//mutexの生成
    if( mtxid < 0) {
        PRINTDEBUG("fatfs : ERROR! Cannot create new mutex.\n");
        return 0;
    }
//    PRINTDEBUG( "fatfs : task[%d] create new mutex(%d).\n", mytskid, mtxid);
	return( (dword)(mtxid));
#else
    OSMutex* my_mtx;
    my_mtx = (OSMutex*)OS_Alloc( sizeof( OSMutex));
    if( my_mtx == NULL) {
        return 0;
    }
    OS_InitMutex( my_mtx);
    return( (dword)my_mtx);
#endif
}

/* This routine takes as an argument a mutex handle that was returned by
   rtfs_port_alloc_mutex(). If the mutex is already claimed it must wait for
   it to be released and then claim the mutex and return.
*/

void rtfs_port_claim_mutex(dword handle)
{	/* ctr modified */
	/* POLLED mode does not require mutexes */
	/* Otherwise implement it */
#if ( TARGET_OS_CTR == 1)
//    ID mytskid;
    ER ercd;

//    ercd = get_tid( &mytskid);
    ercd = loc_mtx( handle);	//mutexのロック
    if( ercd == E_OK) {
//		PRINTDEBUG( "fatfs : task[%d] lock mutex(%d).\n", mytskid, handle);
    }
#else
    OS_LockMutex( (OSMutex*)handle);
#endif
}

/* This routine takes as an argument a mutex handle that was returned by
rtfs_port_alloc_mutex() that was previously claimed by a call to 
rtfs_port_claim_mutex(). It must release the handle and cause a caller 
blocked in rtfs_port_claim_mutex() for that same handle to unblock.
*/

void rtfs_port_release_mutex(dword handle)
{	/* ctr modified */
	/* POLLED mode does not require mutexes */
	/* Otherwise implement it */
#if ( TARGET_OS_CTR == 1)
//    ID mytskid;
    ER ercd;

//    ercd = get_tid( &mytskid);
    ercd = unl_mtx( handle);	//mutexのリリース
    if( ercd == E_OK) {
//		PRINTDEBUG( "fatfs : task[%d] release mutex(%d).\n", mytskid, handle);
    }
#else
    OS_UnlockMutex( (OSMutex*)handle);
    OS_Free( (OSMutex*)handle);
#endif
}

/* Note: Currently ERTFS only requires these functions if the floppy disk device
driver is being used or if the IDE device driver is being used in interrupt
mode rather than polled mode. Additional user supplied device drivers may
opt to use these calls or they may implement their own native signalling 
method at the implementor's discretion. If you are not using the floppy 
disk driver or the IDE driver in interrupt mode please skip this section.

The signalling code is abstracted into four functions that must be modified 
by the user to support the target RTOS. The required functions are:
rtfs_port_alloc_signal(), rtfs_port_clear_signal(), rtfs_port_test_signal()
 and rtfs_port_set_signal(). The requirements of each of these functions is 
defined below.
*/


/* This routine takes no arguments and returns an unsigned long. The routine
must allocate and initialize a signalling device (typically a counting 
semaphore) and set it to the "not signalled" state. It must return an 
unsigned long value that will be used as a handle. ERTFS will
not interpret the value of the return value. The handle will only used as 
an argument to the rtfs_port_clear_signal(), rtfs_port_test_signal()
and rtfs_port_set_signal() calls.
Only required for the supplied floppy disk and ide device driver if the 
ide driver is running in interrupt mode. Otherwise leave this function as 
it is, it will not be used. */

dword rtfs_port_alloc_signal(void)
{
	/* Polled mode has only one global signal */
	/* Otherwise implement it */
    PRINTDEBUG( "not implemented function ( rtfs_port_alloc_signal) is called\n");
	return(1);
}

/* This routine takes as an argument a handle that was returned by
rtfs_port_alloc_signal(). It must place the signal in an unsignalled state
such that a subsequant call to rtfs_port_test_signal() will not return 
success until rtfs_port_set_signal() has been called. This clear function
is neccessary since it is possible although unlikely that an interrupt 
service routine could call rtfs_port_set_signal() after the intended call
to rtfs_port_test_signal() timed out. A typical implementation of this 
function for a counting semaphore is to set the count value to zero or 
to poll it until it returns failure. */
int polled_signal = 0;	/* We only need one signal for polled mode since it 
						   is single threaded */
void rtfs_port_clear_signal(dword handle)
{
	/* Polled mode has only one global signal */
	/* Otherwise implement clear of the signal specified by handle */
    PRINTDEBUG( "not implemented function ( rtfs_port_clear_signal) is called\n");
	RTFS_ARGSUSED_PVOID((void *) handle);
	polled_signal = 0;
}

/* This routine takes as an argument a handle that was returned by
rtfs_port_alloc_signal() and a timeout value in milliseconds. It must 
block until timeout milliseconds have elapsed or  rtfs_port_set_signal()
has been called. If the test succeeds must return 0, if it times out it
must return a non-zero value.
Only required for the supplied floppy disk and ide device driver if the 
ide driver is running in interrupt mode. Otherwise leave this function as 
it is, it will not be used. */


int rtfs_port_test_signal(dword handle, int timeout)
{
    PRINTDEBUG( "not implemented function ( rtfs_port_test_signal) is called\n");
#if (!POLLED_MODE)
	/* Implement timed test of the signal specified by handle here 
	otherwise use the polled mode code provided below */
	return (-1);
#else
	/* This will work in polled mode */
dword last_time, end_time, curr_time;
	/* Polled mode signal wait with timeout. This is portable but the
	   user must implement the routine rtfs_port_get_ticks(void)
       and the macro MILLISECONDS_PER_TICK. */
	
	   /* polled mode only ever uses one signal at a time so we 
	      ignore the handle argument */
	RTFS_ARGSUSED_PVOID((void *) handle);
    if (timeout)
    {
		/* Covert milliseconds to ticks. If 0 set it to 2 ticks */
		timeout = timeout/MILLISECONDS_PER_TICK;
		if (!timeout)
			timeout = 2;
        last_time = end_time = 0;       // keep compiler happy
        curr_time = rtfs_port_get_ticks();    
        end_time = curr_time + (dword) timeout;
        // Check for a wrap
        if (end_time < curr_time)
		{
            end_time = (dword) 0xffffffffL;
		}
        last_time = curr_time;
        do
        {
            // See if we timed out 
            curr_time = rtfs_port_get_ticks();    
            if (curr_time > end_time)
                break;
            // if wrap or clearing of clock then start count over
            if (curr_time < last_time)      
                end_time = curr_time + (dword) timeout;
            last_time = curr_time;
        } while (polled_signal == 0);
    }
    if (polled_signal)
    {
        polled_signal -= 1;
        return(0);
    }
    else
    {
       return(-1);
    }
#endif
}
/*
This routine takes as an argument a handle that was returned by
rtfs_port_alloc_signal(). It must set the signal such that a subsequant 
call to rtfs_port_test_signal() or a call currently blocked in 
rtfs_port_test_signal() will return success. 
This routine is always called from the device driver interrupt service 
routine while the processor is executing in the interrupt context. 
Only required for the supplied floppy disk and ide device driver if the 
ide driver is running in interrupt mode. Otherwise leave this function as 
it is, it will not be used. */


void rtfs_port_set_signal(dword handle) 
{
	/* Implement set of the signal specified by handle here */
	/* This simple implemetation is for polled mode */
    PRINTDEBUG( "not implemented function ( rtfs_port_set_signal) is called\n");
	RTFS_ARGSUSED_PVOID((void *) handle);
	polled_signal += 1;
}

/* 
This routine takes as an argument a sleeptime value in milliseconds. It
must not return to the caller until at least sleeptime milliseconds have
elapsed. In a mutitasking environment this call should yield the task cpu.
*/

void rtfs_port_sleep(int sleeptime) 
{
	/* Implement simple task sleep call here */

	/* This simple implemetation is for polled mode */
	/* Call the signalling system to timeout. This will in effect sleep */
#if (TARGET_OS_CTR == 1)
    dly_tsk( (RELTIM)sleeptime);
#else
    OS_Sleep( (u32)sleeptime);
#endif
    PRINTDEBUG( "fatfs : port_sleep (time : %d). \n", sleeptime);
	rtfs_port_clear_signal(0);
	rtfs_port_test_signal(0, sleeptime);
}

/* This routine takes no arguments and returns an unsigned long. The routine
must return an unsigned long value that will later be passed to 
rtfs_port_elapsed_check() to test if a given number of milliseconds or
more have elapsed. A typical implementation of this routine would read the
system tick counter and return it as an unsigned long. ERTFS makes no 
assumptions about the value that is returned.
*/

dword rtfs_port_elapsed_zero(void)
{
    PRINTDEBUG( "fatfs : rtfs_port_elapsed_zero\n");
	return(rtfs_port_get_ticks());
}

/* This routine takes as arguments an unsigned long value that was returned by
a previous call to rtfs_port_elapsed_zero() and a timeout value in 
milliseconds. If "timeout" milliseconds have not elapsed it should return 
0. If "timeout" milliseconds have elapsed it should return 1. A typical 
implementation of this routine would read the system tick counter, subtract 
the zero value, scale the difference to milliseconds and compare that to 
timeout. If the scaled difference is greater or equal to timeout it should 
return 1, if less than timeout it should return 0.
*/

int rtfs_port_elapsed_check(dword zero_val, int timeout)
{
	dword curr_time;
    
    PRINTDEBUG( "not implemented function ( rtfs_port_elapsed_check) is called\n");

    timeout = timeout/MILLISECONDS_PER_TICK;
	if (!timeout)
		timeout = 2;
    
	curr_time = rtfs_port_get_ticks();
	if ( (curr_time - zero_val) > (dword) timeout)
		return(1);
	else
		return(0);
}

/*
This function must return an unsigned long number that is unique to the 
currently executing task such that each time this function is called from
the same task it returns this same unique number. A typical implementation
of this function would get address of the current task control block, cast
it to a long, and return it.
*/

dword rtfs_port_get_taskid(void) 
{	/* ctr modified */
	/* For an RTOS environment return a dword that is unique to this
	   task. For example the address of its task control block */
	/* Otherwise always return 1 For POLLOS */
#if (TARGET_OS_CTR == 1)
    ID	tskid;
    get_tid( &tskid);
	return( (dword)(tskid));
#else
    OSThread* my_tsk;
    my_tsk = OS_GetCurrentThread();
    return( (dword)my_tsk);
#endif
}

/*
This routine requires porting if you wish to use the interactive test shell.
It must provide a line of input from the terminal driver. The line must be
 NULL terminated and it must not contain ending newline or carriage return
characters.
*/

void rtfs_port_tm_gets(byte *buffer)
{
/* Use Get's or some other console input function. If you have no console
   input function then leave it blank */
/*	gets(buffer); */
}

/*
This routine requires porting if you wish to display messages to your console
This routine must print a line of text from the supplied buffer to
the console. The output routine must not issue a carriage or linefeed 
command unless the text is terminated with the appropriate control 
character (\n or \r).
*/

void rtfs_port_puts(byte *buffer)
{
    PRINTDEBUG( "%s\n", buffer);	//ctr modified
/* Use cputs or some other console output function. If you have no console
   output function then leave it blank */
/*	cputs(buffer); */
}

/*
This function must exit the RTOS session and return to the user prompt. It 
is only necessary when an RTOS is running inside a shell environment like 
Windows. This function must be implemented if you are using the ERTFS command library 
in an environment that will ever exit, for example if ERTFS is running with
an RTOS in a Dos/Windows or Unix environment you should put a call to your 
RTOS's exit code. 
Note: Most embedded systems never exit. If your application never exits then
 please skip this section.
*/

void rtfs_port_exit(void)
{
/* Exit your application here. If you never exit then leave it blank. */
/* 	exit(0); */
    PRINTDEBUG( "not implemented function ( rtfs_port_exit) is called\n");
}
/*
    When the system needs to date stamp a file it will call this routine
    to get the current time and date. YOU must modify the shipped routine
    to support your hardware's time and date routines. 
*/

DATESTR *pc_getsysdate(DATESTR * pd)
{
    word  year;     /* relative to 1980 */ 
    word  month;    /* 1 - 12 */ 
    word  day;      /* 1 - 31 */ 
    word  hour;
    word  minute;
    word  sec;      /* Note: seconds are 2 second/per. ie 3 == 6 seconds */
#if (TARGET_OS_CTR == 1)
    RtcDate rtfs_base_date; //ctr modified
    RtcTime rtfs_base_time; //ctr modified
#endif
    PRINTDEBUG( "not implemented function ( pc_getsysdate) is called\n");

#if (0)
 /* This code will work if yoiu have ANSI time functions. otherwise get 
    rid of it and look below where the time is wired to 3/28/8. 
    You may modify that code to work in your environment. */
    struct tm *timeptr;
    time_t timer;

    time(&timer);
    timeptr = localtime(&timer);
    
    hour    =   (word) timeptr->tm_hour;
    minute  =   (word) timeptr->tm_min;
    sec =   (word) (timeptr->tm_sec/2); 
    /* Date comes back relative to 1900 (eg 93). The pc wants it relative to
        1980. so subtract 80 */
    year  = (word) (timeptr->tm_year-80);
    month = (word) (timeptr->tm_mon+1);
    day   = (word) timeptr->tm_mday;
#else
#if (TARGET_OS_CTR == 1)
    /* This code is useless but very generic */
    /* Hardwire for now */
    /* 7:37:28 PM */
//    hour = 19;
//    minute = 37;
//    sec = 14;
    /* 3-28-88 */
//    year = 8;       /* relative to 1980 */ 
//    month = 3;      /* 1 - 12 */ 
//    day = 28;       /* 1 - 31 */

    /*----- ctr modified -----*/
    rtcGetDateTimeSync( &rtfs_base_date, &rtfs_base_time);

    hour   = rtfs_base_time.hour;
    minute = rtfs_base_time.minute;
    sec    = rtfs_base_time.second / 2;
    year   = rtfs_base_date.year - 1980;
    month  = rtfs_base_date.month;
    day    = rtfs_base_date.day;
    /*------------------------*/
#else
    
#endif
#endif
    pd->time = (word) ( (hour << 11) | (minute << 5) | sec);
    pd->date = (word) ( (year << 9) | (month << 5) | day);

    return (pd);
}
/*--- ctr modified ---*/
/*SDK_WEAK_SYMBOL BOOL rtcGetDateTimeSync(RtcDate *pDate, RtcTime *pTime)
{
    pTime->hour = 0;
    pTime->minute = 0;
    pTime->second = 0;
    pDate->year = 2007;
    pDate->month = 1;
    pDate->day = 1;
    return( FALSE);
}*/
/*--------------------*/

/* This routine must establish an interrupt handler that will call the 
plain 'C' routine void mgmt_isr(void) when the the chip's management 
interrupt event occurs. The value of the argument 'irq' is the interrupt 
number that was put into the 82365 management interrupt selection register 
and is between 0 and 15. This is controlled by the constant 
"MGMT_INTERRUPT" which is defined in pcmctrl.c
*/
#if (INCLUDE_82365_PCMCTRL)

/* The routine mgmt_isr(0) must execute when the PCMCIA controller interrupts 
   this a sample interrupt service routne that will do this, modify it 
   to match your system's interrupt behavior */
void mgmt_isr(int);
void pcmcia_isr()
{
    PRINTDEBUG( "not implemented function ( pcmcia_isr) is called\n");
    mgmt_isr(0);
}

/* This routine must "hook" the interrupt so the above code is jumped to
   when the PCMCIA controller interrupts */
void hook_82365_pcmcia_interrupt(int irq)
{
    PRINTDEBUG( "not implemented function ( hook_82365_pcmcia_interrupt) is called\n");
}

#endif /* (INCLUDE_82365_PCMCTRL) */

#if (INCLUDE_IDE)
/* The routine ide_isr(controller_number) must execute when the IDE
   controller associated with that contoller number interrupts.
   these are sample interrupt service routines that will do this, modify 
   them to match your system's interrupt behavior */

void ide_isr(int);
/* This is a sample interrupt service routine for controller 0 */
void ide_isr_0()
{
    PRINTDEBUG( "not implemented function ( ide_isr_0) is called\n");
    ide_isr(0);
}

/* This is a sample interrupt service routine for controller 1 */
void ide_isr_1()
{
    PRINTDEBUG( "not implemented function ( ide_isr_1) is called\n");
    ide_isr(1);
}


/* hook_ide_interrupt() is called with the interrupt number in the argument 
irq taken from the user's setting of pdr->interrupt_number in pc_ertfs_init(). 
Controller number is taken from the pdr->controller_number field as set in
pc_ertfs_init() by the user. Hook_ide_interrupt() must establish an interrupt
 handler such that the plain 'C' function "void ide_isr(int controller_number)"
is called when the IDE interrupt occurs. The argument to ide_isr() must be 
the controller number that was passed to hook_ide_interrupt(), this value
is typically zero for single controller system.
    
hook_ide_interrupt()は、pc_ertfs_init()内で定義されたpdr->interrupt_number
番号を引数(irq)としてコールされる。
controller_numberはpc_ertfs_init()内でpdr->controller_numberとして定義され
たもの。
*/

void hook_ide_interrupt(int irq, int controller_number)
{
    PRINTDEBUG( "not implemented function ( hook_ide_interrupt) is called\n");
}

#endif /* (INCLUDE_IDE) */

/*  This routine is called by the floppy disk device driver. It must 
establish an interrupt handler such that the plain 'C' function void 
floppy_isr(void) is called when the floppy disk interrupt occurs.
The value in "irq" is always 6, this is the PC's standard mapping of 
the floppy interrupt. If this is not correct for your system just ignore 
the irq argument.
*/
#if (INCLUDE_FLOPPY)
/* This is a sample floppy disk interrupt handler. Modify it to match
   your target system */
void floppy_isr(int);

void floppy_interrupt()
{
    PRINTDEBUG( "not implemented function ( floppy_interrupt) is called\n");
    floppy_isr(0);
}

void hook_floppy_interrupt(int irq)
{
    PRINTDEBUG( "not implemented function ( hook_floppy_interrupt) is called\n");
}

#endif /* (INCLUDE_FLOPPY) */



