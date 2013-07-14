[bits 32]

global storm

extern _bss
extern _bss_
extern _kernel_

stacksize equ 0x200

section .rodata
align 4
strings db "0123456789abcdef"
scancodes db "# 1234567890-=  qwertyuiop[]  asdfghjkl;'` \zxcvbnm,./ *   ffffffffff# 789-456+1230.  ff"
error_boot db "An error occured during boot. Contents of eax: ",0

section .text
align 4
	storm:
		;checks for successful boot
		cmp eax,0xdeadbeef
		je .storm
		push eax
		push error_boot
		call kprintf
		pop eax
		call int2str
		jmp short $


		.storm:
		;reloads the GDT
		lgdt [gdtr]


		;sets up the stack
		cld
		mov ecx,_kernel_
		sub ecx,_bss
		shr ecx,0x02
		mov eax,0x00
		mov edi,_bss
		rep stosd
		mov esp,stack+stacksize
		mov ebp,esp


		;stows the location of the multiboot header
		mov dword [multiboot],ebx


		;enables interrupts
		call idtsetdesc
		lidt [idtr]
		sti


		;memory initialization
		call mmlist
		add dword [cursorpos],0xa0
		call mmapprep
		call stackifypg
		call enpg
		call mmlist
		add dword [cursorpos],0xa0
		mov ebx,dword [cursorpos]
		call movcursor


		;testing block TODO: free
		push 0x08
		call malloc
		jmp $
		push eax
		call free
		jmp $
		;end testing block


		;done
		idle:
			rdtsc
			mov dword [0xb8f98],0x076f0764
			mov dword [0xb8f9c],0x0765076e
			.idle:
				hlt
				jmp short .idle

	gdtr:
		dw _gdt_-_gdt-0x01
		dd _gdt
		_gdt:
			dd 0x00,0x00
			dd 0x0000ffff,0x00cf9a00
			dd 0x0000ffff,0x00cf9200
		_gdt_:

	idtr:
		dw _idt_-_idt-0x01
		dd _idt
		_idt:
			times 0x43 dq 0x00
		_idt_:

	%include "interrupts.inc"
	%include "mmlist.inc"
	%include "video.inc"
	%include "memory.inc"

section .bss
align 4
stack resd stacksize
cursorpos resd 0x01
multiboot resd 0x01
_memstack_ resd 0x01
