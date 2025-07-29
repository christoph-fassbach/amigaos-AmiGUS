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
enum ErrorCodes
  {
  ENoError = 0,
  /* Insert errors below. */
  /* Global errors 1-99 */
  EWrongDriverCPUVersion,
  EOpenDosBase,
  EOpenIntuitionBase,
  EOpenGadToolsBase,
  EOpenReactionBases,
  EOpenLogFile,
  EAllocateLogMem,
  EAllocateAmiGUSCAMDToolBase,

  /* Missing implementation 100-199*/
  ESomethingNotImplemented = 100,  

  /* Hardware errors 200-299 */
  EAmiGUSNotFound = 200,
  EAmiGUSDetectError,
  EAmiGUSFirmwareOutdated,

  /* Hardware errors 300-399 */
  ELockPubScreen = 300,
  EGadToolsGetVisualInfo,
  EGadToolsCreateContext,

  /* Insert errors above. */
  EUnknownError,
  EAmountOfErrors
  };

#endif /* ERRORS_H */
