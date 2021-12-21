PUSH 6
POP [bx+0]

CALL FUCK:
OUT
POP dx
HLT



FUCK:
POP ax
PUSH [bx+0]
PUSH 1
JBE 1:

PUSH ax

PUSH [bx+0] // push N

PUSH [bx+0] // push N
PUSH 1      // push 1
SUB         // N - 1

POP [bx+1]  // next parameter is N - 1

CALL FUCK:

MUL

POP cx      // pop Fuck(n - 1) * n
POP ax      // pop 
PUSH cx     // push Fuck(n - 1) * n
PUSH ax     // 
RET

1:
PUSH 1
PUSH ax
RET
