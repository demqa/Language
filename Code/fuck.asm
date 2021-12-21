push 6
pop [bx+0]

CALL FUCK:
push cx
out
pop dx
hlt

FUCK:

pop  ax
push ax

push [bx+0]
push 1
JBE 1:

push [bx+0]

push [bx+0]
push 1
sub

pop [bx+1]

push bx
push 1
add
pop bx

CALL FUCK:

push bx
push 1
sub
pop bx

push cx
mul

pop cx
ret

1:
push 1
pop cx
ret
