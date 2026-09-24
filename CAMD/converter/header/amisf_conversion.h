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

#ifndef AMISF_CONVERSION_H
#define AMISF_CONVERSION_H

#include <amigus/amisf.h>

struct ConversionInfo;
struct SF2;

struct AmiSF * AllocAmiSFfromSF2(
  struct SF2 * sf2,
  struct ConversionInfo * info,
  struct ProgressDialog * dialog
);


LONG WriteAmiSFtoFile(
  struct AmiSF * amisf,
  STRPTR filePath,
  struct SF2 * sf2,
  struct ConversionInfo * info,
  struct ProgressDialog * dialog
);

APTR GetAmiSfSampleData( struct AmiSF * amisf, struct AmiSF_Sample * sample );

#endif /* AMISF_CONVERSION_H */
