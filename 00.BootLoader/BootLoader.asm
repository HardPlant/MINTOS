[ORG 0x00] ; Code Start Address : 0x00
[BITS 16]  ; Code set to 16bit

SECTION .text	; Define text Section(Segment)

jmp 0x07C0:START	;CS<-0x07C0, goto START

;;;
;MINT64 OS Environment
;;;
TOTALSECTORCOUNT: dw 0x02 ; MINT64OS Image Size, Max to 1152 Sector(0x90000byte)

;;;
;Code Section
;;;

START:
	mov ax, 0x07C0	;BootLoader start Address->SegRegist value
	mov ds, ax		;set DS
	mov ax, 0xB800 	;intel: dest , src
	mov es, ax		;AX(register)->ES(segment register)
					;SegReg:offset->0xB800:0x0000

	;Stack 0x0000:0x0000~0x0000:FFFF, 64KB size
	mov ax, 0x0000;
	mov ss, ax ; set SS
	mov sp, 0xFFFE
	mov bp, 0xFFFE

	mov si,0 ; init si to 0

.SCREENCLEARLOOP:
	mov byte[es: si],0
	mov byte[es: si+1],0x0A

	add si, 2
	cmp si, 80*25*2

	jl .SCREENCLEARLOOP

	;;;
	;Start Message to Upper Screen
	;;;
	push MESSAGE1	; Str
	push 0			; Y
	push 0			; X
	call PRINTMESSAGE
	add sp, 6
	;;;
	;OS Load Message
	;;;
	push IMAGELOADINGMESSAGE
	push 1
	push 0
	call PRINTMESSAGE
	add sp, 6
	;;;
	;OS Image Load
	;;;
	;;;
	;Disk Reset
	;;;
RESETDISK:
	;;;
	;BIOS Reset Func
	;;;
	; Service number 0, Drive Number 0(Floppy)
	mov ax, 0
	mov dl, 0
	int 0x13
	jc HANDLEDISKERROR

	;;;
	;Read Sector From Disk
	;;;
	;Disk->Memory dest address(ES:BX) -> 0x10000
	mov si, 0x1000
	mov es, si
	mov bx, 0x0000 ; 0x1000:0000

	mov di, word[TOTALSECTORCOUNT]


READDATA:
	cmp di,0
	je READEND
	sub di, 0x1

	;;;;;
	;BIOS Read Function Call
	;;;;;
	mov ah, 0x02 ; BIOS Service Number 2(ReadSector)
	mov al, 0x1 ; SectorCount 2
	mov ch, byte [TRACKNUMBER] ; Set TrackNumber
	mov cl, byte [SECTORNUMBER] ; ..
	mov dh, byte [HEADNUMBER] ; ..
	mov dl, 0x00 ; Driver Number (Floopy=0)
	int 0x13 ; do ISR
	jc HANDLEDISKERROR ; if ERROR occur

	;;;
	; Calculate Copy Address, Track, Head, Sector
	;;;
	add si, 0x0020	; 512Byte to SegReg
	mov es, si 		; ES SegRegister +

	; for Sector to 1->18
	mov al, byte [SECTORNUMBER]
	add al, 0x01
	mov byte [SECTORNUMBER], al
	cmp al, 19
	jl READDATA ; jump if Sector less 19

	xor byte[HEADNUMBER], 0x01 ; toggle Head Number
	mov byte[SECTORNUMBER], 0x01 ; set Sector Number to 1

	;1->0 --> was read bi-head
	;inc track num
	cmp byte[HEADNUMBER], 0x00
	jne READDATA ; if HeadNumber is not 0

	add byte [TRACKNUMBER], 0x01
	jmp READDATA

READEND:
	;;;
	;Print OS Image Load Complete
	;;;
	push LOADINGCOMPLETEMESSAGE
	push 1
	push 20
	call PRINTMESSAGE
	add sp, 6

	;;;
	;run OS Image
	;;;
	jmp 0x1000:0x0000

;;;
;Function Code Section
;;;
HANDLEDISKERROR:
	push DISKERRORMESSAGE
	push 1
	push 20
	call PRINTMESSAGE
	jmp $		; Do Infinity Loop on current location

;Print Message
;PARAM: (x,y,String)
PRINTMESSAGE:
	push bp
	mov bp, sp ; bp<-sp, sp [param][param] -> enables bp driven param

	push es
	push si
	push di
	push ax
	push cx
	push dx

	mov ax, 0xB800
	mov es,ax ; es<- Video Memory Address

	;;;
	;Calculate Video memory to X,Y
	;;;
	; Y First -> Line address
	mov ax, word[bp+6] ;( --> [ret]=bp+2 [x]=bp+4 [y]=bp+6 [str]=bp+8 -->)
	mov si, 160	; 2*80col
	mul si		; AX * SI --> Y Address
	mov di, ax	; Y Address ->di

	;X * 2 --> Final Address
	mov ax, word [bp+4]
	mov si, 2	;one chr byte(2)
	mul si
	add di, ax ; di(Y) + ax(X) --> Real Video Memory Address

	mov si, word [bp+8];

.MESSAGELOOP:
	mov cl, byte[si]
	cmp cl, 0
	je .MESSAGEEND

	mov byte [es: di], cl

	add si,1
	add di,2

	jmp .MESSAGELOOP

.MESSAGEEND: ; context recover
	pop dx
	pop cx
	pop ax
	pop di
	pop si
	pop es
	pop bp	; push -> reversed pop(stack)
	ret		; sp = [ret]
;;;;
;DATA Section
;;;;

;BootLoaderMessage
MESSAGE1: db 'MINT64 OS Boot', 0	;NULL-Terminated

DISKERRORMESSAGE: db 'DISK Error', 0
IMAGELOADINGMESSAGE: db 'OS Image Loading...', 0
LOADINGCOMPLETEMESSAGE: db 'Complete',0

;Disk Read Variables

SECTORNUMBER:		db 0x02;
HEADNUMBER:			db 0x00;

TRACKNUMBER:		db 0x00;


times 510 - ($ - $$) db 0x00	; $: Current Line Address
								; $$: Currrent Section(.text) Start Address
								; $-$$: Current Sect based offset
								; 510 - ( $-$$): current->address 510
								; db 0x00: 1byte declare->0x00
								; time: repeat
								; *(current-->0x510)=0x00

db 0x55
db 0xAA
