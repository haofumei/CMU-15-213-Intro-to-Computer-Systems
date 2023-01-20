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

```bash
Dump of assembler code for function phase_4:
=> 0x000000000040100c <+0>:	sub    $0x18,%rsp
   0x0000000000401010 <+4>:	lea    0xc(%rsp),%rcx
   0x0000000000401015 <+9>:	lea    0x8(%rsp),%rdx
   0x000000000040101a <+14>:	mov    $0x4025cf,%esi # ($esi) = %d %d
   0x000000000040101f <+19>:	mov    $0x0,%eax
   0x0000000000401024 <+24>:	callq  0x400bf0 <__isoc99_sscanf@plt>
   0x0000000000401029 <+29>:	cmp    $0x2,%eax # Compare input size with 2
   0x000000000040102c <+32>:	jne    0x401035 <phase_4+41> # Jump to explode if input size != 2
   0x000000000040102e <+34>:	cmpl   $0xe,0x8(%rsp) # Compare input1 with 14
   0x0000000000401033 <+39>:	jbe    0x40103a <phase_4+46> # Jump if 0 <= input1 <= 14
   0x0000000000401035 <+41>:	callq  0x40143a <explode_bomb>
   0x000000000040103a <+46>:	mov    $0xe,%edx 
   0x000000000040103f <+51>:	mov    $0x0,%esi
   0x0000000000401044 <+56>:	mov    0x8(%rsp),%edi
   0x0000000000401048 <+60>:	callq  0x400fce <func4>
   0x000000000040104d <+65>:	test   %eax,%eax
   0x000000000040104f <+67>:	jne    0x401058 <phase_4+76> # Jump to explode if eax != 0
   0x0000000000401051 <+69>:	cmpl   $0x0,0xc(%rsp) # Compare input2 with 0
   0x0000000000401056 <+74>:	je     0x40105d <phase_4+81> # Jump if input2 == 0
   0x0000000000401058 <+76>:	callq  0x40143a <explode_bomb>
   0x000000000040105d <+81>:	add    $0x18,%rsp
   0x0000000000401061 <+85>:	retq   
End of assembler dump.
```

Phase_4 is similar with phase_3 at the beginning, but something need to be noticed:

1. Secret should be two decimals (line 5, 8), and the 0 <= input1 <= 14 (line 10, 11).
2. Before calling  func4 (line 16), edx = 14, esi = 0, edi = input1.
3. func4 should return 0, or explode. (line 17, 18)
4. input2 should be 0 (line 19, 20)

```bash
Dump of assembler code for function func4:
=> 0x0000000000400fce <+0>:	sub    $0x8,%rsp
   0x0000000000400fd2 <+4>:	mov    %edx,%eax # eax = 14
   0x0000000000400fd4 <+6>:	sub    %esi,%eax # eax = eax - esi = 14 - 0 = 14
   0x0000000000400fd6 <+8>:	mov    %eax,%ecx # ecx = eax = 14
   0x0000000000400fd8 <+10>:	shr    $0x1f,%ecx # ecx = ecx >> 31, get sign bit of ecx
   0x0000000000400fdb <+13>:	add    %ecx,%eax # eax = eax + ecx = 14 + 0 = 14
   0x0000000000400fdd <+15>:	sar    %eax # eax = eax >> 1 = 7
   0x0000000000400fdf <+17>:	lea    (%rax,%rsi,1),%ecx # ecx = rax + (rsi * 1) = 7 + 0 * 1 = 7
   0x0000000000400fe2 <+20>:	cmp    %edi,%ecx # Compare input1 with ecx(7)
   0x0000000000400fe4 <+22>:	jle    0x400ff2 <func4+36> # Jump if 7 <= input1
   0x0000000000400fe6 <+24>:	lea    -0x1(%rcx),%edx # edx = rcx - 1 = 6
   0x0000000000400fe9 <+27>:	callq  0x400fce <func4> # another loop
   0x0000000000400fee <+32>:	add    %eax,%eax
   0x0000000000400ff0 <+34>:	jmp    0x401007 <func4+57>
   0x0000000000400ff2 <+36>:	mov    $0x0,%eax # eax = 0
   0x0000000000400ff7 <+41>:	cmp    %edi,%ecx # Compare ecx with input1
   0x0000000000400ff9 <+43>:	jge    0x401007 <func4+57> # Jump if ecx >= input1
   0x0000000000400ffb <+45>:	lea    0x1(%rcx),%esi # esi = rcx + 1
   0x0000000000400ffe <+48>:	callq  0x400fce <func4>
   0x0000000000401003 <+53>:	lea    0x1(%rax,%rax,1),%eax
   0x0000000000401007 <+57>:	add    $0x8,%rsp
   0x000000000040100b <+61>:	retq   
End of assembler dump.
```

we can perform some calculation first:

1. eax = edx
2. eax = ecx = eax - esi
3. sign = ecx >> 31
4. eax = sign + eax
5. eax  = eax >> 1
6. ecx = rax + (rsi * 1)

Combine above 1 to 5, we have:

eax = (edx - esi + (edx - esi) >> 31) >> 1

ecx = eax + (esi * 1)

So we can convert assembly code into C code below:

```c
int func4(int edx, int edi, int esi) {
    eax = (edx - esi + (edx - esi) >> 31) >> 1;
    ecx = eax + (esi * 1);
    if (ecx > edi) {
        edx = ecx - 1;
	return 2 * func4(edx, edi, esi);
    } else {
	eax = 0;
	if (ecx < edi) {
	    esi = ecx + 1;
	    return eax + eax + 1 + func4(edx, edi, esi); 
	} else {
	    return 0;
	}
    }
}
```

if input1 <= 7, edx will become 14, 6, 2, 0, so when input1 = edx / 2, which is 7, 3, 1, 0, we will have input1 == ecx during the loop and return 0.

If input1 > 7, we will always go into line 11 return eax + eax + 1 + func4(edx, edi, esi) >= 1;

So the secrets are pairs (7,0), (3,0), (1,0), (0,0).

# Phase_5:

```bash
Dump of assembler code for function phase_5:
=> 0x0000000000401062 <+0>:	push   %rbx
   0x0000000000401063 <+1>:	sub    $0x20,%rsp
   0x0000000000401067 <+5>:	mov    %rdi,%rbx
   0x000000000040106a <+8>:	mov    %fs:0x28,%rax # Canary check
   0x0000000000401073 <+17>:	mov    %rax,0x18(%rsp)
   0x0000000000401078 <+22>:	xor    %eax,%eax # Clear eax
   0x000000000040107a <+24>:	callq  0x40131b <string_length>
   0x000000000040107f <+29>:	cmp    $0x6,%eax # Compare input size with 6
   0x0000000000401082 <+32>:	je     0x4010d2 <phase_5+112> # Jump to safe place if it equals 6
   0x0000000000401084 <+34>:	callq  0x40143a <explode_bomb>
   0x0000000000401089 <+39>:	jmp    0x4010d2 <phase_5+112>
   0x000000000040108b <+41>:	movzbl (%rbx,%rax,1),%ecx # ecx = input[rbx + rax * 1]
   0x000000000040108f <+45>:	mov    %cl,(%rsp) # Put lower 8 bits of ecx into stack(get the first char) 
   0x0000000000401092 <+48>:	mov    (%rsp),%rdx # rdx = input[i]
   0x0000000000401096 <+52>:	and    $0xf,%edx # rdx = last 4 bits of char
   0x0000000000401099 <+55>:	movzbl 0x4024b0(%rdx),%edx # edx = another_array[rdx]
   0x00000000004010a0 <+62>:	mov    %dl,0x10(%rsp,%rax,1) stack[rax+0x10] = edx(lower 8 bits)
   0x00000000004010a4 <+66>:	add    $0x1,%rax # Increse the loop counter
   0x00000000004010a8 <+70>:	cmp    $0x6,%rax # Compare the counter with 6
   0x00000000004010ac <+74>:	jne    0x40108b <phase_5+41> 
   0x00000000004010ae <+76>:	movb   $0x0,0x16(%rsp)
   0x00000000004010b3 <+81>:	mov    $0x40245e,%esi # Target array
   0x00000000004010b8 <+86>:	lea    0x10(%rsp),%rdi # Constructed array
   0x00000000004010bd <+91>:	callq  0x401338 <strings_not_equal>
   0x00000000004010c2 <+96>:	test   %eax,%eax 
   0x00000000004010c4 <+98>:	je     0x4010d9 <phase_5+119>
   0x00000000004010c6 <+100>:	callq  0x40143a <explode_bomb>
   0x00000000004010cb <+105>:	nopl   0x0(%rax,%rax,1)
   0x00000000004010d0 <+110>:	jmp    0x4010d9 <phase_5+119>
   0x00000000004010d2 <+112>:	mov    $0x0,%eax
   0x00000000004010d7 <+117>:	jmp    0x40108b <phase_5+41>
   0x00000000004010d9 <+119>:	mov    0x18(%rsp),%rax
   0x00000000004010de <+124>:	xor    %fs:0x28,%rax
   0x00000000004010e7 <+133>:	je     0x4010ee <phase_5+140>
   0x00000000004010e9 <+135>:	callq  0x400b30 <__stack_chk_fail@plt>
   0x00000000004010ee <+140>:	add    $0x20,%rsp
   0x00000000004010f2 <+144>:	pop    %rbx
   0x00000000004010f3 <+145>:	retq   
End of assembler dump.
```

1. From line 9, we know the input size is 6.
2. And then, we will enter a loop from line 13 to 21. In this loop, it will extract the last 4 bits of every input chars, and use it as index to get the char from an array beginning at the address 0x4024b0. We can x/s 0x4024b0 and get "maduiersnfotvbyl". The chars will be put into another array.
3. The new contructed array will be used to compare with the string store at 0x40245e, which is "flyers". If they are equal, we will be safe.

Overall, it is similar to the C code:

```C
int phase_5(char input[]) {
    char dict[] = "maduiersnfotvbyl";
    char target[] = "flyers";
    char res[7];
    for (int i = 0; i < 6; i++) {
	char in = input[i];
	res[i] = dict[in & 0xf];
    }
    res[6] = '\0';
    if (!string_not_equal(res, target)) return 0;
    return 1;
}
```

So we can build this corresponding table to figure out the secret:

| target | index of dict | last 4 bits |
| ------ | ------------- | ----------- |
| f      | 9             | 0x1001      |
| l      | 15            | 0x1111      |
| y      | 14            | 0x1110      |
| e      | 5             | 0x0101      |
| r      | 6             | 0x0110      |
| s      | 7             | 0x0111      |

We can just select the 6 printable chars from ascii table where ending at this 4 bits to construct the secret, for example, ionefg.

# Phase 6:

```bash
Dump of assembler code for function phase_6:
# Part 1
=> 0x00000000004010f4 <+0>:	push   %r14
   0x00000000004010f6 <+2>:	push   %r13
   0x00000000004010f8 <+4>:	push   %r12
   0x00000000004010fa <+6>:	push   %rbp
   0x00000000004010fb <+7>:	push   %rbx
   0x00000000004010fc <+8>:	sub    $0x50,%rsp
   0x0000000000401100 <+12>:	mov    %rsp,%r13
   0x0000000000401103 <+15>:	mov    %rsp,%rsi
   0x0000000000401106 <+18>:	callq  0x40145c <read_six_numbers>
   0x000000000040110b <+23>:	mov    %rsp,%r14
   0x000000000040110e <+26>:	mov    $0x0,%r12d # r12d = 0
   0x0000000000401114 <+32>:	mov    %r13,%rbp
   0x0000000000401117 <+35>:	mov    0x0(%r13),%eax # eax = num1
   0x000000000040111b <+39>:	sub    $0x1,%eax # eax -= 1
   0x000000000040111e <+42>:	cmp    $0x5,%eax # compare eax with 5
   0x0000000000401121 <+45>:	jbe    0x401128 <phase_6+52> # Jump if 0 <= eax <= 5
   0x0000000000401123 <+47>:	callq  0x40143a <explode_bomb>
# Part 2
   0x0000000000401128 <+52>:	add    $0x1,%r12d # r12d += 1
   0x000000000040112c <+56>:	cmp    $0x6,%r12d
   0x0000000000401130 <+60>:	je     0x401153 <phase_6+95> # Jump if r12d == 0x6
   0x0000000000401132 <+62>:	mov    %r12d,%ebx # ebx = r12d
   0x0000000000401135 <+65>:	movslq %ebx,%rax # eax = ebx
   0x0000000000401138 <+68>:	mov    (%rsp,%rax,4),%eax # eax = nums[rsp+eax*4]
   0x000000000040113b <+71>:	cmp    %eax,0x0(%rbp) # compare eax with num1
   0x000000000040113e <+74>:	jne    0x401145 <phase_6+81> # Jump if not equal
   0x0000000000401140 <+76>:	callq  0x40143a <explode_bomb> # Can be same as num1
   0x0000000000401145 <+81>:	add    $0x1,%ebx # ebx += 1
   0x0000000000401148 <+84>:	cmp    $0x5,%ebx
   0x000000000040114b <+87>:	jle    0x401135 <phase_6+65> # Jump if ebx <= 5
# Part 3
   0x000000000040114d <+89>:	add    $0x4,%r13 # r13 points to num2
   0x0000000000401151 <+93>:	jmp    0x401114 <phase_6+32>
# Part 4
   0x0000000000401153 <+95>:	lea    0x18(%rsp),%rsi # rsi points to end of nums
   0x0000000000401158 <+100>:	mov    %r14,%rax # rax points to begin of nums
   0x000000000040115b <+103>:	mov    $0x7,%ecx
   0x0000000000401160 <+108>:	mov    %ecx,%edx # edx = 0x7
   0x0000000000401162 <+110>:	sub    (%rax),%edx # edx -= num1
   0x0000000000401164 <+112>:	mov    %edx,(%rax) # update the num1
   0x0000000000401166 <+114>:	add    $0x4,%rax # next number
   0x000000000040116a <+118>:	cmp    %rsi,%rax 
   0x000000000040116d <+121>:	jne    0x401160 <phase_6+108> # Loop the nums
# Part 5
   0x000000000040116f <+123>:	mov    $0x0,%esi
   0x0000000000401174 <+128>:	jmp    0x401197 <phase_6+163>
   0x0000000000401176 <+130>:	mov    0x8(%rdx),%rdx # get next node
   0x000000000040117a <+134>:	add    $0x1,%eax 
   0x000000000040117d <+137>:	cmp    %ecx,%eax 
   0x000000000040117f <+139>:	jne    0x401176 <phase_6+130> # Jump if eax != ecx
   0x0000000000401181 <+141>:	jmp    0x401188 <phase_6+148>
   0x0000000000401183 <+143>:	mov    $0x6032d0,%edx # edx = 0x6032d0 head node
   0x0000000000401188 <+148>:	mov    %rdx,0x20(%rsp,%rsi,2) # Put cur node into the end of nums
   0x000000000040118d <+153>:	add    $0x4,%rsi
   0x0000000000401191 <+157>:	cmp    $0x18,%rsi
   0x0000000000401195 <+161>:	je     0x4011ab <phase_6+183>
   0x0000000000401197 <+163>:	mov    (%rsp,%rsi,1),%ecx # ecx = nums[i]
   0x000000000040119a <+166>:	cmp    $0x1,%ecx
   0x000000000040119d <+169>:	jle    0x401183 <phase_6+143> # Jump if ecx <= 1
   0x000000000040119f <+171>:	mov    $0x1,%eax # eax = 1
   0x00000000004011a4 <+176>:	mov    $0x6032d0,%edx # # edx = 0x6032d0 
   0x00000000004011a9 <+181>:	jmp    0x401176 <phase_6+130>
# Part 6 link the nodes
   0x00000000004011ab <+183>:	mov    0x20(%rsp),%rbx # begin of node address
   0x00000000004011b0 <+188>:	lea    0x28(%rsp),%rax # next node address
   0x00000000004011b5 <+193>:	lea    0x50(%rsp),%rsi # end of the node address
   0x00000000004011ba <+198>:	mov    %rbx,%rcx # cur node address
   0x00000000004011bd <+201>:	mov    (%rax),%rdx # rdx = next node
   0x00000000004011c0 <+204>:	mov    %rdx,0x8(%rcx) cur.next = rdx
   0x00000000004011c4 <+208>:	add    $0x8,%rax
   0x00000000004011c8 <+212>:	cmp    %rsi,%rax
   0x00000000004011cb <+215>:	je     0x4011d2 <phase_6+222>
   0x00000000004011cd <+217>:	mov    %rdx,%rcx # cur = cur.next
   0x00000000004011d0 <+220>:	jmp    0x4011bd <phase_6+201>
   0x00000000004011d2 <+222>:	movq   $0x0,0x8(%rdx) # cur.next = null
# Part 7
   0x00000000004011da <+230>:	mov    $0x5,%ebp 
   0x00000000004011df <+235>:	mov    0x8(%rbx),%rax # rax = next node address
   0x00000000004011e3 <+239>:	mov    (%rax),%eax # eax = next node
   0x00000000004011e5 <+241>:	cmp    %eax,(%rbx) # Compare cur.val with next.val
   0x00000000004011e7 <+243>:	jge    0x4011ee <phase_6+250> # Jump if cur >= next
   0x00000000004011e9 <+245>:	callq  0x40143a <explode_bomb>
   0x00000000004011ee <+250>:	mov    0x8(%rbx),%rbx
   0x00000000004011f2 <+254>:	sub    $0x1,%ebp
   0x00000000004011f5 <+257>:	jne    0x4011df <phase_6+235>
   0x00000000004011f7 <+259>:	add    $0x50,%rsp
   0x00000000004011fb <+263>:	pop    %rbx
   0x00000000004011fc <+264>:	pop    %rbp
   0x00000000004011fd <+265>:	pop    %r12
   0x00000000004011ff <+267>:	pop    %r13
   0x0000000000401201 <+269>:	pop    %r14
   0x0000000000401203 <+271>:	retq   
End of assembler dump.
```

Part1:

Secrets are six number and 0 <= num - 1 <= 5, which is 1 <= num <= 6

Part2:

Loop to check the rest of number with num1, if they are equal, explode.

Part3:

Point to the num2, go back to part1 and part2, check if the rest of numbers are equal to the current one. In ohter word, all the numbers should be unique and <= 6.

Part4:

Loop all the numbers and update them with nums[i] = 7 - nums[i].

Part5:

A loop to create a node array following by offset nums[i]

| nums[i] | 0x8(%rdx),%rdx(address of node) |
| ------- | ------------------------------- |
| 1       | 0x6032d0                        |
| 2       | 0x6032e0                        |
| 3       | 0x6032f0                        |
| 4       | 0x603300                        |
| 5       | 0x603310                        |
| 6       | 0x603320                        |

After the loop, you can just x/24x $rsp+0x20, you will see the all nodes.

Part6:

Convert the node array we created on part 5, and link the node by this order. Suppose nums = [1,2,3,4,5,6]

```bash
(gdb) x/24x $rbx
0x6032d0 <node1>:	0x0000014c	0x00000001	0x006032e0	0x00000000
0x6032e0 <node2>:	0x000000a8	0x00000002	0x006032f0	0x00000000
0x6032f0 <node3>:	0x0000039c	0x00000003	0x00603300	0x00000000
0x603300 <node4>:	0x000002b3	0x00000004	0x00603310	0x00000000
0x603310 <node5>:	0x000001dd	0x00000005	0x00603320	0x00000000
0x603320 <node6>:	0x000001bb	0x00000006	0x00000000	0x00000000
```

Part7:

Check if it is an nondecreasing node list, if not, explode.

Above all, we first enter 6 nums from 1 to 6, and get another 6 numbers by 7-nums, let's name this new numbers as orders. And then we will arrange the nodes by the this orders such that every node val is larger than the next one. Therefore, we can reverse this process:

|        | node1 | node2 | node3 | node4 | node5 | node6 |
| ------ | ----- | ----- | ----- | ----- | ----- | ----- |
| val    | 0x39c | 0x2b3 | 0x1dd | 0x1bb | 0x14c | 0x0a8 |
| index  | 3     | 4     | 5     | 6     | 1     | 2     |
| secret | 4     | 3     | 2     | 1     | 6     | 5     |
