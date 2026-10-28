/*
 * This file is part of the SoundFontConverter.
 *
 * SoundFontConverter is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, version 3 of the License only.
 *
 * SoundFontConverter is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with SoundFontConverter.
 *
 * If not, see <http://www.gnu.org/licenses/>.
 */

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