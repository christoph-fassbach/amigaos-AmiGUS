#ifndef LIB_INIT_H
#define LIB_INIT_H

#ifdef BASE_GLOBAL

extern struct ExecBase          * SysBase;
extern struct DosLibrary        * DOSBase;
extern struct IntuitionBase     * IntuitionBase;
extern struct Library           * UtilityBase;
extern struct Library           * ExpansionBase;
extern struct Device            * TimerBase;
extern struct AmiGUSmhi         * AmiGUSmhiBase;

#endif

VOID CustomLibClose( struct AmiGUSmhi * base );  
LONG CustomLibInit( struct AmiGUSmhi * base, struct ExecBase * sysBase );

#endif
