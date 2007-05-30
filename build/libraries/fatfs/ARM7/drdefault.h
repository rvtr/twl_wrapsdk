
#ifndef	__DRDEFAULT_H__
#define __DRDEFAULT_H__


void defaultRtfsAttach( int driveno);
BOOL defaultRtfsIo( int driveno, dword block, void* buffer, word count, BOOLEAN reading);
int defaultRtfsCtrl( int driveno, int opcode, void* pargs);


#endif /*__DRDEFAULT_H__*/
