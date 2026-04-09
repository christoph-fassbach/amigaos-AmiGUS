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

#ifndef ERRORS_H
#define ERRORS_H

/**
 * Enumeration of all possible error codes.
 */
enum ErrorCodes
  {
  ENoError = 0,
  /* Insert errors below. */
  /* Global errors 0x0001 - 0x00ff */
  EWrongDriverCPUVersion,
  EOpenCamdBase,
  EOpenDosBase,
  EOpenGfxBase,
  EOpenIntuitionBase,
  EOpenButtonBase,
  EOpenGetFileBase,
  EOpenLabelBase,
  EOpenLayoutBase,
  EOpenListBrowserBase,
  EOpenUtilityBase,
  EOpenWindowBase,
  ECreateMessagePort,
  EOpenLogFile,
  EAllocateLogMem,
  EAllocateAmiGUSCAMDToolBase,
  ENoMidiNodes,
  ECreateMidi,
  EAddMidiLink,
  ENoAmigusFound,
  ENoAmigusRetry,

  /* Missing implementation 100-199*/
  ESomethingNotImplemented = 100,  

  /* amigus.library errors 0x0100 - 0x05ff */

  /* SF2 read errors 0x1000 - 0x10ff */
  EOpenFileFailed = 0x1000,
  EInvalidFileSize,
  EParseFailed,
  ENoRiffChunk,
  ENoSfbkChunk,
  ENoInfoChunk,
  ENoSdtaChunk,
  ENoPdtaChunk,
  ENoSmplChunk,
  EInvalidInfoChunk,
  EInvalidVersionChunk,
  EIncompatibleVersion,
  EInvalidInfoSubChunkSize,
  EInvalidPresetSubChunk,
  EInvalidPresetDataSize,
  ENoPresets,
  EInvalidPresetIndex,
  EInvalidGeneratorIndex,
  EInvalidModulatorIndex,
  EInvalidPresetBags,
  EInvalidPresetModulators,
  EDuplicatedSmplChunk,
  EDuplicatedSm24Chunk,

  /* Insert errors above. */
  EUnknownError,
  EAmountOfErrors
  };

#endif /* ERRORS_H */
