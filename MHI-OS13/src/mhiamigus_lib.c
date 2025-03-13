/*
 * This file is part of the mhiAmiGUS.library.
 *
 * mhiAmiGUS.library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, version 3 of the License only.
 *
 * mhiAmiGUS.library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU LesserGeneral Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with mhiAmiGUS.library.  If not, see <http://www.gnu.org/licenses/>.
 */

#include    <exec/types.h>
#include    <clib/exec_protos.h>
#include    <pragmas/exec_pragmas.h>

/* These prototypes are just to keep the compiler happy since we don't
** want them in the distributable proto file.
*/

int __saveds __UserLibInit(void);
void __saveds __UserLibCleanup(void);

/* Following global item (UtilityBase) is created so our library can make
** utility.library calls.  Technically putting it in the near section is a waste
** of memory, but for this example, it serves its purpose.  Note also that we
** don't actually MAKE any Utility calls, but we COULD.
*/

struct Library *SysBase = NULL;

/*
*/

int __saveds
__UserLibInit( void )
{
    int retval = 1;

    SysBase = (*((void **)4));

    /* Here we attempt to open Utility library.  Not a particularly good
    ** example, but it gets the point across.  If exec.library could not
    ** be opened (say for a second it couldn't), then __UserLibInit would
    ** return 1 indicating failure (where a return of 0 means success).
    */

//    if (UtilityBase = OpenLibrary( "utility.library", 0L ))
        retval = 0;

    return( retval );
}


/* About as basic a routine as you can get, this routine cleans up the library
** by providing a matching CloseLibrary of the Utility library base we opened
** in the __UserLibInit() routine.
*/

void __saveds
__UserLibCleanup( void )
{
//    CloseLibrary( UtilityBase );
}
