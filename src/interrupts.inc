gate_task equ 0x85
gate_int equ 0x8e
gate_trap equ 0x8f

idtmakedesc:
	;eax=offset bl=type cx=descriptor# dx=selector
	mov esi,ecx
	shl esi,3
	add esi,_idt
	cmp eax,0xffff
	jg largeoffset
	mov word [esi],ax
	mov word [esi+6],0x00
	jmp .descr
	largeoffset:
		mov word [esi],ax
		shr eax,16
		mov word [esi+6],ax
	idtmakedesc.descr:
	mov word [esi+2],dx
	mov byte [esi+4],0x00
	mov byte [esi+5],bl
	inc cx
	ret

idtsetdesc:
mov eax,int0
mov bl,gate_int
xor cx,cx
mov dx,0x08
call idtmakedesc
jmp int1setup
int0:
	mov eax,'#de'
	call kprint
	jmp short $
	iretd

int1setup:
mov eax,int1
mov bl,gate_int
mov dx,0x08
call idtmakedesc
jmp int2setup
int1:
	mov eax,'#db'
	call kprint
	jmp short $
	iretd

int2setup:
mov eax,int2
mov bl,gate_int
mov dx,0x08
call idtmakedesc
jmp int3setup
int2:
	mov eax,'#nmi'
	call kprint
	jmp short $
	iretd

int3setup:
mov eax,int3
mov bl,gate_int
mov dx,0x08
call idtmakedesc
jmp int4setup
int3:
	mov eax,'#bp'
	call kprint
	jmp short $
	iretd

int4setup:
mov eax,int4
mov bl,gate_int
mov dx,0x08
call idtmakedesc
jmp int5setup
int4:
	mov eax,'#of'
	call kprint
	jmp short $
	iretd

int5setup:
mov eax,int5
mov bl,gate_int
mov dx,0x08
call idtmakedesc
jmp int6setup
int5:
	mov eax,'#br'
	call kprint
	jmp short $
	iretd

int6setup:
mov eax,int6
mov bl,gate_int
mov dx,0x08
call idtmakedesc
jmp int7setup
int6:
	mov eax,'#ud'
	call kprint
	jmp short $
	iretd

int7setup:
mov eax,int7
mov bl,gate_int
mov dx,0x08
call idtmakedesc
jmp int8setup
int7:
	mov eax,'#nm'
	call kprint
	jmp short $
	iretd

int8setup:
mov eax,int8
mov bl,gate_int
mov dx,0x08
call idtmakedesc
jmp int9setup
int8:
	pop eax
	mov eax,'#df'
	call kprint
	jmp short $
	iretd

int9setup:
mov eax,int9
mov bl,gate_int
mov dx,0x08
call idtmakedesc
jmp intasetup
int9:
	mov eax,'#cso'
	call kprint
	jmp short $
	iretd

intasetup:
mov eax,inta
mov bl,gate_int
mov dx,0x08
call idtmakedesc
jmp intbsetup
inta:
	pop eax
	mov eax,'#ts'
	call kprint
	jmp short $
	iretd

intbsetup:
mov eax,intb
mov bl,gate_int
mov dx,0x08
call idtmakedesc
jmp intcsetup
intb:
	pop eax
	mov eax,'#np'
	call kprint
	jmp short $
	iretd

intcsetup:
mov eax,intc
mov bl,gate_int
mov dx,0x08
call idtmakedesc
jmp intdsetup
intc:
	pop eax
	mov eax,'#ss'
	call kprint
	jmp short $
	iretd

intdsetup:
mov eax,intd
mov bl,gate_int
mov dx,0x08
call idtmakedesc
jmp intesetup
intd:
	pop eax
	call int2str
	mov eax,' #gp'
	call kprint
	jmp short $
	iretd

intesetup:
mov eax,inte
mov bl,gate_int
mov dx,0x08
call idtmakedesc
jmp intfsetup
inte:
	;registers used eax,ebx,ecx,edx,esi
	push eax
	mov eax,' pg '
	call kprint
	pop eax
	pushad

	mov eax,dword [esp+0x20] ;error code
	bt ax,0x00
	jc .other ;present bit is set so it's r/w or s/u

	mov eax,cr2
	mov esi,cr2
	mov edi,0x01
	call pgmap
	cmp eax,0xffffffff
	jz .error

	popad
	add esp,0x04 ;"pop" error code
	iretd

	.other:
		mov edi,eax
		mov eax,'#pfO'
		call kprint
		add word [cursorpos],0x02
		mov eax,edi
		call int2str
		add word [cursorpos],0x02
		mov eax,cr2
		call int2str
		jmp short $

	.error:
		mov eax,'#pfE'
		call kprint
		jmp short $

intfsetup:
mov eax,intf
mov bl,gate_int
mov dx,0x08
call idtmakedesc
jmp int10setup
intf:
	mov eax,'#res'
	call kprint
	iretd

int10setup:
mov eax,int10
mov bl,gate_int
mov dx,0x08
call idtmakedesc
jmp int11setup
int10:
	mov eax,'#mf'
	call kprint
	iretd

int11setup:
mov eax,int11
mov bl,gate_int
mov dx,0x08
call idtmakedesc
jmp int12setup
int11:
	pop eax
	mov eax,'#ac'
	call kprint
	iretd

int12setup:
mov eax,int12
mov bl,gate_int
mov dx,0x08
call idtmakedesc
jmp int13setup
int12:
	mov eax,'#mc'
	call kprint
	iretd

int13setup:
mov eax,int13
mov bl,gate_int
mov dx,0x08
call idtmakedesc
jmp idtreserved-4
int13:
	mov eax,'#xf'
	call kprint
	iretd

mov di,0x00
idtreserved:
mov eax,intreserved
mov bl,gate_int
mov dx,0x08
call idtmakedesc
inc di
cmp di,0x0a
jl idtreserved
jmp int1esetup
intreserved:
	mov eax,'!res'
	call kprint
	iretd

int1esetup:
mov eax,int1e
mov bl,gate_int
mov dx,0x08
call idtmakedesc
jmp int1fsetup
int1e:
	mov eax,'#sx'
	call kprint
	iretd

int1fsetup:
mov eax,int1f
mov bl,gate_int
mov dx,0x08
call idtmakedesc
jmp int20setup
int1f:
	mov eax,'#res'
	call kprint
	iretd

int20setup:
mov eax,int20
mov bl,gate_int
mov dx,0x08
call idtmakedesc
jmp int21setup
int20: ;irq0
	push eax
	;mov eax,'irq0'
	;call kprint
	mov al,0x20
	out 0x20,al
	pop eax
	iretd

int21setup:
mov eax,int21
mov bl,gate_int
mov dx,0x08
call idtmakedesc
jmp int22setup
int21: ;irq1
	;note: arrow keys yield <number pad make> <0xe0> <number pad break>
	push eax
	push ecx
	xor eax,eax
	in al,0x60
	test al,0x80
	jnz .return
	mov ecx,scancodes
	add ecx,eax
	mov al,byte [ecx]
	call kprint
	.return:
	mov al,0x20
	out 0x20,al
	pop ecx
	pop eax
	iretd

int22setup:
mov eax,int22
mov bl,gate_int
mov dx,0x08
call idtmakedesc
jmp int23setup
int22: ;irq2
	mov eax,'irq2'
	call kprint
	mov al,0x20
	out 0x20,al
	iretd

int23setup:
mov eax,int23
mov bl,gate_int
mov dx,0x08
call idtmakedesc
jmp int24setup
int23: ;irq3
	mov eax,'irq3'
	call kprint
	mov al,0x20
	out 0x20,al
	iretd

int24setup:
mov eax,int24
mov bl,gate_int
mov dx,0x08
call idtmakedesc
jmp int25setup
int24: ;irq4
	mov eax,'irq4'
	call kprint
	mov al,0x20
	out 0x20,al
	iretd

int25setup:
mov eax,int25
mov bl,gate_int
mov dx,0x08
call idtmakedesc
jmp int26setup
int25: ;irq5
	mov eax,'irq5'
	call kprint
	mov al,0x20
	out 0x20,al
	iretd

int26setup:
mov eax,int26
mov bl,gate_int
mov dx,0x08
call idtmakedesc
jmp int27setup
int26: ;irq6
	mov eax,'irq6'
	call kprint
	mov al,0x20
	out 0x20,al
	iretd

int27setup:
mov eax,int27
mov bl,gate_int
mov dx,0x08
call idtmakedesc
jmp int28setup
int27: ;irq7
	mov eax,'irq7'
	call kprint
	mov al,0x20
	out 0x20,al
	iretd

int28setup:
mov eax,int28
mov bl,gate_int
mov dx,0x08
call idtmakedesc
jmp int29setup
int28: ;irq8
	mov eax,'irq8'
	call kprint
	mov al,0x20
	out 0xa0,al
	out 0x20,al
	iretd

int29setup:
mov eax,int29
mov bl,gate_int
mov dx,0x08
call idtmakedesc
jmp int2asetup
int29: ;irq9
	mov eax,'irq9'
	call kprint
	mov al,0x20
	out 0xa0,al
	out 0x20,al
	iretd

int2asetup:
mov eax,int2a
mov bl,gate_int
mov dx,0x08
call idtmakedesc
jmp int2bsetup
int2a: ;irq10
	mov eax,'ir10'
	call kprint
	mov al,0x20
	out 0xa0,al
	out 0x20,al
	iretd

int2bsetup:
mov eax,int2b
mov bl,gate_int
mov dx,0x08
call idtmakedesc
jmp int2csetup
int2b: ;irq11
	mov eax,'ir11'
	call kprint
	mov al,0x20
	out 0xa0,al
	out 0x20,al
	iretd

int2csetup:
mov eax,int2c
mov bl,gate_int
mov dx,0x08
call idtmakedesc
jmp int2dsetup
int2c: ;irq12
	mov eax,'ir12'
	call kprint
	mov al,0x20
	out 0xa0,al
	out 0x20,al
	iretd

int2dsetup:
mov eax,int2d
mov bl,gate_int
mov dx,0x08
call idtmakedesc
jmp int2esetup
int2d: ;irq13
	mov eax,'ir13'
	call kprint
	mov al,0x20
	out 0xa0,al
	out 0x20,al
	iretd

int2esetup:
mov eax,int2e
mov bl,gate_int
mov dx,0x08
call idtmakedesc
jmp int2fsetup
int2e: ;irq14
	mov eax,'ir14'
	call kprint
	mov al,0x20
	out 0xa0,al
	out 0x20,al
	iretd

int2fsetup:
mov eax,int2f
mov bl,gate_int
mov dx,0x08
call idtmakedesc
jmp int42setup
int2f: ;irq15
	mov eax,'ir15'
	call kprint
	mov al,0x20
	out 0xa0,al
	out 0x20,al
	iretd

int42setup:
mov eax,int42
mov bl,gate_int
mov cx,0x42
mov dx,0x08
call idtmakedesc
jmp idtsetdone
int42:
	mov eax,'sys'
	call kprint
	iretd

idtsetdone:
	ret
