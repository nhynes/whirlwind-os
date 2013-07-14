[org 0x7c00]
start:
	xor ax,ax
	mov ds,ax
	mov es,ax
	mov ss,ax
	mov sp,0x7f00

	mov ax,0x00 ;clear the multiboot flags for errors
	mov cx,0x2b
	mov di,0x50
	mov es,di
	xor di,di
	rep stosw

	in al,0x92
	or al,0x02
	out 0x92,al
	call io_wait

	load:
		xor ax,ax
		int 0x13
		mov ah,0x02
		mov al,0x11
		mov ch,0x00
		mov cl,0x02
		mov dh,0x00
		mov bx,0xffff
		mov es,bx
		mov bx,0x10
		int 0x13
		jc short load
		movzx edx,dl ;store the boot device to multiboot info struct
		or edx,0xff00
		mov dword [0x50c],edx


	mov ah,0x00
	mov al,0x03
	int 0x10

	xor ebx,ebx
	mov di,0x55
	mov es,di
	mov edi,0x08
	mov esi,0x01
	memory:
		mov eax,0xe820
		mov ecx,0x18
		mov edx,0x534d4150
		mov dword [es:di+0x14],0x01
		int 0x15
		jc short .process
		cmp eax,0x534d4150
		jnz short .error
		test ebx,ebx
		jz short .process
		test ecx,ecx
		jz short memory
		add di,0x18
		inc si
		jmp short memory
		.error:
			or dword [0x500],0x40 ;mark the mmap flag to be cleared
		.process:
			mov dword [0x52c],esi ;multiboot mmap length
			mov dword [0x530],0x558 ;multiboot mmap address
			xor eax,eax
			xor esi,esi
			mov ecx,dword [0x52c]
			.lowsize:
				;eax=size cx=iter edx=phys location si=map iteration
				jcxz .lowsize_store
				mov edx,dword [0x558+si]
				cmp edx,0xfffff
				jnc short .lowsize_step
				mov edx,dword [0x55c+si]
				test edx,edx
				jnz short .lowsize_step
				add eax,dword [0x560+si]
				.lowsize_step:
					dec cx
					add si,0x18
					jmp short .lowsize
				.lowsize_store:
					mov dword [0x504],eax
			xor eax,eax
			xor esi,esi
			mov ecx,dword [0x52c]
			.highsize:
				;eax=size cx=iter edx=phys location si=map iteration
				jcxz .highsize_store
				mov edx,dword [0x55c+si]
				test edx,edx
				jnz short .highsize_large
				add eax,dword [0x560+si]
				dec cx
				add si,0x18
				jmp short .highsize
				.highsize_large:
					mov dword [0x508],0xffffffff
					jmp short .return
				.highsize_store:
					sub eax,dword [0x504]
					mov dword [0x508],eax
		.return:

	in al,0x21
	mov bl,al
	in al,0xa1
	mov bh,al
	mov al,0x11
	out 0x20,al
	call io_wait
	out 0xa0,al
	call io_wait
	mov al,0x20
	out 0x21,al
	call io_wait
	mov al,0x28
	out 0xa1,al
	call io_wait
	mov al,0x04
	out 0x21,al
	call io_wait
	mov al,0x02
	out 0xa1,al
	call io_wait
	mov al,0x01
	out 0x21,al
	call io_wait
	out 0xa1,al
	call io_wait
	mov bl,al
	out 0x21,al
	mov bh,al
	out 0xa1,al

	cli
	lgdt [ds:gdtr]
	mov eax,cr0
	or al,0x01
	mov cr0,eax
	mov ax,0x10
	mov ds,ax
	mov es,ax
	mov fs,ax
	mov gs,ax
	mov ss,ax

	multiboot:
		mov dword [0x540],'gale'
		xor dword [0x500],0x243 ;001001000011 - no errors
		mov eax,0xdeadbeef
		mov ebx,0x500

	jmp dword 0x08:0x0100000

	gdtr:
		dw _gdt_-_gdt-1
		dd _gdt
		_gdt:
			dd 0x00,0x00
			dd 0x0000ffff,0x00cf9a00
			dd 0x0000ffff,0x00cf9200
		_gdt_:

	io_wait:
		mov al,0x00
		out 0x80,al
		ret

times 510-($-$$) db 0
dw 0xaa55
