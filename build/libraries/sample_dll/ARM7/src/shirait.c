#include <twl_sp.h>

static int a_func( void);
static int i_func( void);
void g_func( void);

static u16 i_data = 2;
u16 g_data = 3;

#define D_NUM	0x4


#include <nitro/code32.h>
static int a_func( void)
{
    u16 a = 5;
    u16 b = 7;

    a+= b;
    if( ((vu16)(g_data)) > 2) {
        a+= i_func();
    }

    OS_TPrintf( "arm_a_func\n");
    return a;
}
#include <nitro/codereset.h>



#include <nitro/code16.h>
static int i_func( void)
{
    u16 a = 1;
    u16 b = 2;
    u16 i;
    u16 c[D_NUM];

    i_data = 1;
    a+= b;

    OS_TPrintf( "thumb_i_func\n");
    return a;
}

void g_func( void)
{
    u16 c;

    if( ((vu16)(g_data)) > 2) {
        c = a_func();
        i_func();
    }else{
        c = i_func();
        a_func();
    }
    
    OS_Printf( "t_func %d\n", c);
}
#include <nitro/codereset.h>


