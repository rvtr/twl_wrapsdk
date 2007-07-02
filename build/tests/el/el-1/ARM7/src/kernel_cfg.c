#include "kernel_cfg.h"
#ifndef ATTRIBUTE_ALIGN
#define ATTRIBUTE_ALIGN(n) __attribute__ ((aligned(n)))
#endif

#define MAIN_TASK  1

#define TNUM_TSKID 32
#define TNUM_SEMID 32
#define TNUM_FLGID 32
#define TNUM_DTQID 32
#define TNUM_MBXID 32
#define TNUM_MPFID 32
#define TNUM_CYCID 32
#define TNUM_INHNO 32
#define TNUM_EXCNO 0
#define TNUM_MPLID 32
#define TNUM_ALMID 32
#define TNUM_ATTACHED_ISRNO 0
#define TNUM_ISRID 0
#define TNUM_MBFID 32
#define TNUM_MTXID 32
#define TNUM_PORID 0
#define TNUM_SVCNO 0
#if (CTR_DEF_SYSTEM_IOP == 1)
#define TSIZE_KERNEL_BUFFER (256*1024)  /* 256KByte */
#define TSIZE_MAIN_STACK    (32*1024)   /*  32KByte */
#else
#define TSIZE_KERNEL_BUFFER (1024*1024) /* 1MByte */
#define TSIZE_MAIN_STACK    (128*1024)  /* 128KByte */
#endif
#define KERNEL_MAGIC_NUMBER 0x01234567


#if TKERNEL_PRVER >= 0x1040
#define CFG_INTHDR_ENTRY(inthdr) INTHDR_ENTRY(inthdr)
#define CFG_EXCHDR_ENTRY(exchdr) EXCHDR_ENTRY(exchdr)
#define CFG_INT_ENTRY(inthdr) INT_ENTRY(inthdr)
#define CFG_EXC_ENTRY(exchdr) EXC_ENTRY(exchdr)
#else
#error "This configuration file has no compatibility with TOPPERS/JSP rel 1.3 or earlier."
#endif

#if defined(_MSC_VER) || defined(__BORLANDC__)
#define __ZERO(x,y) _declspec(naked) void y(void){}
#else
#define __ZERO(x,y) x y[1]
#endif

    /* User specified include files*/
#include "sample1.h"
#include "hw_timer.h"
#include "timer.h"
#include "logtask.h"


/********* Object initializer [task] *********/

const ID _kernel_tmax_tskid = (TMIN_TSKID + TNUM_TSKID - 1);

static VP __stack_MAIN_TASK[TSIZE_MAIN_STACK/sizeof(VP)] ATTRIBUTE_ALIGN(32);

#define TNUM_TINIA (1)
const UINT _kernel_tnum_tinia = TNUM_TINIA;
const TINIA _kernel_tinia_table[TNUM_TINIA] = {
  {MAIN_TASK, {0x00u | 0x02u, (VP_INT)(0), (FP)(main_task), INT_PRIORITY(5), sizeof(__stack_MAIN_TASK), __stack_MAIN_TASK, TA_NULL, (FP)(NULL)}}};

const ID _kernel_torder_table[TNUM_TINIA] = {1};

TCB _kernel_tcb_table[TNUM_TSKID]  ATTRIBUTE_ALIGN(32);
TINIB _kernel_tinib_table[TNUM_TSKID];

#define TNUM_TSK_RID (0)     /* ó\ñÒIDÇÃê›íË */
const UINT _kernel_tnum_trid = TNUM_TSK_RID;
__ZERO(const ID, _kernel_trid_table);


/************** Object initializer [semaphore] ****************/
//miya #define TNUM_SEMID 4

const ID _kernel_tmax_semid = (TMIN_SEMID + TNUM_SEMID - 1);

#define TNUM_SEMINIA (0)
const UINT _kernel_tnum_seminia = TNUM_SEMINIA;
__ZERO(const SEMINIA, _kernel_seminia_table);


SEMCB _kernel_semcb_table[TNUM_SEMID]  ATTRIBUTE_ALIGN(32);
SEMINIB _kernel_seminib_table[TNUM_SEMID];

#define TNUM_SEM_RID (0)
const UINT _kernel_tnum_semrid = TNUM_SEM_RID;
__ZERO(const ID, _kernel_semrid_table);


/********** Object initializer [eventflag] *********/

// miya #define TNUM_FLGID 0

const ID _kernel_tmax_flgid = (TMIN_FLGID + TNUM_FLGID - 1);

#define TNUM_FLGINIA (0)
const UINT _kernel_tnum_flginia = TNUM_FLGINIA;
__ZERO(const FLGINIA,_kernel_flginia_table);

FLGCB _kernel_flgcb_table[TNUM_FLGID]  ATTRIBUTE_ALIGN(32);
FLGINIB _kernel_flginib_table[TNUM_FLGID];


#define TNUM_FLG_RID (0)
const UINT _kernel_tnum_flgrid = TNUM_FLG_RID;
__ZERO(const ID, _kernel_flgrid_table);


/******** Object initializer [dataqueue] ********/

// miya   #define TNUM_DTQID 0

const ID _kernel_tmax_dtqid = (TMIN_DTQID + TNUM_DTQID - 1);

#define TNUM_DTQINIA (0)
const UINT _kernel_tnum_dtqinia = TNUM_DTQINIA;
__ZERO(const DTQINIA,_kernel_dtqinia_table);

DTQCB _kernel_dtqcb_table[TNUM_DTQID]  ATTRIBUTE_ALIGN(32);
DTQINIB _kernel_dtqinib_table[TNUM_DTQID];

#define TNUM_DTQ_RID (0)
const UINT _kernel_tnum_dtqrid = TNUM_DTQ_RID;
__ZERO(const ID, _kernel_dtqrid_table);


/******** Object initializer [mailbox] ********/

// miya   #define TNUM_MBXID 0

const ID _kernel_tmax_mbxid = (TMIN_MBXID + TNUM_MBXID - 1);

#define TNUM_MBXINIA (0)
const UINT _kernel_tnum_mbxinia = TNUM_MBXINIA;
__ZERO(const MBXINIA,_kernel_mbxinia_table);
MBXCB  _kernel_mbxcb_table[TNUM_MBXID]  ATTRIBUTE_ALIGN(32);
MBXINIB _kernel_mbxinib_table[TNUM_MBXID];

#define TNUM_MBX_RID (0)
const UINT _kernel_tnum_mbxrid = TNUM_MBX_RID;
__ZERO(const ID, _kernel_mbxrid_table);


/******** Object initializer [mempfix] *******/

// miya   #define TNUM_MPFID 0

const ID _kernel_tmax_mpfid = (TMIN_MPFID + TNUM_MPFID - 1);

#define TNUM_MPFINIA (0)
const UINT _kernel_tnum_mpfinia = TNUM_MPFINIA;
__ZERO(const MPFINIA,_kernel_mpfinia_table);

MPFCB _kernel_mpfcb_table[TNUM_MPFID]  ATTRIBUTE_ALIGN(32);
MPFINIB _kernel_mpfinib_table[TNUM_MPFID];

#define TNUM_MPF_RID (0)
const UINT _kernel_tnum_mpfrid = TNUM_MPF_RID;
__ZERO(const ID, _kernel_mpfrid_table);


/************* Object initializer [cyclic] ***********/

// miya   #define TNUM_CYCID 1

const ID _kernel_tmax_cycid = (TMIN_CYCID + TNUM_CYCID - 1);

#define TNUM_CYCINIA (0)
const UINT _kernel_tnum_cycinia = TNUM_CYCINIA;
__ZERO(const CYCINIA, _kernel_cycinia_table);

CYCCB _kernel_cyccb_table[TNUM_CYCID]  ATTRIBUTE_ALIGN(32);
CYCINIB _kernel_cycinib_table[TNUM_CYCID];

#define TNUM_CYC_RID (0)
const UINT _kernel_tnum_cycrid = TNUM_CYC_RID;
__ZERO(const ID, _kernel_cycrid_table);


/******* Object initializer [interrupt] *********/

// miya   #define TNUM_INHNO 1

const UINT _kernel_tnum_inhno = TNUM_INHNO;

#if (CTR_DEF_SYSTEM_IOP == 1)
CFG_INTHDR_ENTRY(timer_handler);

const INHINIB _kernel_inhinib_table[TNUM_INHNO] = {
    {OS_INTR_ID_TIMER1,0,(FP)CFG_INT_ENTRY(timer_handler)}};
#else
__ZERO(const INHINIB, _kernel_inhinib_table);
#endif

/******** Object initializer [exception] **********/


/********** Object initializer [mempvar] ***********/

// miya   #define TNUM_MPLID 0

const ID _kernel_tmax_mplid = (TMIN_MPLID + TNUM_MPLID - 1);

#define TNUM_MPLINIA (0)
const UINT _kernel_tnum_mplinia = TNUM_MPLINIA;
__ZERO(const MPLINIA,_kernel_mplinia_table);
MPLCB _kernel_mplcb_table[TNUM_MPLID]  ATTRIBUTE_ALIGN(32);
MPLINIB _kernel_mplinib_table[TNUM_MPLID];


#define TNUM_MPL_RID (0)
const UINT _kernel_tnum_mplrid = TNUM_MPL_RID;
__ZERO(const ID, _kernel_mplrid_table);


/******* Object initializer [alarm] ********/

// miya   #define TNUM_ALMID 0

const ID _kernel_tmax_almid = (TMIN_ALMID + TNUM_ALMID - 1);

#define TNUM_ALMINIA (0)
const UINT _kernel_tnum_alminia = TNUM_ALMINIA;
__ZERO(const ALMINIA,_kernel_alminia_table);
__ZERO(ALMCB, _kernel_almcb_table)  ATTRIBUTE_ALIGN(32);
__ZERO(ALMINIB, _kernel_alminib_table);

#define TNUM_ALM_RID (0)
const UINT _kernel_tnum_almrid = TNUM_ALM_RID;
__ZERO(const ID, _kernel_almrid_table);

/* Overrun handler */

void (null)(ID tskid, VP_INT exinf);
FP _kernel_overrun_handler_function = (FP)NULL;



/******** Object initializer [attached_isr] ********/

// miya   #define TNUM_ATTACHED_ISRNO 0

const UINT _kernel_tnum_attached_isrno = TNUM_ATTACHED_ISRNO;

__ZERO(const ATTACHED_ISRINIB,_kernel_attached_isrinib_table);


/******* Object initializer [interrupt_service] ********/

// miya   #define TNUM_ISRID 0

const ID _kernel_tmax_isrid = (TMIN_ISRID + TNUM_ISRID - 1);

#define TNUM_ISRINIA (0)
const UINT _kernel_tnum_isrinia = TNUM_ISRINIA;
__ZERO(const ISRINIA,_kernel_isrinia_table);
__ZERO(ISRCB, _kernel_isrcb_table)  ATTRIBUTE_ALIGN(32);
__ZERO(ISRINIB, _kernel_isrinib_table);

#define TNUM_ISR_RID (0)
const UINT _kernel_tnum_isrrid = TNUM_ISR_RID;
__ZERO(const ID, _kernel_isrrid_table);


/******** Object initializer [messagebuffer] ********/

// miya   #define TNUM_MBFID 0

const ID _kernel_tmax_mbfid = (TMIN_MBFID + TNUM_MBFID - 1);

#define TNUM_MBFINIA (0)
const UINT _kernel_tnum_mbfinia = TNUM_MBFINIA;
__ZERO(const MBFINIA,_kernel_mbfinia_table);
MBFCB _kernel_mbfcb_table[TNUM_MBFID]  ATTRIBUTE_ALIGN(32);
MBFINIB _kernel_mbfinib_table[TNUM_MBFID];

#define TNUM_MBF_RID (0)
const UINT _kernel_tnum_mbfrid = TNUM_MBF_RID;
__ZERO(const ID, _kernel_mbfrid_table);


/********* Object initializer [mutex] ************/

// miya   #define TNUM_MTXID 0

const ID _kernel_tmax_mtxid = (TMIN_MTXID + TNUM_MTXID - 1);

#define TNUM_MTXINIA (0)
const UINT _kernel_tnum_mtxinia = TNUM_MTXINIA;
__ZERO(const MTXINIA,_kernel_mtxinia_table);
MTXCB _kernel_mtxcb_table[TNUM_MTXID]  ATTRIBUTE_ALIGN(32);
MTXINIB _kernel_mtxinib_table[TNUM_MTXID];

#define TNUM_MTX_RID (0)
const UINT _kernel_tnum_mtxrid = TNUM_MTX_RID;
__ZERO(const ID, _kernel_mtxrid_table);


/******** Object initializer [rendezvous] ********/

// miya   #define TNUM_PORID 0

const ID _kernel_tmax_porid = (TMIN_PORID + TNUM_PORID - 1);

#define TNUM_PORINIA (0)
const UINT _kernel_tnum_porinia = TNUM_PORINIA;
__ZERO(const PORINIA,_kernel_porinia_table);
__ZERO(PORCB, _kernel_porcb_table)  ATTRIBUTE_ALIGN(32);
__ZERO(PORINIB, _kernel_porinib_table);

#define TNUM_POR_RID (0)
const UINT _kernel_tnum_porrid = TNUM_POR_RID;
__ZERO(const ID, _kernel_porrid_table);


/***** Object initializer [exservicecall] *****/

// miya   #define TNUM_SVCNO 0

const UINT _kernel_tnum_svcno = TNUM_SVCNO;

__ZERO(const SVCINIB,_kernel_svcinib_table);
__ZERO(SVCCB, _kernel_svccb_table)  ATTRIBUTE_ALIGN(32);

#define TNUM_SVC_RID (0)
const UINT _kernel_tnum_svcrid = TNUM_SVC_RID;
__ZERO(const ID, _kernel_svcrid_table);

/******* Kernel buffer ********/

// miya   #define TSIZE_KERNEL_BUFFER (1024*1024)

VP __kernel_bufmgr_buffer[TSIZE_KERNEL_BUFFER/sizeof(VP)]  ATTRIBUTE_ALIGN(32);
const SIZE __kernel_bufmgr_buffer_size = TSIZE_KERNEL_BUFFER;

TMEVTN  _kernel_tmevt_heap[TNUM_TSKID + TNUM_CYCID + TNUM_ALMID];


/******** Variables for kernel checker ********/
#if (CTR_DEF_SYSTEM_IOP == 1)
static const UW _checker_magic_number = KERNEL_MAGIC_NUMBER;
#endif
/************************************************************/

// miya   #define TNUM_EXCNO 0

const UINT _kernel_tnum_excno = TNUM_EXCNO;

__ZERO(const EXCINIB,_kernel_excinib_table);
    /* Initialization handler */

void
_kernel_call_inirtn(void)
{
    timer_initialize( (VP_INT)(0) );
    // serial_initialize( (VP_INT)(0) );
}

void
_kernel_call_terrtn(void)
{
    timer_terminate( (VP_INT)(0) );
}

    /* Object initialization routine */

void
_kernel_object_initialize(void)
{
    _kernel_bufmgr_initialize();

    if( _kernel_tmax_tskid ) {
      _kernel_task_initialize();
    }
    if( _kernel_tmax_semid ) {
      _kernel_semaphore_initialize();
    }
    if( _kernel_tmax_flgid ) {
      _kernel_eventflag_initialize();
    }
    if( _kernel_tmax_dtqid ) {
      _kernel_dataqueue_initialize();
    }

    if( _kernel_tmax_mtxid ) {
      _kernel_mutex_initialize();
    }
    if( _kernel_tmax_mbxid ) {
      _kernel_mailbox_initialize();
    }
    if( _kernel_tmax_mpfid ) {
      _kernel_mempfix_initialize();
    }

    if( _kernel_tmax_mplid ) {
      _kernel_mempvar_initialize();
    }

    if( _kernel_tmax_cycid ) {
      _kernel_cyclic_initialize();
    }
    if( _kernel_tmax_mbfid ) {
      _kernel_messagebuffer_initialize();
    }
    if( _kernel_tmax_almid ) {
      _kernel_alarm_initialize();
    }
    if( _kernel_tmax_porid ) {
      _kernel_rendezvous_initialize();
    }
#if 0

    if( _kernel_tmax_isrid )
    //_kernel_svchash_initialize();

    task_overrun.c:overrun_initialize();
    exception.c:exception_initialize();
    interrupt.c:attached_isr_initialize();
    interrupt.c:interrupt_service_initialize();
    servicecall.c:_kernel_svchash_initialize();
    servicecall.c:exservicecall_initialize();
#endif

#if (CTR_DEF_SYSTEM_IOP == 1)
    _kernel_interrupt_initialize();
#endif
}

