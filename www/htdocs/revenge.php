<?php
include 'header.php'
?>

<H1>Revenge Emulator Documentation</H1>
<H1>1. Aims.</H1>
<H4>1) The first planned piece of software is going to be called
&quot;revenge&quot;.</H4>
<H4>2) It will emulate i386 32-bit protected mode for user space
applications.</H4>
<DL>
	<DD>Later Pentium4/AMD etc. will be added.</DD><DD>
	We might even add PPC/Sparc/MIPS support. We will only support 32bit
	Protected mode in user space.</DD><DD>
	We will not support Real or Old mode but someone might want to add
	support for that and we will let them.</DD><DD>
	If an application uses a call gate (ring3 -&gt; other ring call), we
	will prevent it from accessing ring0, and instead fully emulate what
	the call would have achieved.</DD><DD STYLE="margin-bottom: 0.2in">
	Later we might support some ring0 emulations in order to reverse
	engineer device drivers.</DD></DL>
<H4>
3) Different processor threads will be emulated as if running on
their own CPU with options to share memory or not.</H4>
<DL>
	<DD STYLE="margin-bottom: 0.2in">The first version will not support
	threads.</DD></DL>
<H4>
4) Initially it will support a.out executable format, followed by elf
and then followed by other formats that are supported in other
environments apart from Linux</H4>
<H4>5) The first application to be worked on will be a simple &ldquo;Hello
World&rdquo; binary.</H4>
<H4>6) CPU registers will just look like another type of memory, and
will be treated as though they are memory locations for the purposes
of recording and analysis</H4>
<H4>7) The Stack will just look like another type of memory, and will
be treated as though they are memory locations for the purposes of
recording and analysis.</H4>
<DL>
	<DD STYLE="margin-bottom: 0.2in">The SP register marks the top of
	the stack (top as in a pile of books type stack. One puts a book on
	the top, the first to come off will be the one on the top.) Anything
	above SP on the stack represents thin air. We give a particular
	memory location, that is part of the stack, a label. When an item is
	removed from the stack, the memory location's label would be marked
	as expired. When an item is placed back on to the stack and accesses
	the expired memory location, it will be given a new label. This will
	give us nice unique names for local variables so that they do not
	get confused with local variables from other functions. After the
	functions have been analysed, they will be renamed so that the human
	observer can then determine their scope easily. E.g.
	local_variable_1, local_variable_2 etc.</DD></DL>
<H4>
8) As each assembler instruction is executed, it will be recorded.</H4>
<DL>
	<DD STYLE="margin-bottom: 0.2in">Each instruction executed will be
	given an index. This index will start at 1, and increase with each
	instruction executed. If there is a loop, and the same instruction
	is executed again, it will be given a new index number. This index
	will point to a record associated with this instruction.</DD><DD STYLE="margin-bottom: 0.2in">
	If an instruction accessed memory or register, the location and
	value used is stored in the record.</DD><DD STYLE="margin-bottom: 0.2in">
	The record will also store where in memory the instruction was and
	also the function name and instruction number within the function
	that we think the instruction belongs.</DD></DL>
<H4>
9) When a memory location is accessed, it will be recorded.</H4>
<DL>
	<DD STYLE="margin-bottom: 0.2in">All memory locations will be given
	a record. These records will make up a number of tables with one
	table for each continuous slice of memory. The CPU registers and CPU
	flags will be treated as a table of it's own with each flag having
	it's own record (some instructions do not effect all the flags, so
	we will need to keep separate histories for each flag). The record
	will store a history of pointers to all the instructions that have
	accessed that memory location. The pointer will contain the
	instruction index from (6).</DD><DD STYLE="margin-bottom: 0.2in">
	The record will keep a history of how the memory was accessed. I.E.
	As an instruction, as a memory location, if the access is a read or
	write.</DD></DL>
<H4>
10) All calls, jump, ret's will be executed as if they are a
combination of PUSH/POP and JUMP.</H4>
<DL>
	<DD STYLE="margin-bottom: 0.2in">Later, after the program flow has
	been logged, groups of instructions can then be grouped into
	functions. One reason for this is that a &ldquo;ret&rdquo;
	instruction can be turned into a &ldquo;jump&rdquo; just by
	preceding it with some PUSH instructions, so call and ret
	instructions cannot really be used to determine the start and end
	points of a function. 
	</DD></DL>
<H4 STYLE="margin-top: 0in">
11) Function generation.</H4>
<DL>
	<DD STYLE="margin-bottom: 0.2in">The generated functions will be
	based round how registers and memory are accessed. For example. If
	AX is given a value 10, and that register is used for something,
	then later down the program, AX is given a value 20. We can create a
	new function for all the instructions that used AX when it had value
	10. and a separate function for all the instructions that used AX
	when given a value of 20. Also, interleaved instructions can then be
	separated. For example, AX=10, and BX=20, but AX is never used in
	calculations regarding BX, and visa versa, then all the instructions
	using AX can be put in one function, and all the instructions using
	BX can be put in a different function. As a result, function
	generation might not actually match up with call-ret instructions in
	the code. The outputted functions will represent a particular
	operation happening, rather than being flow based. As a result, I
	would expect the resulting functions to be easier for a human to
	understand.</DD><DD STYLE="margin-bottom: 0.2in">
	e.g. Original C source code: -</DD><DD STYLE="margin-bottom: 0.2in">
	main(..) {</DD><DD STYLE="margin-bottom: 0.2in">
	a=10;</DD><DD STYLE="margin-bottom: 0.2in">
	b=30</DD><DD STYLE="margin-bottom: 0.2in">
	printf(&ldquo;value=%d\n&rdquo;,a);</DD><DD STYLE="margin-bottom: 0.2in">
	a=20;</DD><DD STYLE="margin-bottom: 0.2in">
	printf(&ldquo;value=%d\n&rdquo;,a);</DD><DD STYLE="margin-bottom: 0.2in">
	printf(&ldquo;value=%d\n&rdquo;,b);</DD><DD STYLE="margin-bottom: 0.2in">
	}</DD><DD STYLE="margin-bottom: 0.2in">
	Would probably result in the following output after revenge: -</DD><DD STYLE="margin-bottom: 0.2in">
	void print_a_value(int param) {</DD><DD STYLE="margin-bottom: 0.2in">
	printf(&ldquo;value=%d\n&rdquo;,param);</DD><DD STYLE="margin-bottom: 0.2in">
	}</DD><DD STYLE="margin-bottom: 0.2in">
	main(..) {</DD><DD STYLE="margin-bottom: 0.2in">
	print_a_value(10);</DD><DD STYLE="margin-bottom: 0.2in">
	print_a_value(20);</DD><DD STYLE="margin-bottom: 0.2in">
	print_a_value(30);</DD><DD STYLE="margin-bottom: 0.2in">
	}</DD></DL>
<H4>
12) Instruction expansion.</H4>
<DL>
	<DD STYLE="margin-bottom: 0.2in">Take example: -</DD><DD STYLE="margin-bottom: 0.2in">
	MOV SI, 10 &lt;- Instruction number 1.</DD><DD STYLE="margin-bottom: 0.2in">
	MOV AX, [SI] &lt;- Instruction number 2</DD><DD STYLE="margin-bottom: 0.2in">
	At start, Record SI is empty. Record AX is empty.</DD><DD STYLE="margin-bottom: 0.2in">
	After instruction 1, Record SI contains a history entry &ldquo;Write
	with 10 by instruction 1&rdquo;, and instruction 1's record will
	point to the particular history entry of record SI.</DD><DD STYLE="margin-bottom: 0.2in">
	When expanding instruction 2, we look at the Record SI. It will tell
	us to look at instruction 1. We see that SI was written with a
	constant &ldquo;10&rdquo; and did not arise from any other variable.
	If it did arise from any other variable (determined by examining the
	instruction that wrote to it), we would re-curse backwards to that
	record and gain more expanded information, which will eventually
	result with a record that was initiated with a constant value, and
	thus the recursion stops. Instruction 2 could then be expanded to: -</DD><DD STYLE="margin-bottom: 0.2in">
	MOX AX,[10]</DD><DD STYLE="margin-bottom: 0.2in">
	The expansion is only done in order to display it to the user or
	when written to a file, the instruction record only stores the
	original instruction, so that future instructions can be expanded.</DD></DL>
<H4>
13) A C program will be produced, that if compiled, should function
in exactly the same way as the original binary.</H4>
<P STYLE="margin-bottom: 0in">Although it will compile as C, it will
probably be very difficult to understand, as it will have no symbols,
and the functional structure might be wrong.</P>
<H4>14) The next task will be using the records together with the
generated C program and find ways to automate the manipulation of it
so that it is easier to read and understand.</H4>
<H4>15) Recovery of the parameter types passed to a function.</H4>
<H4>16) Recovery of variable types.</H4>
<H4>17) Recovery of returned types.</H4>
<H4>18) Determine which variables are local, global, or
runtime(malloc'ed) created.</H4>
<DL>
	<DD STYLE="margin-bottom: 0.2in">For example. AX=10, if it then used
	for some calculation DX=DX+AX. Then later on AX=20, we can establish
	that for all the instructions between DX=DX+AX and AX=20, AX is not
	required, and we can therefore establish that AX is a local variable
	between the AX=10 and DX=DX+AX instructions.</DD></DL>
<H4>
19) Determine which variables need to be initialised before calling a
function.</H4>
<H4>20) Determine what range of values a variable can take.</H4>
<DL>
	<DD STYLE="margin-bottom: 0.2in">Useful for analysing sizes of
	lookup tables.</DD></DL>
<H4>
21) Determine which variables are modified or created by a function.</H4>
<H4>22) Using the recorded counts of the amount of times a particular
function is called.</H4>
<DL>
	<DD STYLE="margin-bottom: 0.2in">Efforts will be made to understand
	most used functions first.</DD></DL>
<H4>
23) Allow human interaction to add comments, function and variable
names.</H4>
<H4>24) The GUI controlling front end will be a separate application
from the actual emulator, allowing for remote emulator control and
interactivity.</H4>
<H4>25) Every application is likely to depend on external libs. The
emulator will be able to run on the entire program including any
external libs, but logging and analysis can be restricted to a
particular lib the program uses or even just a particular function
inside a lib.</H4>
<H4>26) Some shim code might be needed to buffer access the
application has to outside libs.</H4>
<DL>
	<DD STYLE="margin-bottom: 0.2in">For example, if the application
	being examined uses a lib that does not need to be reverse
	engineered, we might only need to log parameters passed and results
	returned from the lib as we will already know exactly what each
	function does.</DD></DL>
<H4>
27) Be able to save the entire emulated application state, so that
one can come back later and continue. As one will know which memory
locations have been used, and which have not, one can save on the
amount of information needed to be saved in order to save the state.</H4>
<H2>2. Expected problems to overcome.</H2>
<H4>1) The i386 instruction set is huge.</H4>
<DL>
	<DD STYLE="margin-bottom: 0.2in">We will start by only supporting
	the i386 instructions needed for &ldquo;Hello World!&rdquo;.</DD></DL>
<H4>
2) We have no real idea if emulation will actually help us any more
than the current reverse engineering methods out there until we try
it!</H4>
<H4>3) How to emulate calls to external libs with a variable number
of parameters and variable parameter types. E.g. How to catch calls
to &ldquo;printf(...)&rdquo; from the emulated program and translate
them into native system calls.</H4>
<DL>
	<DD STYLE="margin-bottom: 0.2in">Suggestions welcome from anyone!</DD><DD STYLE="margin-bottom: 0.2in">
	My first thought is to take the format string from the first
	parameter, and break it up into more simple printf statements that
	require only a second parameter. E.g. Printf(&ldquo;First value=%d,
	second value=%d\n&rdquo;,value1, value2); would be converted to two
	printf's like printf(&ldquo;First value=%d&rdquo;, value1);
	printf(&ldquo;,second value=%d\n&rdquo;, value2); Thus more simple
	printf's can with a fixed number of parameters can easily be
	emulated.</DD></DL>
<H4 STYLE="margin-top: 0in">
4) The i386 instructions will all use the little-endian memory format
and not need to be memory aligned. How does one create a endian free
and memory aligned representation in C? In case we want to compile
the generated C source code on a big-endian system that also requires
the accesses to memory to be aligned. 
</H4>
<H2>3. Plan of action</H2>
<H4>1) Create a lib that will do the i386 disassembly.</H4>
<DL>
	<DD STYLE="margin-bottom: 0.2in">Disassemble(uint8_t *buffer,
	decoded_instruction_t *result)</DD><DD STYLE="margin-bottom: 0.2in">
	The buffer will be just a pointer to a string of bytes that need to
	be disassembled. The result will contain the disassembled
	instruction together with how many bytes it needed so that the
	calling routine can correctly set the buffer pointer when it needs
	to disassemble the next instruction.</DD><DD STYLE="margin-bottom: 0.2in">
	The decoded_instruction_t structure will probably contain:</DD><DD STYLE="margin-bottom: 0.2in">
	a) one variable for the decoded op code. E.g. ADD, SUB, CALL. 
	</DD><DD STYLE="margin-bottom: 0.2in">
	b) one variable for the first source register/memory expression. (As
	i386 expressions can be quite complicated, we might have to split
	this up a bit.)</DD><DD STYLE="margin-bottom: 0.2in">
	c) one variable for the second source register/memory expression</DD><DD STYLE="margin-bottom: 0.2in">
	d) one variable for the destination register/memory expression.</DD><DD STYLE="margin-bottom: 0.2in">
	e) one variable detailing which flags have to be set in order for
	this instruction to actually do anything.</DD><DD STYLE="margin-bottom: 0.2in">
	f) one variable detailing which flags should get effected by this
	instruction.</DD><DD STYLE="margin-bottom: 0.2in">
	g) one variable detail any special variant action needing to be
	performed.</DD><DD STYLE="margin-bottom: 0.2in">
	These 7 points should be able to handle most CPU's instructions out
	there. Obviously, for the i386, (b) and (d) are always the same
	thing, but on ARM processors they can be different.</DD><DD STYLE="margin-bottom: 0.2in">
	The general idea here, is to make the disassembly happen in such a
	way, that the ADD instruction on an i386 cpu will be disassembled to
	the same ADD instruction on the ARM cpu, thus letting the emulator
	understand different CPUs without too much trouble.</DD><DD STYLE="margin-bottom: 0.2in">
	To simplify CPU emulation, it might be beneficial to disassemble a
	single CPU instruction into a list of instructions for the emulator.
	Similar to taking a CISC instruction and turning it into a RISC
	instruction.</DD><DD STYLE="margin-bottom: 0.2in">
	E.g. MOV AX, [SI*2+DX+10] can be written as 5 simpler instructions:
	-</DD><DD STYLE="margin-bottom: 0.2in">
	MOV tmp, SI; MUL tmp, 2; ADD tmp, DX; ADD tmp, 10; MOV AX,[tmp];</DD><DD STYLE="margin-bottom: 0.2in">
	This would greatly simplify the emulator at the expense of a more
	complicated disassembler, but disassemblers are easier to write than
	emulators so this is probably a good idea.</DD><DD STYLE="margin-bottom: 0.2in">
	Futures: Handle different CPUs.</DD></DL>
<H4>
2) Create a lib that will handle all the database details. 
</H4>
<DL>
	<DD STYLE="margin-bottom: 0.2in">This will handle storage of the
	memory/instruction records and history tables.</DD></DL>
<H4>
3) Create a lib that will handle all the cpu emulation. 
</H4>
<DL>
	<DD STYLE="margin-bottom: 0.2in">This will take the output from (1)
	and create all the database entries using (2) and actually process
	the CPU instructions.</DD></DL>
<H4>
4) Create a lib that will handle the loading of the program.</H4>
<DL>
	<DD STYLE="margin-bottom: 0.2in">This will handle reading of the
	executable file and loading of it's contents into the correct
	database tables. Initially handling linux elf executables, then
	follow that with a.out and then others, for example win32 PE.</DD></DL>
<H4>
5) Create a lib that will handle platform specific actions. 
</H4>
<DL>
	<DD STYLE="margin-bottom: 0.2in">This will handle things like
	emulation of the printf function, so that the application being
	emulated can actually send out to the screen and receive input from
	the user.</DD></DL>
<H4>
6) Create a GUI that will probably talk to the emulator over a TCP/IP
connection.</H4>
<DL>
	<DD>This will provide interaction for the developer who is
	controlling revenge.</DD><DD STYLE="margin-bottom: 0.2in">
	<BR><BR>
	</DD><DD STYLE="margin-bottom: 0.2in">
	<BR><BR>
	</DD><DD STYLE="margin-bottom: 0.2in">
	<FONT FACE="arial"><FONT SIZE=3>Copyright (c) 2003 James
	Courtier-Dutton. All Rights Reserved.</FONT></FONT></DD></DL>
</BODY>
</HTML>
