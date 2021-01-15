	.text				#Declare we are in text region
.globl tas				#Declare tas to be a global symbol
	.type	tas,@function		#Declare tas to be a function-type
tas:					#Label marking start of tas function
	pushl	%ebp			#Save base pointer on stack
	movl	%esp, %ebp		#Establish new base pointer
	movl	8(%ebp), %edx		#Fetch arg#1 to edx
	movl	$1, %eax		#Prepare a constant 1
	lock;xchgb	%al,(%edx)	#The actual atomic instruction
	movsbl	%al,%eax		#Sign-extend char to an int
	popl	%ebp			#Restore base pointer
	ret				#Return (retval in eax)
.Lfe1:
	.size	tas,.Lfe1-tas		#Declare the size of function tas
