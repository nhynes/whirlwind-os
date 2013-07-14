mmlist:
	pushad
	mov ecx,[multiboot]
	mov ecx,[ecx+0x2c]
	xor edi,edi
	.list:
		cmp edi,ecx
		jz .return
		mov eax,edi
		mov edi,0x06
		mul edi
		mov edi,eax
		shl edi,2
		mov eax,dword [edi+0x558]
		call int2str
		mov eax,0x02
		call kprint.space
		mov eax,dword [edi+0x55c]
		call int2str
		mov eax,0x02
		call kprint.space
		mov eax,dword [edi+0x560]
		call int2str
		mov eax,0x02
		call kprint.space
		mov eax,dword [edi+0x564]
		call int2str
		mov eax,0x02
		call kprint.space
		mov eax,dword [edi+0x568]
		call int2str
		mov eax,0x02
		call kprint.space
		mov eax,dword [edi+0x56c]
		call int2str
		mov eax,0x0a
		call kprint.space
		shr edi,2
		mov eax,edi
		mov edi,0x06
		div edi
		mov edi,eax
		inc edi
		jmp .list
	.return:
		popad
		ret
