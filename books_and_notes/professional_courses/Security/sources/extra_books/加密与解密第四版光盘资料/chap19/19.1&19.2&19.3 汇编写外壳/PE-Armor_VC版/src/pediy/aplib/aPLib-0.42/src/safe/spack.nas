;;
;; aPLib compression library  -  the smaller the better :)
;;
;; NASM safe assembler wrapper for aP_pack
;;
;; Copyright (c) 1998-2004 by Joergen Ibsen / Jibz
;; All Rights Reserved
;;
;; http://www.ibsensoftware.com/
;;

; header format:
;
;  offs  size    data
; --------------------------------------
;    0   dword   tag ('AP32')
;    4   dword   header_size (24 bytes)
;    8   dword   packed_size
;   12   dword   packed_crc
;   16   dword   orig_size
;   20   dword   orig_crc

bits 32

%include "nasmlcm.inc"

section lcmtext

lcmglobal aPsafe_pack,24

lcmexport aPsafe_pack,24

lcmextern aP_pack,24
lcmextern aP_crc32,8

; =============================================================

lcmlabel aPsafe_pack,24
    ; unsigned int aPsafe_pack(const void *source,
    ;                          void *destination,
    ;                          unsigned int length,
    ;                          void *workmem,
    ;                          int (*callback)(unsigned int, unsigned int, void *),
    ;                          void *cbparam);

    .ret$  equ 7*4
    .src$  equ 8*4 + 4
    .dst$  equ 8*4 + 8
    .len$  equ 8*4 + 12
    .wmem$ equ 8*4 + 16
    .cb$   equ 8*4 + 20
    .cbp$  equ 8*4 + 24

    pushad

    mov    ebp, esp

    mov    esi, [ebp + .src$] ; esi -> inbuffer
    mov    edi, [ebp + .dst$] ; edi -> outbuffer
    mov    ecx, [ebp + .len$] ; ecx =  length

    or     eax, byte -1       ; eax = -1

    test   esi, esi           ; check parameters
    jz     short .return_eax  ;
    test   edi, edi           ;
    jz     short .return_eax  ;
    test   ecx, ecx           ;
    jz     short .return_eax  ;

    mov    ebx, 032335041h
    mov    [edi], ebx         ; set header.tag
    mov    ebx, 24
    mov    [edi + 4], ebx     ; set header.header_size

    add    ebx, edi           ; ebx -> destination for packed data

    mov    [edi + 16], ecx    ; set header.orig_size

    push   ecx
    push   esi
    call   aP_crc32
    lcmadjust 8               ; adjust stack (if function is cdecl)

    mov    [edi + 20], eax    ; set header.orig_crc

    push   dword [ebp + .cbp$] ; callback param
    push   dword [ebp + .cb$] ; callback
    push   dword [ebp + .wmem$] ; workmem
    push   ecx                ; length
    push   ebx                ; destination
    push   esi                ; source
    call   aP_pack
    lcmadjust 24              ; adjust stack (if function is cdecl)

    cmp    eax, -1
    je     short .return_eax

    mov    [edi + 8], eax     ; set header.packed_size

    mov    edx, eax           ; edx = packed size

    push   eax
    push   ebx
    call   aP_crc32
    lcmadjust 8               ; adjust stack (if function is cdecl)

    mov    [edi + 12], eax    ; set header.packed_crc

    lea    eax, [edx + 24]    ; eax = packed size + header size

  .return_eax:
    mov    [esp + .ret$], eax ; return unpacked length in eax

    popad
    lcmret 24

; =============================================================

%ifdef LCM_OBJ
  section lcmdata
%endif

; =============================================================
