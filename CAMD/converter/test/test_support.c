#include <stdio.h>

#include "support.h"

BOOL TestC_strncat( VOID ) {

  BOOL failed;
  STRPTR a = "banana!abcdefghijkl";             /* length 20   */
  STRPTR b = "cucumber salad!";                 /* length 16 */
  STRPTR c = "1234567890";                      /* length 10   */

  C_strncat( a, 20, 2, b, c );
  failed = !( C_strcmp( a, "cucumber salad!1324" ));
  printf( "result: '%s' -> %s\n", 
          a,
          failed ? "FAIL!" : "OK." );

  return failed;
}

int main( VOID ) {

  TestC_strncat();
}