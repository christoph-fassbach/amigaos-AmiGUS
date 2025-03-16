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

#include <proto/exec.h>
#include <proto/intuition.h>

#include "amigus_mhi.h"
#include "support.h"

VOID ShowError( STRPTR title, STRPTR message, STRPTR button ) {

  struct EasyStruct req;

  req.es_StructSize = sizeof( struct EasyStruct );
  req.es_Flags = 0;
  req.es_Title = title;
  req.es_TextFormat = "Error %ld : %s";
  req.es_GadgetFormat = button;

  EasyRequest( NULL, &req, NULL, 0, message );
}

VOID ShowAlert( ULONG alertNum ) {

  Alert( alertNum );
}