<?php
include 'header.php'
?>

<table width="90%" class="centered">
<H2>Revenge Status
</H2>
<P> The revenge software application developement has already started. We have built the BFD (binary file decoder, program loader) part. We can do the equivalent to what objdump does, except that we don't do disassembly in this module.
</P>
<P> We have also started writing the x86 to RTL instruction decoder. This will take x86 instruction hex code and convert it directly into RTL instructions. This will be our equivalent of disassembly. This has been about 50% done. <A HREF="http://developer.berlios.de/svn/?group_id=1165">See svn repository for latest test version.</A>
</P>
<P> The next step is writing the memory module. The BFD will load the program into memory in much the same way that the linux kernel loads a program into memory so that the CPU can execute it. Any extra details that might happen to be in the binary headers (e.g. debug info), will be loaded into a separate table, so that other modules can use them as they see fit. The BFD and memory module will not execute any program code. <A HREF="http://libreverse.bkbits.net/">Latest code for main libreverse project</A>
</P>
<P> After that, the CPU Emulator will be implemented which will take the program bytes from the memory module, and determine which bytes are data and which are instructions.
</P>
</table>
</table>
<?php
include 'copyright.php' 
?>
</BODY>
</html>
