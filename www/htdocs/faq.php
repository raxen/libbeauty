<?php
include 'header.php'
?>

<table width="90%" class="centered">
<H2>Revenge Frequently Asked Questions 
</H2>
<H4>1. What is Revenge? 
</H4>
<BLOCKQUOTE>Revenge is research into reverse engineering and decompiling. Any budding reverse engineer or decompiler will find this site useful.
</BLOCKQUOTE>
<H4>2. Is there any reverse engineering software here? 
</H4>
<BLOCKQUOTE>The first planned piece of software is going to be called
&quot;revenge&quot;. It is only in planning at the moment. See the
<A HREF="revenge.php">User Guide</A> for the plans so far.</BLOCKQUOTE>
<H4>3. Other tools being planned. 
</H4>
<BLOCKQUOTE>Another tool that will need to be developed is a way to decompress executable files that have run-time decompression software in them. Some wine32 .dll files are compressed, and when loaded create 3 segments in memory and also contain the symbols of all the functions contained in the .dll file. Segment 1 is designated empty space with a certain size. It is this segment that all the function symbols point to. Segment 2 is data, and Segment 3 is some machine code. The Machine code takes the data in segment 2 and expands it into segment 1, so that after expandsion, the function symbols all point to the correct places with valid code in them. We will need to create a tool that takes the original .dll file, and expands it and then saves it again in a .dll format that is still runnable and functions exactly the same as the compressed version.</BLOCKQUOTE>
<H4>4. Another tool being planned. 
</H4>
<BLOCKQUOTE>It would be nice to take a lib or .dll and be able to watch it being accessed. One way to do this would be to provide a tool that would take a .dll and replace it with a new .dll. This new .dll would be a version of the revenge emulator that would act exactly the same as the original .dll, by running the original .dll in the emulator, but at the same time storing a complete record log of all activity in each .dll call from the calling application.</BLOCKQUOTE>
<H4>5. Another tool being planned. 
</H4>
<BLOCKQUOTE>It would be nice to be able to do the same revenge emulator tracing of device drivers. One expects this to work in a similar way that (4) does.</BLOCKQUOTE>
</body>
</html>
