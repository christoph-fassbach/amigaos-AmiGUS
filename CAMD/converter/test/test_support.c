#include "support.h"

void TestC_strcatn( VOID ) {

  STRPTR a = "banana!abcdefghijkl";             /* length 20   */
  STRPTR b = "cucumber salad!";                 /* length 16 */
  STRPTR c = "1234567890";                      /* length 10   */

  C_strcatn( a, 20, 2, b, c );

  printf( "result: '%s'\n", a );
}
#if 0
  STRPTR a = "banana!\0\0\0\0\0\0\0\0\0\0\0\0"; /* length 20 */
  STRPTR b = "cucumber!\0\0";                   /* length 12 */
  STRPTR c = "---------";                       /* length 10 */
  STRPTR d = C_strcat_VD(5, "Hallo ", "Mama ", "", "wie ", "gehts?");

  b = C_strcat(b, c, 12);
  LOG_INFO(Fmt("'cucumber!--' = '%s' while '---------' = '%s'", b, c));
  C_strcat(a, b, 20);
  LOG_INFO(Fmt("'banana!cucumber!--' = '%s'", a));
  LOG_INFO(Fmt("Expected: 'Hallo Mama wie gehts?' = '%s'", d));

  FreeVec(d);
cucumber!---------

}
#endif
int main() {

  TestC_strcatn();
}