<?php
include 'header.php'
?>

<table width="90%" class="centered">
<H2>Revenge Status
</H2>
<P> The revenge software application developement has already started. We have built the BFD (binary file decoder, program loader) part and also the x86 to RTL instruction decoder. This will take x86 instruction hex code and convert it directly into RTL instructions. This will be our equivalent of disassembly. We can currently disassemble about 50% of the x86 instructions. We can disassemble 100% of the instructions contained in our first simple test program.
</P>
<P> The current source code can be found in the SVN repository under the "trunk->revenge". <A HREF="http://developer.berlios.de/svn/?group_id=1165">See svn repository for latest test version.</A>
</P>
<P> The next step is writing the memory module. The BFD will load the program into memory in much the same way that the linux kernel loads a program into memory so that the CPU can execute it. Any extra details that might happen to be in the binary headers (e.g. debug info), will be loaded into a separate table, so that other modules can use them as they see fit. The BFD and memory module will not execute any program code.
</P>
<P> Now work on the CPU Emulator can start. This will take the program bytes from the memory module, and determine which bytes are data and which are instructions, build program flow graphs, determine function parameters and gather meta information.
</P>
<P> This page was last updated on 12th September 2004
</P>
</table>
</table>
<?php
include 'copyright.php' 
?>
</BODY>
</html>
