;;
;;
;;    _    __        _      __                           
;;   | |  / /____   (_)____/ /_      __ ____ _ _____ ___ 
;;   | | / // __ \ / // __  /| | /| / // __ `// ___// _ \
;;   | |/ // /_/ // // /_/ / | |/ |/ // /_/ // /   /  __/
;;   |___/ \____//_/ \__,_/  |__/|__/ \__,_//_/    \___/ 
;;                                                       
;;  Copyright (©) Voidware 2018.
;;
;;  Permission is hereby granted, free of charge, to any person obtaining a copy
;;  of this software and associated documentation files (the "Software"), to
;;  deal in the Software without restriction, including without limitation the
;;  rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
;;  sell copies of the Software, and to permit persons to whom the Software is
;;  furnished to do so, subject to the following conditions:
;; 
;;  The above copyright notice and this permission notice shall be included in
;;  all copies or substantial portions of the Software.
;; 
;;  THE SOFTWARE IS PROVIDED "AS IS," WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
;;  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
;;  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
;;  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
;;  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
;;  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
;;  IN THE SOFTWARE.
;; 
;;  contact@voidware.com


    .module fios
	.globl	_ioBuf
    
    .area   _CODE

dos_open          .equ  0x4420
dos_open_exist    .equ  0x4424    
dos_read_byte     .equ  0x13
dos_write_byte    .equ  0x1B
dos_close         .equ  0x4428
dos_seek          .equ  0x4442
dos_read_record   .equ  0x4436
dos_write_record  .equ  0x4439
    
    ;; uchar fopen_exist(FCB f)
_fopen_exist::
    pop  hl
    pop  de                     ;f 
    push de
    push hl
    ld   hl, #_ioBuf
    ld   b,#0
    call dos_open_exist
    jr   nz,.fiofinish
    xor  a,a
    
.fiofinish:
    ;; A contains error
    ld  l,a
    ret

    ;; uchar fopen(FCB f)
_fopen::
    pop  hl
    pop  de                     ;f 
    push de
    push hl
    ld   hl, #_ioBuf
    ld   b,#0                   ;lrl
    call dos_open
    jr   nz,.fo1
    xor  a,a
.fo1:
    jp  .fiofinish

    ;; int fgetc(FCB f)
    ;;  return char or if < 0, LSB = error
_fgetc::
    pop  hl
    pop  de                     ; f
    push de
    push hl
    call dos_read_byte
    jr  nz,.readError
    ld  h,#0
    jp  .fiofinish
.readError:
    ld  h,#0xff
    jp  .fiofinish

    ;; char fputc(char c, FCB f)
_fputc::
    pop  hl
    dec  sp
    pop  af                     ;a=c
    pop  de                     ;f
    push de
    push af
    inc  sp
    push hl
    call dos_write_byte
    jr  nz,.fp1
    xor a,a
.fp1:   
    jp  .fiofinish

    ;; uchar fwrite(const void* buf, FCB f)
    ;; will write one record 
_fwrite::
    pop  bc
    pop  hl                     ;buf
    pop  de                     ;f
    push de
    push hl
    push bc
    call dos_write_record
    jr   nz,.fw1
    xor  a,a
.fw1:   
    jp   .fiofinish

    ;; void fclose(FDB f)
_fclose::
    pop  hl
    pop  de                     ;f
    push de
    push hl
    call dos_close
    ret
    
    
