kprintf:
	;void kprintf(char* str)
	mov esi,dword [esp+0x04]
	mov ebx,dword [cursorpos]
	add ebx,0xb8000
	.print:
		test esi,esi
		jz .return
		lodsb
		cmp al,0x00
		jz .return
		cmp al,0x0a
		jz .newline
		cmp al,0x0b
		jz .tab
		mov byte [ebx],al
		inc ebx
		mov byte [ebx],0x07
		inc ebx
		jmp .print
	.newline:
		sub ebx,0xb8000
		mov eax,ebx
		mov ebx,0xa0
		xor edx,edx
		div ebx
		inc eax
		mul ebx
		mov ebx,eax
		add ebx,0xb8000
		jmp .print
	.tab:
		sub ebx,0xb8000
		mov eax,ebx
		mov ebx,0x08
		xor edx,edx
		div ebx
		inc eax
		mul ebx
		mov ebx,eax
		add ebx,0xb8000
		jmp .print
	.return:
		sub ebx,0xb8000
		mov dword [cursorpos],ebx
		call movcursor
		ret 0x04

kprint:
	;eax=string || esi=pointer to string || eax=spaces
	pushad
	mov ebx,dword [cursorpos]
	add ebx,0xb8000
	.print:
		cmp al,0x00
		jz .return
		mov byte [ebx],al
		inc ebx
		mov byte [ebx],0x07
		inc ebx
		shr eax,0x08
		jmp .print
	.space:
		pushad
		mov ebx,dword [cursorpos]
		add ebx,0xb8000
		shl eax,0x01
		add ebx,eax
	.return:
		sub ebx,0xb8000
		mov dword [cursorpos],ebx
		call movcursor
		popad
		ret

int2str:
	;eax=hex esi=temp ecx=iteration
	pushad
	rol eax,0x04
	mov ecx,eax
	mov eax,'0x'
	call kprint
	mov eax,ecx
	xor ecx,ecx
	.convert:
		cmp cx,0x08
		je .return
		mov esi,eax
		and esi,0x0f
		push eax
		xor eax,eax
		mov al,byte [strings+esi]
		call kprint
		pop eax
		rol eax,0x04
		inc cx
		jmp .convert
	.return:
		popad
		ret

itoa:
	;char* itoa(int num)
	push ebp
	mov ebp,esp
	sub esp,0x28

	mov word [ebp-0x28],'0x'
	rol eax,0x04
	mov ecx,0x08
	.convert:
		jecxz .return
		mov esi,eax
		and esi,0x0f
		push eax
		xor eax,eax
		mov al,byte [strings+esi]
		mov ebx,ebp
		sub ebx,ecx
		mov byte [ebx-0x1e],al
		pop eax
		rol eax,0x04
		dec ecx
		jmp .convert
	.return:
		mov byte [ebp-0x1e],0x00
		mov esi,esp		
		mov esp,ebp
		pop ebp
		ret

movcursor:
	;ebx = cursorpos
	;this function is massive and probably slow
	;I dislike it...
	;and I dislike how "out" is so restrictive
	mov dx,word [0x463]
	shr ebx,0x01
	mov al,0x0f
	out dx,al
	mov eax,ebx
	and eax,0xff
	inc dx
	out dx,al
	dec dx
	mov al,0x0e
	out dx,al
	mov eax,ebx
	shr eax,0x08
	and eax,0xff
	inc dx
	out dx,al
	dec dx
	ret
