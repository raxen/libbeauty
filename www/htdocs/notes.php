<?php
include 'header.php'
?>

<table width="90%" class="centered">
<H2>Revenge Notes
</H2>
<p>Revenge CPU Emulator. &lt;- What should it do.
<p>
Before calling a function, the only ia32 register that we initialise will be the SP register.
During execution, registers like SP, BP etc. will be changed to S1,S2,S3... so we can track registers that act on the STACK.
Registers eAX, eBX etc. will be changed to R1, R2 etc. (FIXME: What to do with 32bit register access, and 16 or 8 bit register accesses.)
<p>
We store the inital SP value in S0, and S1.
The calling programs IP is at position S0 on the stack, so any values from below S0 are local variables, and any values from above S0 are function parameters. Each time a CALL is made. i.e. the IP is put onto the stack, S0 and the current function's entry point are stored with it as metadata, and a new S0 is initiated. This will therefore create a new S0 for each stack frame. This will sort out any recursive function call problems and enable correct identification of parameters and local variables.
<p>
Example program in RTL format (As output by revenge). The "Instruction X:" are output by revenge, the extra "S1 = S1 - 4" etc. are manually added for now to help explain things, but they could very easily be automatically added by revenge CPU.<br>
r0x14 is SP, r0x18 is BP.<br>
Instruction 0:SUB  di0x4, dr0x14<br>

S1 = S1 - 4<br>
Instruction 1:MOV  dr0x18, ds[r0x14]<br>
[S1] = S2  (BP or r0x18 is renamed to S2)<br>
Instruction 0:MOVf dr0x14, dr0x18<br>
S3 = S1 (BP has been initialised with a new value, so BP is now S3)<br>
Instruction 0:MOV  dr0x18, dr0x28<br>
S4 = S3 (r0x28 is a TMP register, implemented to make generation of RTL code easier)<br>
Instruction 1:ADD  bi0x8, dr0x28<br>
S4 = S4 + 8<br>

Instruction 2:ADDf di0x1, ds[r0x28]<br>
[S4] = [S4] + 1<br>
Instruction 0:MOV  dr0x18, dr0x28<br>
S5 = S3<br>
Instruction 1:ADD  bi0x8, dr0x28<br>
S5 = S5 + 8<br>
Instruction 2:SUBf di0x1, ds[r0x28]<br>
[S5] = [S5] - 1<br>
Instruction 0:MOV  dr0x18, dr0x28<br>

S6 = S3<br>
Instruction 1:ADD  bi0x8, dr0x28<br>
S6 = S6 + 8<br>
Instruction 2:MOVf ds[r0x28], dr0x4<br>
R1 = [S6]<br>
Instruction 0:MOV  ds[r0x14], dr0x18<br>
S7 = [S1]<br>
Instruction 1:ADD  di0x4, dr0x14<br>
S1 = S1 + 4<br>

Instruction 0:MOV  ds[r0x14], dr0x28<br>
I1 = [S1]  (r0x28 is now I1, because the IP from the calling process was stored at this position on the stack.)<br>
Instruction 1:ADD  di0x4, dr0x14<br>
S1 = S1 + 4<br>
Instruction 2:MOV  dr0x28, dr0x24<br>
IP = I8  (A JUMP instruction, but as I1 came from the stack, it is a RET)<br>
<p>
Now extract the new lines:<br>
S1 = S1 - 4<br>

[S1] = S2  (BP or r0x18 is renamed to S2)<br>
S3 = S1 (BP has been initialised with a new value, so BP is now S3)<br>
S4 = S3 (r0x28 is a TMP register, implemented to make generation of RTL code easier)<br>
S4 = S4 + 8<br>
[S4] = [S4] + 1  (As S4 &gt; S0, S4 = S0 + 4, This is param1)<br>
S5 = S3<br>
S5 = S5 + 8<br>
[S5] = [S5] - 1  (As S5 &gt; S0, S5 = S0 + 4, This is param1)<br>

S6 = S3<br>
S6 = S6 + 8<br>
R1 = [S6]        (As S6 &gt; S0, S6 = S0 + 4, This is param1) <br>
S7 = [S1]<br>
S1 = S1 + 4<br>
I1 = [S1]  (r0x28 is now I1, because the IP from the calling process was stored at this position on the stack.)<br>
S1 = S1 + 4<br>
IP = I8  (A JUMP instruction, but as I1 came from the stack, it is a RET)<br>

<p>
So, this function seems to have just one PARAM1, and returns R1 (In the eAX register)
<p>
Resulting C code removes all references to Sregisters, and does register substitutions. Any 32bit value is assumed to be int32_t unless we can tell otherwise.<br>
We get:<br>
<code>
int32_t test (int32_t param1);<br><br>
int32_t test (int32_t param1)<br>
{
<pre>  param1 = param1 + 1;
  param1 = param1 - 1;
  return param1;
</pre>}
</code>

<p>
The original test source code was:<br>
<code>
int test ( int value );<br><br>
int test ( int value )<br>
{
<pre>  value++;
  value--;
  return value;
</pre>}<br>
</code>
<p>
Now we just need to automate this revenge CPU step.

<P> This page was last updated on 13th September 2004
</P>
</table>
</table>
<?php
include 'copyright.php' 
?>
</BODY>
</html>
