;
; Copyright (c) 2019 Anthony Beaucamp.
;
; This software is provided 'as-is', without any express or implied warranty.
; In no event will the authors be held liable for any damages arising from
; the use of this software.
;
; Permission is granted to anyone to use this software for any purpose,
; including commercial applications, and to alter it and redistribute it
; freely, subject to the following restrictions:
;
;   1. The origin of this software must not be misrepresented; you must not
;   claim that you wrote the original software. If you use this software in a
;   product, an acknowledgment in the product documentation would be
;   appreciated but is not required.
;
;   2. Altered source versions must be plainly marked as such, and must not
;   be misrepresented as being the original software.
;
;   3. This notice may not be removed or altered from any distribution.
;
;   4. The names of this software and/or it's copyright holders may not be
;   used to endorse or promote products derived from this software without
;   specific prior written permission.
;

	.export _DHRLine
	.export _Blit

	.segment	"DATA"
		
_dhrLinesHI: 
	.byte $20,$24,$28,$2C,$30,$34,$38,$3C
	.byte $20,$24,$28,$2C,$30,$34,$38,$3C
	.byte $21,$25,$29,$2D,$31,$35,$39,$3D
	.byte $21,$25,$29,$2D,$31,$35,$39,$3D
	.byte $22,$26,$2A,$2E,$32,$36,$3A,$3E
	.byte $22,$26,$2A,$2E,$32,$36,$3A,$3E
 	.byte $23,$27,$2B,$2F,$33,$37,$3B,$3F
	.byte $23,$27,$2B,$2F,$33,$37,$3B,$3F
	.byte $20,$24,$28,$2C,$30,$34,$38,$3C
	.byte $20,$24,$28,$2C,$30,$34,$38,$3C
	.byte $21,$25,$29,$2D,$31,$35,$39,$3D
	.byte $21,$25,$29,$2D,$31,$35,$39,$3D
	.byte $22,$26,$2A,$2E,$32,$36,$3A,$3E
	.byte $22,$26,$2A,$2E,$32,$36,$3A,$3E
 	.byte $23,$27,$2B,$2F,$33,$37,$3B,$3F
	.byte $23,$27,$2B,$2F,$33,$37,$3B,$3F
	.byte $20,$24,$28,$2C,$30,$34,$38,$3C
	.byte $20,$24,$28,$2C,$30,$34,$38,$3C
	.byte $21,$25,$29,$2D,$31,$35,$39,$3D
	.byte $21,$25,$29,$2D,$31,$35,$39,$3D
	.byte $22,$26,$2A,$2E,$32,$36,$3A,$3E
	.byte $22,$26,$2A,$2E,$32,$36,$3A,$3E
 	.byte $23,$27,$2B,$2F,$33,$37,$3B,$3F
	.byte $23,$27,$2B,$2F,$33,$37,$3B,$3F
	
_dhrLinesLO: 
	.byte $00,$00,$00,$00,$00,$00,$00,$00
 	.byte $80,$80,$80,$80,$80,$80,$80,$80
	.byte $00,$00,$00,$00,$00,$00,$00,$00
 	.byte $80,$80,$80,$80,$80,$80,$80,$80
	.byte $00,$00,$00,$00,$00,$00,$00,$00
 	.byte $80,$80,$80,$80,$80,$80,$80,$80
	.byte $00,$00,$00,$00,$00,$00,$00,$00
 	.byte $80,$80,$80,$80,$80,$80,$80,$80
 	.byte $28,$28,$28,$28,$28,$28,$28,$28
 	.byte $A8,$A8,$A8,$A8,$A8,$A8,$A8,$A8
 	.byte $28,$28,$28,$28,$28,$28,$28,$28
 	.byte $A8,$A8,$A8,$A8,$A8,$A8,$A8,$A8
 	.byte $28,$28,$28,$28,$28,$28,$28,$28
 	.byte $A8,$A8,$A8,$A8,$A8,$A8,$A8,$A8
 	.byte $28,$28,$28,$28,$28,$28,$28,$28
 	.byte $A8,$A8,$A8,$A8,$A8,$A8,$A8,$A8
 	.byte $50,$50,$50,$50,$50,$50,$50,$50
 	.byte $D0,$D0,$D0,$D0,$D0,$D0,$D0,$D0
 	.byte $50,$50,$50,$50,$50,$50,$50,$50
 	.byte $D0,$D0,$D0,$D0,$D0,$D0,$D0,$D0
 	.byte $50,$50,$50,$50,$50,$50,$50,$50
 	.byte $D0,$D0,$D0,$D0,$D0,$D0,$D0,$D0
 	.byte $50,$50,$50,$50,$50,$50,$50,$50
 	.byte $D0,$D0,$D0,$D0,$D0,$D0,$D0,$D0

_mainAuxTog: .res 1

	.segment	"LOWCODE"

; ---------------------------------------------------------------
; int __near__ _DHRLine (char line)
;	Return address of DHR line
; ---------------------------------------------------------------	
	
.proc _DHRLine: near

	; Save DHR address to registers A/X
	tay
	ldx _dhrLinesHI,y
	lda _dhrLinesLO,y
	rts
.endproc	
	
; ---------------------------------------------------------------
; void __near__ Blit (void)
;	Fast copy data between PROGRAM and DHR memory
;	Zero Page Data:
;		$e3: Number of bytes per row
;		$eb: Number of rows
;		$ec: DHR offset X
;		$ed: DHR offset Y
;		$ee: 16 bit output address (optional)
;		$fa: 16 bit input address (optional)
;
;		$fc: 16 bit address of DHR line (generated from offsets)
; ---------------------------------------------------------------	

.proc _Blit: near

	; Init Main/Aux Toggle
	lda #0
	sta _mainAuxTog

	; X loop: Number of lines
	ldx $eb
loopRow:
	; Copy from DHR Tables (with Line Offset Y and Byte Offset X) to $fc/$fd
	ldy $ed				; Y offset within table
	lda _dhrLinesHI,y
	sta $fd
	lda _dhrLinesLO,y
	adc $ec				; Add X Offset
	sta $fc

	; Main/Aux Toggle
branchAux:	
	sta $c055		; Switch to AUX memory
	jmp switchDone
branchMain:
	sta $c054		; Switch to MAIN memory
switchDone:
	
	; Copy bytes from DHR to ouput	
screen2output:
	lda $ef
	beq input2screen  ; If high-byte is zero, then skip
	ldy #0			; Y loop: Copy ? bytes (see $e3)
loopCopy1:
	lda ($fc),y		; Copy 1 byte
	sta ($ee),y
	iny				; Iterate Y loop
	cpy $e3
	bne loopCopy1
incAddress1:
	clc				; Increment address of output block
	lda $ee			
	adc $e3			; Move ? bytes (see $e3)
	sta $ee	
	bcc nocarry1	; Check if carry to high-byte
	inc $ef
nocarry1:
	
	; Copy bytes from input to DHR
input2screen:	
	lda $fb
	beq toggleBlocks  ; If high-byte is zero, then skip	
	ldy #0			; Y loop: Copy ? bytes (see $e3)
loopCopy2:
	lda ($fa),y		; Copy 1 byte
	sta ($fc),y
	iny				; Iterate Y loop
	cpy $e3
	bne loopCopy2
incAddress2:
	clc				; Increment address of input block
	lda $fa			
	adc $e3			; Move ? bytes (see $e3)
	sta $fa	
	bcc nocarry2	; Check if carry to high byte
	inc $fb
nocarry2:
	
toggleBlocks:
	; Process Main Block?
	clc
	lda _mainAuxTog
	eor #1
	sta _mainAuxTog
	bne branchMain

nextRow:
	; Move to next row
	inc $ed			; Increment Y-Line offset in DHR Table
	clc
	dex				; Iterate X loop
	bne loopRow	
	rts
.endproc
