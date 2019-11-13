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


    .module fios4
	.globl	_ioBuf
    
    .area   _CODE

svc_open          .equ  59
svc_init          .equ  58
svc_close         .equ  60
svc_get           .equ  3
svc_put           .equ  4
    
    ;; uchar fopen_exist4(FCB f)
_fopen_exist4::
    pop  hl
    pop  de                      ;f
    push de
    push hl
    ld   hl, #_ioBuf
    ld   b,#0
    ld   a,#svc_open
    rst  0x28
    jr   nz,.fiofinish
    xor  a,a
    
.fiofinish:
    ;; A contains error
    ld  l,a
    ret

    ;; uchar fopen4(FCB f)
_fopen4::
    pop  hl
    pop  de                     ;f 
    push de
    push hl
    ld   hl, #_ioBuf
    ld   b,#0                   ;lrl
    ld   a,#svc_init
    rst  0x28
    jr   nz,.fo1
    xor  a,a
.fo1:
    jp  .fiofinish

    ;; int fgetc4(FCB f)
    ;;  return char or if < 0, LSB = error
_fgetc4::
    pop  hl
    pop  de                     ; f
    push de
    push hl
    ld   a,#svc_get
    rst  0x28
    jr  nz,.readError
    ld  h,#0
    jp  .fiofinish
.readError:
    ld  h,#0xff
    jp  .fiofinish

    ;; char fputc4(char c, FCB f)
_fputc4::
    pop  hl
    dec  sp
    pop  bc                     ;b=char
    pop  de                     ;f
    push de
    push bc
    inc  sp
    push hl
    ld   c,b
    ld   a,#svc_put
    rst  0x28
    jr  nz,.fp1
    xor a,a
.fp1:   
    jp  .fiofinish

    ;; void fclose4(FDB f)
_fclose4::
    pop  hl
    pop  de                     ;f
    push de
    push hl
    ld   a,#svc_close
    rst  0x28
    ret
    
    
