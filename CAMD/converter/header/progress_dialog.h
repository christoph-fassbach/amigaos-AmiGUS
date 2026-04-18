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

#ifndef PROGRESS_DIALOG_H
#define PROGRESS_DIALOG_H

#include <intuition/classes.h>

struct ProgressDialog {

  struct Window    * pd_ParentWindow;
  struct Requester   pd_LockRequest;

  struct Window    * pd_ProgressWindow;
  Object           * pd_ProgressContent;
  struct Gadget    * pd_ProgressGauge;
};

struct ProgressDialog * CreateProgressDialog(
  struct Window * parentWindow,
  CONST_STRPTR title,
  CONST_STRPTR message,
  CONST_STRPTR button );

ULONG ShowProgressDialog( struct ProgressDialog * dialog );

BOOL HandleProgressDialogTick( struct ProgressDialog * dialog,
                               ULONG newValue,
                               ULONG newMax );

VOID CloseProgressDialog( struct ProgressDialog * dialog );

#endif /* PROGRESS_DIALOG_H */