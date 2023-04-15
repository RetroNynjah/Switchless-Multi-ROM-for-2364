
palntsc
        lda #$00
        tay
raszero
        ; wait for raster line 0
        lda $9004
        beq rasstart
        jmp raszero
rasstart
        ; wait for raster line other than 0
        lda $9004
        beq rasstart
rascount
        ; couunt until raster back at 0
        lda $9004
        beq rasend
        tay
        jmp rascount
rasend
        sty lines
        ;9b/155 pal
        ;82/130  ntsc
        ; lets use #$96 as treshold 
        lda #$96
        cmp lines
        bcs ntscsys
        ; PAL detecetd
        lda #0
        sta ntsc
        rts
ntscsys
        ; NTSC detected
        lda #1
        sta ntsc
        rts
