	.file	"main.cpp"
# GNU C++17 (Ubuntu 11.2.0-19ubuntu1) version 11.2.0 (x86_64-linux-gnu)
#	compiled by GNU C version 11.2.0, GMP version 6.2.1, MPFR version 4.1.0, MPC version 1.2.1, isl version isl-0.24-GMP

# GGC heuristics: --param ggc-min-expand=100 --param ggc-min-heapsize=131072
# options passed: -mtune=generic -march=x86-64 -fasynchronous-unwind-tables -fstack-protector-strong -fstack-clash-protection -fcf-protection
	.text
	.section	.rodata
.LC0:
	.string	"Terminating with error \n"
	.text
	.globl	main
	.type	main, @function
main:
.LFB0:
	.cfi_startproc
	endbr64	
	pushq	%rbp	#
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp	#,
	.cfi_def_cfa_register 6
	subq	$32, %rsp	#,
	movl	%edi, -20(%rbp)	# argc, argc
	movq	%rsi, -32(%rbp)	# argv, argv
# src/main.cpp:11:     int err = my_shell(argc, argv);
	movq	-32(%rbp), %rdx	# argv, tmp86
	movl	-20(%rbp), %eax	# argc, tmp87
	movq	%rdx, %rsi	# tmp86,
	movl	%eax, %edi	# tmp87,
	call	_Z8my_shelliPPKc@PLT	#
	movl	%eax, -4(%rbp)	# _7, err
# src/main.cpp:12:     if (err != 0)
	cmpl	$0, -4(%rbp)	#, err
	je	.L2	#,
# src/main.cpp:14:         fprintf(stderr, "Terminating with error \n");
	movq	stderr(%rip), %rax	# stderr, stderr.0_1
	movq	%rax, %rcx	# stderr.0_1,
	movl	$24, %edx	#,
	movl	$1, %esi	#,
	leaq	.LC0(%rip), %rax	#, tmp88
	movq	%rax, %rdi	# tmp88,
	call	fwrite@PLT	#
.L2:
# src/main.cpp:17:     return err;
	movl	-4(%rbp), %eax	# err, _10
# src/main.cpp:18: }
	leave	
	.cfi_def_cfa 7, 8
	ret	
	.cfi_endproc
.LFE0:
	.size	main, .-main
	.ident	"GCC: (Ubuntu 11.2.0-19ubuntu1) 11.2.0"
	.section	.note.GNU-stack,"",@progbits
	.section	.note.gnu.property,"a"
	.align 8
	.long	1f - 0f
	.long	4f - 1f
	.long	5
0:
	.string	"GNU"
1:
	.align 8
	.long	0xc0000002
	.long	3f - 2f
2:
	.long	0x3
3:
	.align 8
4:
