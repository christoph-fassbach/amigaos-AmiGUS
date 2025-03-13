#ifndef MHIAMIGUS_LIB_PROTOS_H
#define MHIAMIGUS_LIB_PROTOS_H

#include "SDI_compiler.h"

ASM( APTR ) SAVEDS MHIAllocDecoder( REG( a0, struct Task * task ),
                                REG( d0, ULONG signal ));

ASM( VOID ) SAVEDS MHIFreeDecoder( REG( a3, APTR handle ));

ASM( BOOL ) SAVEDS MHIQueueBuffer( REG( a3, APTR handle ),
                                   REG( a0, APTR buffer ),
                                   REG( d0, ULONG size ));

ASM( APTR ) SAVEDS MHIGetEmpty( REG( a3, APTR handle ));

ASM( UBYTE ) SAVEDS MHIGetStatus( REG( a3, APTR handle ));

ASM( VOID ) SAVEDS MHIPlay( REG( a3, APTR handle ));

ASM( VOID ) SAVEDS MHIStop( REG( a3, APTR handle ));

ASM( VOID ) SAVEDS MHIPause( REG( a3, APTR handle ));

ASM( ULONG ) SAVEDS MHIQuery( REG( d1, ULONG query ));

ASM( VOID ) SAVEDS MHISetParam( REG( a3, APTR handle ),
                                REG( d0, UWORD param ),
                                REG( d1, ULONG value ));

#endif /* MHIAMIGUS_LIB_PROTOS_H */
