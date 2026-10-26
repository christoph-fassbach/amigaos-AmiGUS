/*
 * This file is part of the AmiGUS CAMD MIDI driver.
 *
 * AmiGUS CAMD MIDI driver is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, version 3 of the License only.
 *
 * AmiGUS CAMD MIDI driver is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with AmiGUS CAMD MIDI driver.
 *
 * If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ERRORS_H
#define ERRORS_H

/**
 * Enumeration of all possible error codes.
 */
enum ErrorCodes {

  ENoError = 0,
  /* Insert errors below. */
  /* Global errors 0x0001 - 0x00ff */
  EWrongDriverCPUVersion,
  ELibraryBaseInconsistency,
  EOpenAmiGusBase,
  EOpenCamdBase,
  EOpenDosBase,
  EOpenGfxBase,
  EOpenIntuitionBase,
  EOpenButtonBase,
  EOpenDoubBasBase,
  EOpenDoubTransBase,
  EOpenFuelGaugeBase,
  EOpenGetFileBase,
  EOpenLabelBase,
  EOpenLayoutBase,
  EOpenListBrowserBase,
  EOpenSliderBase,
  EOpenUtilityBase,
  EOpenWindowBase,
  ECreateMessagePort,
  EOpenLogFile,
  EAllocateLogMem,
  EAllocateAmiGUSCAMDToolBase,
  EHandleUnknown,
  EAllocateAmiGUSCAMDBase,
  EAllocateInterrupt,
  EAllocateHandle,
  EAllocateBuffer,
  EDriverInUse,
  EDriverNotInUse,
  EWorkerProcessCreationFailed,
  EWorkerProcessDied,
  EWorkerProcessSignalsFailed,
  EMainProcessSignalsFailed,
  EAmiGUSNotFound,
  EAmiGUSDetectError,
  EAmiGUSFirmwareOutdated,
  ENoMidiNodes,
  ECreateMidi,
  EAddMidiLink,
  ENoAmigusFound,
  ENoAmigusNoRetry,
  ENoAmigusRetry,
  EAllocateProgressDialog,

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
  EInvalidBags,
  EInvalidModulators,
  EInvalidGenerators,
  EInvalidInstrumentDataSize,
  ENoInstruments,
  EInvalidInstrumentIndex,
  EInvalidInstrumentGenerator,
  EInvalidSampleGenerator,
  EDuplicatedSmplChunk,
  EDuplicatedSm24Chunk,
  EInvalidSampleSize,

  /* AmiSF read errors  0x1100 - 0x11ff */
  EOpenAmiSFreadFailed = 0x1100,

  /* AmiSF write errors 0x1200 - 0x12ff */
  EAmiSFwriteOpenFailed = 0x1200,
  EAmiSFwriteAborted,
  EAmiSFwriteError,
  EAmiSFsampleWriteError,

  /* Insert errors above. */
  EUnknownError = 0x1600,
  EAmountOfErrors
};

#endif /* ERRORS_H */
