#include <twl_sp.h>
#include  <stdarg.h>

void global_func( void );
static int internal_data = 2;
int global_data = 3;
extern int no_data;

extern void g_func( void);

#define NUM_OF_DATA 0x1

static int internal_func( void )
{
  int a = 1;
  int b = 2;
  int i;
  int c[NUM_OF_DATA];
  internal_data = 1;
  a += b;

  for( i = 0 ; i < NUM_OF_DATA ; i++) {
    c[i] = i;
  }

  for( i = 0 ; i < NUM_OF_DATA ; i++) {
    a += c[i];
  }

  OS_TPrintf("aho\n");
  g_func();
    
  return a;
}

void global_func( void )
{
  int c;
  c = internal_func();
  global_data = 1;
  no_data = 4;
  OS_Printf("aho %d\n",c);
}
