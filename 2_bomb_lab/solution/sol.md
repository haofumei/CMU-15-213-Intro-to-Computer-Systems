# Phase_1:

```bash
Dump of assembler code for function phase_1:
=> 0x0000000000400ee0 <+0>:	sub    $0x8,%rsp # rsp = rsp - 8
   0x0000000000400ee4 <+4>:	mov    $0x402400,%esi # esi = $4203520
   0x0000000000400ee9 <+9>:	callq  0x401338 <strings_not_equal> # return 0 if equal
   0x0000000000400eee <+14>:	test   %eax,%eax 
   0x0000000000400ef0 <+16>:	je     0x400ef7 <phase_1+23> # j if eax == 0
   0x0000000000400ef2 <+18>:	callq  0x40143a <explode_bomb>
   0x0000000000400ef7 <+23>:	add    $0x8,%rsp
   0x0000000000400efb <+27>:	retq   
End of assembler dump.
```

On line 2, %esi was assigned to an adress $4203520, and then pass to the function <strings_not_equal> as the second parameter.

So we can just (gdb) x/s $esi, which will print the string pointed by this address.

So we get ($esi) = "Border relations with Canada have never been better."

# Phase_2:

```bash
Dump of assembler code for function phase_2:
=> 0x0000000000400efc <+0>:	push   %rbp
   0x0000000000400efd <+1>:	push   %rbx
   0x0000000000400efe <+2>:	sub    $0x28,%rsp # reserve 40 bytes
   0x0000000000400f02 <+6>:	mov    %rsp,%rsi # rsi = rsp
   0x0000000000400f05 <+9>:	callq  0x40145c <read_six_numbers> # read six numbers from input
   0x0000000000400f0a <+14>:	cmpl   $0x1,(%rsp) # compare first number with 1
   0x0000000000400f0e <+18>:	je     0x400f30 <phase_2+52> # j if equal
   0x0000000000400f10 <+20>:	callq  0x40143a <explode_bomb> 
   0x0000000000400f15 <+25>:	jmp    0x400f30 <phase_2+52>
   0x0000000000400f17 <+27>:	mov    -0x4(%rbx),%eax # eax = last number
   0x0000000000400f1a <+30>:	add    %eax,%eax # eax = eax * 2
   0x0000000000400f1c <+32>:	cmp    %eax,(%rbx) # compare last number * 2 with next input number
   0x0000000000400f1e <+34>:	je     0x400f25 <phase_2+41> # j if equal
   0x0000000000400f20 <+36>:	callq  0x40143a <explode_bomb>
   0x0000000000400f25 <+41>:	add    $0x4,%rbx # rbx = next number' index
   0x0000000000400f29 <+45>:	cmp    %rbp,%rbx # check if reach the end of six number
   0x0000000000400f2c <+48>:	jne    0x400f17 <phase_2+27>
   0x0000000000400f2e <+50>:	jmp    0x400f3c <phase_2+64>
   0x0000000000400f30 <+52>:	lea    0x4(%rsp),%rbx # rbx = next number's address
   0x0000000000400f35 <+57>:	lea    0x18(%rsp),%rbp # rbp = 24 bytes aways from base(end of six numbers)
   0x0000000000400f3a <+62>:	jmp    0x400f17 <phase_2+27>
   0x0000000000400f3c <+64>:	add    $0x28,%rsp
   0x0000000000400f40 <+68>:	pop    %rbx
   0x0000000000400f41 <+69>:	pop    %rbp
   0x0000000000400f42 <+70>:	retq   
End of assembler dump.
```

1. From line 6 <read_six_numbers>, we can guess the secrets are six numbers.
2. From line 7, we know the first number is 1.
3. From line 11,12,13, we know the second number = first number * 2, and then it goes to next loop.

```c
void phase_2(int inputs[]) {
    int secret = 1;
    for (int i = 0; i < 6; i++) {
	if (inputs[i] != secret) {
	    explode_bomb();
	}
	secret *= 2;
    }
} 
```

We can get similar function from assembly code above, so the secrets are:

1 2 4 8 16 32

# Phase_3:

```bash
Dump of assembler code for function phase_3:
=> 0x0000000000400f43 <+0>:	sub    $0x18,%rsp # Reserve 24 bytes 
   0x0000000000400f47 <+4>:	lea    0xc(%rsp),%rcx 
   0x0000000000400f4c <+9>:	lea    0x8(%rsp),%rdx 
   0x0000000000400f51 <+14>:	mov    $0x4025cf,%esi # esi as second param to sscanf
   0x0000000000400f56 <+19>:	mov    $0x0,%eax # eax = 0
   0x0000000000400f5b <+24>:	callq  0x400bf0 <__isoc99_sscanf@plt>
   0x0000000000400f60 <+29>:	cmp    $0x1,%eax # Check if return value from sscanf with 1 
   0x0000000000400f63 <+32>:	jg     0x400f6a <phase_3+39> # J if input size > 1
   0x0000000000400f65 <+34>:	callq  0x40143a <explode_bomb>
   0x0000000000400f6a <+39>:	cmpl   $0x7,0x8(%rsp) # Compare first input number with 7
   0x0000000000400f6f <+44>:	ja     0x400fad <phase_3+106> # J to explode if > 7 or < 0
   0x0000000000400f71 <+46>:	mov    0x8(%rsp),%eax # eax = first input value
   0x0000000000400f75 <+50>:	jmpq   *0x402470(,%rax,8) # Jump by value at the address (0x402470 + input1 * 8)
   0x0000000000400f7c <+57>:	mov    $0xcf,%eax # case 0
   0x0000000000400f81 <+62>:	jmp    0x400fbe <phase_3+123>
   0x0000000000400f83 <+64>:	mov    $0x2c3,%eax # case 2
   0x0000000000400f88 <+69>:	jmp    0x400fbe <phase_3+123>
   0x0000000000400f8a <+71>:	mov    $0x100,%eax # case 3
   0x0000000000400f8f <+76>:	jmp    0x400fbe <phase_3+123>
   0x0000000000400f91 <+78>:	mov    $0x185,%eax # case 4
   0x0000000000400f96 <+83>:	jmp    0x400fbe <phase_3+123>
   0x0000000000400f98 <+85>:	mov    $0xce,%eax # case 5
   0x0000000000400f9d <+90>:	jmp    0x400fbe <phase_3+123>
   0x0000000000400f9f <+92>:	mov    $0x2aa,%eax # case 6
   0x0000000000400fa4 <+97>:	jmp    0x400fbe <phase_3+123>
   0x0000000000400fa6 <+99>:	mov    $0x147,%eax # case 7
   0x0000000000400fab <+104>:	jmp    0x400fbe <phase_3+123>
   0x0000000000400fad <+106>:	callq  0x40143a <explode_bomb>
   0x0000000000400fb2 <+111>:	mov    $0x0,%eax
   0x0000000000400fb7 <+116>:	jmp    0x400fbe <phase_3+123>
   0x0000000000400fb9 <+118>:	mov    $0x137,%eax # case 1
   0x0000000000400fbe <+123>:	cmp    0xc(%rsp),%eax
   0x0000000000400fc2 <+127>:	je     0x400fc9 <phase_3+134>
   0x0000000000400fc4 <+129>:	callq  0x40143a <explode_bomb>
   0x0000000000400fc9 <+134>:	add    $0x18,%rsp
   0x0000000000400fcd <+138>:	retq   
End of assembler dump.
```

1. From line 5, we can x/s $esi, get the input format %d %d, which are two decimal values.
2. From line 11, 12, we know the first input should be 0<=first<=7.
3. From line 14, we know there might be a switch control here, and the address of index = (0x402470 + first * 8). So we can use print *(address of index) to get all the cases jump address.

| case | address | value(hex) | value(d) |
| ---- | ------- | ---------- | -------- |
| 0    | 400f7c  | 0xcf       | 207      |
| 1    | 400fb9  | 0x137      | 311      |
| 2    | 400f83  | 0x2c3      | 707      |
| 3    | 400f8a  | 0x100      | 256      |
| 4    | 400f91  | 0x185      | 389      |
| 5    | 400f98  | 0xce       | 206      |
| 6    | 400f9f  | 0x2aa      | 682      |
| 7    | 400fa6  | 0x147      | 327      |

Pick either case + value(d) as secret.

# Phase_4:
