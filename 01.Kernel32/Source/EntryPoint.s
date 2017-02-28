[ORG 0x00]
[BITS 16]

SECTION .text	;text Section

START:
	mov ax, 0x1000	; Entry Address -> Segment Register

	mov ds, ax
	mov es, ax

	;;;
	;Enable A20 Gate
	;try BIOS, then SysCtrlPort
	;;;
	mov ax,0x2401 ; set service to actviate A20 gate
	int 0x15	; call BIOS interrupt

	jc .A20GATEERROR ; check if successes
	jmp .A20GATESUCCESS

	.A20GATEERROR:
		in al,0x92
		or al, 0x02
		and al, 0xFE
		out 0x92, al

	.A20GATESUCCESS:
		cli
		lgdt[GDTR]
		mov eax, 0x4000003B	;PG=0, CD=1, NW=0, AM=0,WP=0,
							;NE=1, ET=1, TS=1, EM=0,MP=1,PE=1
		mov cr0, eax	; copy to CR0 -> go to protect mode


		jmp dword 0x08: (PROTECTEDMODE - $$ + 0x10000) ; +offset



[BITS 32]
PROTECTEDMODE:
	mov ax, 0x10	;Protected Mode Kernel DS Descriptor->AX
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	;stack 0x0000 0000-FFFF, 64KB size
	mov ss, ax
	mov esp, 0xFFFE
	mov ebp, 0xFFFE
	;print change completed message

	push(SWITCHSUCCESSMESSAGE - $$ + 0x10000)
	push 2
	push 0

	call PRINTMESSAGE
	add esp, 12
	jmp dword 0x08: 0x10200 ; CS Segment Selector -> Kernel Code Descriptor(0x08)
							; jmp to C Lang Kernel


;Prints Message
;x, y, str
PRINTMESSAGE:
	push ebp
	mov ebp, esp
	push esi
	push edi
	push eax
	push ecx
	push edx	;function prologue

	;;
	;Get VideoMemory Address
	;;

	;Line address First, use Y
	mov eax, dword [ebp+12] ; param 2
	mov esi, 160	;2byte * 80col
	mul esi
	mov edi, eax

	;X, 2byte

	mov eax, dword [ ebp+8 ] ; param 1
	mov esi, 2
	mul esi

	add edi, eax ; join X,Y


	mov esi, dword [ebp+16];

.MESSAGELOOP:
	mov cl, byte[esi]	; cl -> ecx low bit

	cmp cl, 0
	je .MESSAGEEND

	mov byte [edi+0xB8000], cl

	add esi, 1
	add edi, 2

	jmp .MESSAGELOOP
.MESSAGEEND:
	pop edx
	pop ecx
	pop eax
	pop edi
	pop esi
	pop ebp

	ret

;;
;Data Section
;;
;8Byte sort
align 8, db 0

;
dw 0x0000 ; end of GDTR -> 8bit

GDTR: ; GDTR Data Structure
	dw GDTEND - GDT - 1; GDT Table Total Size
	dd (GDT - $$ + 0x10000);GDT Table Start Address (GDT offset + segmant base addr 0x10000)
GDT:
	NULLDescriptor:
		dw 0x0000
		dw 0x0000
		db 0x00
		db 0x00
		db 0x00
		db 0x00
	CODEDESCRIPTOR:
		dw 0xFFFF	;Limit[15:0]
		dw 0x0000	;Base[15:0]
		db 0x00	;Base [23:16]
		db 0x9A	;P=1, DPL=0, Code Segment, Execute/Read
		db 0xCF	;G=1, D=1, L=0, Limit[19:16]
		db 0x00	;base [31:24]
	DATADESCRIPTOR:
		dw 0xFFFF
		dw 0x0000
		db 0x00
		db 0x92	;Read/Write
		db 0xCF	;
		db 0x00
GDTEND:

SWITCHSUCCESSMESSAGE: db 'Switched to Protected Mode', 0

times 512 - ($ - $$) db 0x00 ; 512byte

