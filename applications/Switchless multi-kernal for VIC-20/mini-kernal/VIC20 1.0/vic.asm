
vicinit
        ldx #$10

        lda ntsc
        cmp #1
        bne initpal
initntsc
        lda victblntsc-1,x    ; initialize NTSC VIC
        sta VICREG-1,x
        dex
        bne initntsc
        rts
initpal
        lda victblpal-1,x    ; initialize PAL VIC
        sta VICREG-1,x
        dex
        bne initpal
        rts

victblntsc  byte $05,$19,$16,$2E,$0,$C2,$0,0
            byte 0,0,0,0,0,0,0,$1B
victblpal   byte $0c,$26,$16,$2E,$0,$c2,0,0
            byte $00,$00,0,0,0,0,0,$1B


vicclear        
        ldx #00
        lda #$20
@clr                    ; clear screen
        sta SCRRAM,x
        sta SCRRAM+256,x
        inx
        bne @clr
        rts


; highlight row stored in a
vicrow_hl
        pha
        clc
        adc #$05 ; add five to row index to skip header rows
        sta $dc ; store selected row

        lda #<COLRAM
        ldy #>COLRAM
        sta $da         ; start of data lo byte
        sty $db         ; start of data hi byte

        ldx #0
@row_loop
        ldy #0
        cpx $dc
        beq @hl_on
        bne @hl_off
@hl_off
        ; inactive row, turn off highlight
        lda #CHRC
        jmp @chars
@hl_on
        lda #CHRCHL
@chars
        sta($da),y
        iny
        cpy #22
        bne @chars

@row_done
        inx
        cpx #23
        beq @done
        lda $da
        clc
        adc #22
        sta $da
        bcc @row_loop
        inc $db
        bne @row_loop
@done
        pla
        rts
