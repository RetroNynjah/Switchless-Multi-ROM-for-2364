; VIC-20 kernal menu

CHARSET   2
NOLOADADDR
GenerateTo vic20kernalmenu.bin

KRNIMGS = 6    ; number of kernal images in menu/ROM 1..10 where 1 would be quite pointless :)
VICREG  = $9000
CHRC    = $00   ; mnutxt color (VIC-20 default = #$00 )
CHRCHL  = $06   ; menu highlightedtext color

SCRRAM  = $1000
COLRAM  = $9400
LINES   = $d0
NTSC    = $d1
SHIFTS  = $d2 ; 0 = shift not held, 1 = shift held
LASTKEY = $d3 ; last key pressed. used to prevent key repeat
KRNIMG  = $d4

CMDADDR = $d5
;CMDADDR = #4590
PORTA   = $9121
PORTB   = $9120
DDRA    = $9123
DDRB    = $9122


*=$e000
start
        sei
        lda #$00
        sta KRNIMG
        sta LASTKEY

        jsr palntsc
        jsr vicinit
        jsr vicclear

        ;setup VIA port data directions
        lda #%11111111 ; all outputs
        sta DDRA             
        lda #%00000000 ; all inputs
        sta DDRB

        lda #$00
        sta SHIFTS      
        sta LASTKEY     
        lda #$01
        sta KRNIMG     ; initial kernal image = 1

        jsr menu
        jsr hilite
        jsr scankey


menu    ldy #$00
@drwmnu lda mnutxt,y    ; copy menu to screen memory and set char color
        sta SCRRAM,y
        lda #CHRC
        sta COLRAM,y

        lda mnutxt+256,y
        sta SCRRAM+256,y
        lda #CHRC
        sta COLRAM+256,y

        iny
        bne @drwmnu
        rts

hilite 
        pha
        txa
        pha
        tya
        pha
        lda KRNIMG
        jsr vicrow_hl
        pla
        tay
        pla
        tax
        pla
        rts

scankey lda #$00
        cmp $9004       ; wait for raster line 0 to only scan keys once
        bne scankey     ; per frame to debounce keys 
        sta SHIFTS      ; clear old shift pressed indicators
        tay             ; clear pressed key
        ; find out if any shift key is pressed and set #$01 at $shiftkey
        ; check for "Right Shift" row 6, column 4
        lda #%10111111  ; select row 6
        sta PORTA 
        lda PORTB       ; read columns
        and #%00010000  ; check against column bit 4
        beq shifted
        ; check for "Left Shift" row 1, column 7
        lda #%11111101  ; select row 1
        sta PORTA 
        lda PORTB       ; read columns
        and #%00001000  ; check against column bit 3
        beq shifted
        jmp crsrchk
shifted lda #$01
        sta SHIFTS
crsrchk ; check for "cursor" row 7, column 3
        lda #%01111111  ; select row 7
        sta PORTA 
        lda PORTB       ; read columns
        and #%00001000  ; check against column bit 3
        bne retchk
        ldx SHIFTS
        cpx #$01
        beq crsrup
        jmp crsrdn
retchk  ; check for "RETURN" row 7, column bit 1
        lda #%01111111  ; select row 7
        sta PORTA
        lda PORTB       ; read columns
        and #%00000010  ; check against column bit 1
        bne keyhndl
        ldy #$03
        jmp keyhndl
crsrup  ldy #$02
        jmp keyhndl
crsrdn  ldy #$01
        jmp keyhndl
keyhndl cpy LASTKEY
        beq keydone     ; same key as last time - do nothing.
godown  cpy #$01
        bne goup
        lda KRNIMG
        cmp #KRNIMGS     ; already at end of list?
        bcs keydone
        beq keydone
        inc KRNIMG
        jsr hilite
goup    cpy #$02
        bne retpush
        lda KRNIMG
        cmp #$01        ; already at beginning of list
        bcc keydone
        beq keydone
        dec KRNIMG
        jsr hilite
retpush cpy #$03
        bne keydone
        jmp sendcmd
keydone sty LASTKEY
        jmp scankey     ; nothing to do now. keep scanning keys

sendcmd
        jsr vicclear
        ldx #$00
sendasc lda cmdasc,x
        sta CMDADDR,x   ; write command string to data bus to be picked up by kernal switcher
        inx
        cpx #cmdascend-cmdasc   ; length of command string
        bne sendasc
        lda KRNIMG
        sta CMDADDR,x   ; send selected kernal image number to data bus
        ldx #$00
        jmp sendasc

; ascii RNROM20# command for kernal switcher on data bus
cmdasc  byte $52, $4e, $52, $4f, $4d, $32, $30, $23
cmdascend


mnutxt  ; Menu layout
        ; 6 header rows
        text '                      '
        text '      RetroNinja      '
        text '                      '
        text 'VIC-20 Kernal Switcher'
        text '         v1.0         '
        text '                      '
        ; Up to 10 menu choices. number of shown lines controlled by value in $kernalimages
        text 'CBM DOS (EN NTSC)     '
        text 'CBM DOS (EN PAL)      '
        text 'CBM DOS (SW/FI PAL)   '
        text 'JiffyDOS (EN NTSC)    '
        text 'JiffyDOS (EN PAL)     '
        text 'JiffyDOS (SW/FI PAL)  '
        text '                      '
        text '                      '
        text '                      '
        text '                      '
        ; 7 footer rows
        text '                      '
        text 'Use cursor up/down to '
        text 'select a kernal       '
        text '                      '
        text 'RETURN to confirm     '
        text '                      '
        text '                      '


incasm vic.asm
incasm system.asm

*=$fffc ; start vector
;        byte <start, >start, <scankey, >scankey
        byte <start, >start, <start, >start
