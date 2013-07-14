mmapprep:
	;ebx=entry location cx=iteration dx=length
	mov edx,dword [multiboot]
	mov ebx,edx
	mov edx,dword [edx+0x2c]
	mov ebx,dword [ebx+0x30]
	xor ecx,ecx
	.memsort:
		cmp cx,dx
		jz .startgroup
		test cx,cx
		jnz .increment
		inc cx
		add ebx,0x18
		jmp short .memsort
		mov eax,dword [ebx]
		cmp eax,dword [ebx-0x18]
		jc .sort
		.increment:
		inc cx
		add ebx,0x18
		jmp short .memsort

		.sort:
		push ecx
		mov ecx,0x0c
		mov edi,esp
		sub edi,0x30
		mov esi,ebx
		sub esi,0x18
		rep movsd

		mov ecx,0x06
		mov esi,esp
		sub esi,0x18
		mov edi,ebx
		sub edi,0x18
		rep movsd

		mov ecx,0x06
		mov esi,esp
		sub esi,0x30
		mov edi,ebx
		rep movsd
		
		pop ecx
		dec cx
		sub ebx,0x18
		jmp .memsort

	.startgroup:
	;ebx=entry location cx=iterator edx=map data ebp=entries
	push ebp
	mov ecx,dword [multiboot]
	mov ebx,ecx
	mov ecx,dword [ecx+0x2c]
	mov ebp,ecx
	mov ebx,dword [ebx+0x30]
	add ebx,0x10
	.memgroup:
		jecxz .return
		mov edx,dword [ebx]
		cmp edx,dword [ebx-0x18]
		jz .memgroupcont
		dec ecx
		add ebx,0x18
		jmp short .memgroup

		.memgroupcont:
		mov edx,dword [ebx-0x10]
		sub edx,dword [ebx-0x28]
		add edx,dword [ebx-0x08]
		mov dword [ebx-0x20],edx

		push ecx
		mov esi,ebx
		add esi,0x08
		mov edi,ebx
		sub edi,0x10
		mov eax,ecx
		mov eax,0x06
		mul ecx
		mov ecx,eax
		rep movsd

		pop ecx
		dec ecx
		dec ebp
		jmp .memgroup
	.return:
		mov ecx,[multiboot]
		mov dword [ecx+0x2c],ebp
		pop ebp
		ret

stackifypg:
	;ebx=entry location esi=pages edi=stack location
	mov ecx,dword [multiboot]
	mov ebx,ecx
	mov ecx,dword [ecx+0x2c]
	mov ebx,dword [ebx+0x30]
	mov eax,0x18
	mul ecx
	add ebx,eax
	sub ebx,0x08
	xor esi,esi
	mov edi,_kernel_
	add edi,0x012
	.stackify:
		jecxz .return
		cmp dword [ebx-0x10],0x100000
		jl short .minus
		cmp dword [ebx],0x01
		jz short .stackage
		.minus:
		sub ebx,0x18
		dec ecx
		jmp short .stackify

		.stackage:
		mov eax,dword [ebx-0x08]
		shr eax,0x0c
		add esi,eax
		push ecx
		mov ecx,eax
		mov eax,dword [ebx-0x10]
		add eax,dword [ebx-0x08]
		sub eax,edx
		jecxz .popecx
		.checkdone:
		cmp eax,0x100000
		jz short .done
		sub eax,0x1000
		mov dword [edi],eax
		add edi,0x04
		dec ecx
		jmp short .checkdone
		.done:
		pop ecx
		sub ebx,0x18
		dec ecx
		jmp .stackify

		.popecx:
		pop ecx
	.return:
		mov dword [_kernel_],esi ;total pages
		mov dword [_kernel_+0x04],esi ;remaining pages
		sub edi,0x04
		mov dword [_kernel_+0x08],edi ;actual end of memstack
		
		sub edi,0x100000
		test edi,0xfff
		jz short .noround ;no rounding
		add edi,0x1000
		.noround:
		shr edi,0x0c
		add edi,0x02
		sub dword [_kernel_+0x04],edi
		shl edi,0x02
		sub dword [_kernel_+0x08],edi	
		ret

enpg:
	;call pgalloc ;page directory address
	mov eax,dword [_kernel_+0x08]
	mov eax,dword [eax+0x08]
	mov edx,eax
	
	mov eax,0x00 ;zero the page directory
	mov edi,edx
	mov ecx,0x400
	rep stosd

	;call pgalloc ;page table address
	;push eax
	mov eax,dword [_kernel_+0x08]
	mov eax,dword [eax+0x04]
	push eax

	mov edi,eax ;blank PT #0
	mov eax,0x00
	mov ecx,0x400
	rep stosd

	pop eax
	or eax,0x01
	mov dword [edx],eax ;add PT #0 to PD

	and eax,0xfffff000
	mov ecx,0x1000
	add ecx,eax ;end of PT #0
	xor esi,esi
	or esi,0x01
	.idmap:
		jecxz .return
		mov dword [eax],esi
		add eax,0x04
		add esi,0x1000
		sub ecx,0x1000
		jmp .idmap
	.return:
		mov dword [edx+0xffc],edx
		or dword [edx+0xffc],0x03
		mov cr3,edx
		mov eax,cr0
		or eax,0x80000000
		mov cr0,eax
		ret

pgmap:
	;eax=int* virtual  ;esi=int* physical  ;mov edi=flags
	;returns -1 if out of memory
	;registers used - eax, ebx, ecx:math, edi
	pushad
	mov ebp,eax ;store the from address for later
	mov ecx,eax
	shr ecx,0x16 ;PDE entry number
	shl ecx,0x02 ;PDE offset
	mov eax,dword [0xfffff000+ecx] ;PD entry for 'from' table
	bt eax,0x00 ;present?
	jc short .maptable

	;put into page directory
	cmp dword [_kernel_+0x04],0x00 ;call pgalloc
	jz .error
	mov eax,dword [_kernel_+0x08]
	mov eax,dword [eax]
	sub dword [_kernel_+0x08],0x04
	dec dword [_kernel_+0x04] ;end pgalloc
	or eax,edi
	mov [0xfffff000+ecx],eax ;map a new PT
	mov edx,edi
	mov edi,ecx
	shr edi,0x02 ;PDE entry number
	shl edi,0x0c ;page table offset
	add edi,0xffc00000
	mov ecx,0x400
	mov eax,0x00
	stosd
	mov edi,edx

	.maptable:
	;put into page table
	mov eax,ebp
	mov ecx,eax
	shr ecx,0x16
	shl ecx,0x0c ;page table offset in top 4 MiB
	shr eax,0x0c
	and eax,0x3ff
	shl eax,0x02
	or esi,edi
	mov dword [0xffc00000+ecx+eax],esi
	invlpg [esi]

	.return:
		popad
		ret

	.error:
		popad
		mov eax,0xffffffff
		ret

;make (s)brk ;void* brk(void* addr) void* sbrk(void* increment) *COMPLETE*
;make malloc and free (along with realloc)
;bring everything together
;start on multitasking

brk:
	;void* brk(void *addr,int flags) ;moves program break to addr
	;wrapper adds flags
	;returns the new break on success, -1 on failure
	push ebx
	push ecx
	push edx
	push esi
	push edi
	
	mov eax,0xffc00000
	xor esi,esi
	cmp dword [0xfffffff8],0x00
	jz .startbrk
	.brktest:
	cmp dword [esi-0x08],0x00
	jz .brkprep
	cmp dword [esi-0x0c],0x00
	jz .brkprep
	sub esi,0x04
	sub eax,0x400000
	jmp short .brktest

	.brkprep:
	shr esi,0x02 ;PT number
	shl esi,0x0c ;PT offset
	.brkpreploop:
	cmp dword [esi-0x1004],0x00
	jz .startbrk
	sub eax,0x1000
	sub esi,0x04
	inc ecx
	cmp ecx,0x400
	jz .startbrk
	jmp short .brkpreploop

	;eax contains the current break
	.startbrk:
	mov ecx,dword [esp+0x1c] ;new break
	sub ecx,0xffc00000
	add ecx,eax ;adjustment in bytes
	cmp ecx,0x00
	jg .alloc
	jl .dealloc
	jz .return

	.alloc:
		test ecx,0xfff
		jz .brknoround
		add ecx,0x1000
		.brknoround:
		shr ecx,0x0c

		;start allocating block
		cmp dword [_kernel_+0x04],ecx
		jb .error

		cli
		sub dword [_kernel_+0x04],ecx
		mov edx,dword [_kernel_+0x08]
		shl ecx,0x02
		sub dword [_kernel_+0x08],ecx
		shr ecx,0x02
		mov edi,dword [esp+0x18]

		.brkalloc:
		jecxz .return
		mov esi,dword [edx]
		sub eax,0x1000
		call pgmap
		sub edx,0x04
		dec ecx
		jmp short .brkalloc
		;end allocating block

	.dealloc:
		neg ecx
		shr ecx,0x0c ;pages to dealloc

		;start deallocating block
		cli
		mov edx,dword [_kernel_+0x08]
		add edx,0x04
		add dword [_kernel_+0x04],ecx
		shl ecx,0x02
		add dword [_kernel_+0x08],ecx
		shr ecx,0x02

		mov ebx,ecx
		.brkfree:
		jecxz .brkdealloccont
		mov dword [edx],eax
		add edx,0x04
		dec ecx
		invlpg [eax]
		add eax,0x1000
		jmp short .brkfree

		.brkdealloccont:
		mov ecx,ebx
		mov edi,ecx
		shl edi,0x0c
		sub edi,0xffc00000
		neg edi
		mov ebx,eax
		mov eax,ecx

		shr edi,0x16 ;page table number
		inc edi
		shl edi,0x0c ;page table offset
		add edi,0xffc00000 ;page table offset high
		shl eax,0x02
		sub edi,eax
		xor eax,eax
		rep stosd

		mov eax,ebx

	.return:
		sti
		pop edi
		pop esi
		pop edx
		pop ecx
		pop ebx
		ret 0x08

	.error:
		mov eax,0xffffffff
		pop edi
		pop esi
		pop edx
		pop ecx
		pop ebx
		ret 0x08

sbrk:
	;void* sbrk(void* increment,int flags) ;moves break by increment bytes
	;wrapper adds flags
	;returns the new break on success, -1 on failure
	push ebx
	push ecx
	push edx
	push esi
	push edi
	
	mov eax,0xffc00000
	xor esi,esi
	cmp dword [0xfffffff8],0x00
	jz .startsbrk
	.sbrktest:
	cmp dword [esi-0x08],0x00
	jz .sbrkprep
	cmp dword [esi-0x0c],0x00
	jz .sbrkprep
	sub esi,0x04
	sub eax,0x400000
	jmp short .sbrktest

	.sbrkprep:
	shr esi,0x02 ;PT number
	shl esi,0x0c ;PT offset
	.sbrkpreploop:
	cmp dword [esi-0x1004],0x00
	jz .startsbrk
	sub eax,0x1000
	sub esi,0x04
	inc ecx
	cmp ecx,0x400
	jz .startsbrk
	jmp short .sbrkpreploop

	;eax contains the current break
	.startsbrk:
	mov ecx,dword [esp+0x1c] ;adjustment
	cmp ecx,0x00
	jg .alloc
	jl .dealloc
	jz .return

	.alloc:
		test ecx,0xfff
		jz .sbrknoround
		add ecx,0x1000
		.sbrknoround:
		shr ecx,0x0c

		;start allocating block
		cmp dword [_kernel_+0x04],ecx
		jb .error

		cli
		sub dword [_kernel_+0x04],ecx
		mov edx,dword [_kernel_+0x08]
		shl ecx,0x02
		sub dword [_kernel_+0x08],ecx
		shr ecx,0x02
		mov edi,dword [esp+0x18]

		.sbrkalloc:
		jecxz .return
		mov esi,dword [edx]
		sub eax,0x1000
		call pgmap
		sub edx,0x04
		dec ecx
		jmp short .sbrkalloc
		;end allocating block

	.dealloc:
		neg ecx
		shr ecx,0x0c ;pages to dealloc

		;start deallocating block
		cli
		mov edx,dword [_kernel_+0x08]
		add edx,0x04
		add dword [_kernel_+0x04],ecx
		shl ecx,0x02
		add dword [_kernel_+0x08],ecx
		shr ecx,0x02

		mov ebx,ecx
		.sbrkfree:
		jecxz .sbrkcont
		mov dword [edx],eax
		add edx,0x04
		dec ecx
		invlpg [eax]
		add eax,0x1000
		jmp short .sbrkfree
		.sbrkcont:
		mov ecx,ebx
		mov edi,ecx
		shl edi,0x0c
		sub edi,0xffc00000
		neg edi
		mov ebx,eax
		mov eax,ecx

		shr edi,0x16 ;page table number
		inc edi
		shl edi,0x0c ;page table offset
		add edi,0xffc00000 ;page table offset high
		shl eax,0x02
		sub edi,eax
		xor eax,eax
		rep stosd

		mov eax,ebx

	.return:
		sti
		pop edi
		pop esi
		pop edx
		pop ecx
		pop ebx
		ret 0x08

	.error:
		mov eax,0xffffffff
		pop edi
		pop esi
		pop edx
		pop ecx
		pop ebx
		ret 0x08
		
malloc:
	push ebx
	push ecx
	push edx
	push esi
	push edi
	
	mov edx,dword [esp+0x18] ;bytes to alloc
	test edx,edx
	jz .error
	test edx,0x03
	jz .mallocnoround
	add edx,0x04 ;round to nearest dword
	.mallocnoround:
	shr edx,0x02
	shl edx,0x01 ;dword rounded bytes to alloc >> 1
	cmp edx,0x7f0 ;needs large allocation? TODO: large alloc
	ja .error
	
	push 0x00
	push 0x00
	call sbrk ;sbrk() - returns current break
	mov ecx,0xffc00000
	cmp eax,0xffc00000 ;if the current break is the top of address space
	jne .ssearch ;make the first superblock

	;SETUP RUN ONCE
	push 0x1000 ;one page
	push 0x01 ;flags
	call sbrk ;returns a new page
	cmp eax,0xffffffff
	je .error
	mov dword [0xffbffffc],0x00 ;location of previous superblock header
	mov dword [0xffbffff8],0x00 ;location of next sblock header
	mov dword [0xffbffff4],0xfe0 ;bytes remaining
	mov dword [0xffbffff0],0xfe0 ;average free block size
	mov dword [0xffbfffec],0x01 ;number of free blocks/status
	
	mov dword [0xffbfffe8],0xfe0 ;block size
	mov dword [0xffbfffe4],0x06 ;status
	;bit0 used, bit1 bottom block, b2 top block, b3-15 dist from sblock >> 2
	mov dword [0xffbff000],0xffbfffec ;location of block header
	;END SETUP
		
	.ssearch:
		cmp edx,dword [ecx-0x10] ;has the sblock sufficient room?
		ja .scont
		shl edx,0x01 ;actual dword rounded number of bytes to alloc
		mov ebx,ecx
		sub ebx,0x14 ;actual location of block header
		jmp .search
		
		.scont:
		cmp dword [ecx-0x08],0x00 ;is there a next sblock?
		jne .sadv
		push 0x1000
		push 0x01
		call sbrk
		cmp eax,0xffffffff
		je .error
		add eax,0x1000
		mov dword [eax-0x04],ecx ;previous sblock header
		mov dword [eax-0x08],0x00 ;next sblock header
		mov dword [eax-0x0c],0xfe0 ;bytes remaining
		mov dword [eax-0x10],0xfe0 ;average free block size
		mov dword [eax-0x14],0x01 ;number of free blocks/status
		mov dword [ecx-0x08],eax ;link to previous sblock

		mov dword [eax-0x18],0x0fe0 ;block size
		mov dword [eax-0x1c],0x06 ;status
	;status: bit0 used, bit1 bottom block, b2 top block, b3-15 dist from sblock
		mov dword [eax-0x1000],eax ;location of block header
		sub dword [eax-0x1000],0x14
		.sadv:
		mov ecx,dword [ecx-0x08]
		jmp .ssearch
		
	.error:
		mov eax,0xffffffff
		jmp .return
		
	.search:
		bt dword [ebx-0x08],0x00 ;used
		jc .adv
		cmp edx,dword [ebx-0x04] ;large enough?
		ja .adv
		
		;at this point it should have found a free block
		mov esi,dword [ebx-0x04] ;original size
		push edx
		push esi
		sub esi,edx ;size minus allocation
		mov dword [ebx-0x04],edx ;block size
		sub dword [ecx-0x0c],edx ;update remaining bytes in sblock
		or dword [ebx-0x08],0x01 ;used
		mov eax,ebx ;return the block
		sub eax,edx ;minus the allocation space
		sub eax,0x08 ;minus the header
		
		mov dword [eax-0x04],eax
		add dword [eax-0x04],edx
		add dword [eax-0x04],0x08 ;store location of block header
		
		mov edi,ecx ;sblock location
		sub edi,ebx ;distance from sblock to block reference
		shl edi,0x01
		and dword [ebx-0x08],0x07
		or dword [ebx-0x08],edi
		
		cmp dword [ecx-0x0c],0x00 ;last byte in sblock used?
		jne .cont
		mov dword [ecx-0x10],0x00 ;if so, set average free block size to 0
		mov dword [ecx-0x14],0x00 ;and number of free blocks
		jmp .done
		
		.cont:		
		cmp esi,0x0c
		ja .split
		
		mov edi,dword [ecx-0x14]
		cmp edi,0x01
		je .straightsub
		push eax
		mov eax,dword [ecx-0x10]
		mul edi
		sub eax,edx
		dec edi
		div edi
		mov dword [ecx-0x14],edi
		mov dword [ecx-0x10],eax
		pop eax
		jmp short .done
		
		.straightsub:
			sub dword [ecx-0x10],edx
		
		.done:
		pop esi
		pop edx
		jmp .return
		
		.adv:
			bt dword [ebx-0x08],0x01 ;bottom block?
			jc .scont
			mov eax,dword [ebx-0x04]
			sub ebx,eax
			sub ebx,0x0c
			jmp .search
		
		.split:
			pop edx ;original block size
			mov edi,ebx ;top 
			sub edi,edx ;minus size
			sub edi,0x0c ;minus headers
			mov dword [edi],eax ;location of block header
			sub esi,0x0c ;header size
			mov dword [eax-0x08],esi ;block size
			mov dword [eax-0x0c],0x00 ;status
			
			btc dword [ebx-0x08],0x01 ;bottom block?
			jnc .average
			or dword [eax-0x0c],0x02

			.average:
			pop ebx
			push eax
			mov edi,dword [ecx-0x14]
			mov eax,dword [ecx-0x10]
			mul edi
			sub eax,ebx
			sub eax,0x08
			div edi
			mov dword [ecx-0x14],edi
			mov dword [ecx-0x10],eax
			pop eax
	
	.return:
		pop edi
		pop esi
		pop edx
		pop ecx
		pop ebx
		ret 0x04

free:
	;eax: free ebx:block header ecx:sblock edx:size? esi: block status
	push ebx
	push ecx
	push edx
	push esi
	push edi
	
	mov ebx,dword [esp+0x18] ;pointer to memory to free
	cmp ebx,0xffffffff ;null pointer?
	je .return	
	
	mov ebx,dword [ebx-0x04] ;location of header of memory to free
	
	mov ecx,dword [ebx-0x08] ;status word
	and ecx,0xfffffff8
	shr ecx,0x01
	add ecx,ebx ;sblock header location
	
	and word [ebx-0x08],0xfffe ;set block to free
	
	movzx esi,word [ebx-0x08] ;status
	and esi,0x07
	
	;time to combine free blocks
	mov edx,dword [ecx-0x14]
	mov eax,dword [ecx-0x10]
	mul edx
	mov edx,eax
	jmp $
	;sub eax,ebx
	;div edi
	;mov dword [ecx-0x14],edi
	;mov dword [ecx-0x10],eax
	;pop eax
	
	bt si,0x01 ;bottom block?
	jc .checktop ;if so, try to combine the previous block
	mov eax,dword [esp+0x18] ;location of next block header+4
	bt word [eax-0x08],0x00 ;next block free?
	jc .checktop ;if not, check the previous block
	
	jmp $
	
	
	.checktop:
	
	mov edi,0xdeadbeef
	jmp $
		
	
	.return:
		mov eax,0xffffffff
		pop edi
		pop esi
		pop edx
		pop ecx
		pop ebx
		ret 0x04
