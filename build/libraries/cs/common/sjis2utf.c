
#include <twl.h>
#include <twl/cs/cs.h>

//#define PRINTDEBUG    OS_TPrintf
#define PRINTDEBUG( ... )    ((void)0)



#define iskanji(c) ((c)>=0x81 && (c)<=0x9F || (c)>=0xE0 && (c)<=0xFC)
#define iskanji2(c) ((c)>=0x40 && (c)<=0xFC && (c)!=0x7F)



extern void jis_to_unicode( void *to, void *p); //csjistab.c

static int jis_strlen(u8 * string);
static int jis_char_length(u8 *p);
static u8 *jis_increment(u8 *p);




/*---------------------------------------------------------------------------*
  Name:         CS_Sjis2Unicode

  Description:  

  Arguments:    

  Returns:      < 0 : success( return string length)
                > 0 : error code
 *---------------------------------------------------------------------------*/
int CS_Sjis2Unicode( void* uni_str, void* jis_str)
{
    int i;
    int len;

    len = jis_strlen( jis_str);
    if( len > 255) {
        return -1;
    }
    for( i=0; i<len; i++) {
        jis_to_unicode( uni_str, jis_str);
        jis_str = jis_increment( jis_str);
        (u8*)uni_str+=2;
    }
    return len;
}


/* return number of jis chars in a string */
static int jis_strlen(u8 * string)   /*__fn__*/
{
    int len=0;
    while (*string)
    {
        string = jis_increment(string);
        len++;
    }
    PRINTDEBUG( "len:%d\n", len);
    return len;
}
/* Return the length of a JIS character, 1 or 2 */
static int jis_char_length(u8 *p)
{
    if ((*p >= 0x81 && *p <= 0x9f) || (*p >= 0xe0 && *p <= 0xfc))
        return(2);
    else
        return(1);
}

/* Advance a pointer to the next JIS character in a string */
static u8 *jis_increment(u8 *p)
{
    return(p + jis_char_length(p));
}



