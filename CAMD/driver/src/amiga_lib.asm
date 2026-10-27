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

    INCLUDE 'utility/hooks.i'

    xdef _DoMethod

    section text,code

_DoMethod:
	move.l	a2,-(a7)
	move.l	8(sp),a2
	move.l	a2,d0
	beq.s	return_null_result
	lea		12(sp),a1
	move.l	-4(a2),a0
	pea.l	return_hook_result(pc)
	move.l	h_Entry(a0),-(sp)
	rts

return_null_result:

	moveq.l	#0,d0

return_hook_result:

	move.l	(sp)+,a2
	rts

    end
