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
<BLOCKQUOTE>Another tool that will need to be developed is a way to decompress executable files that have run-time decompression software in them. Some wine32 .dll files are compressed, and when loaded create 3 segments in memory and also contain the symbols of all the functions contained in the .dll file. Segment 1 is designated empty space with a certain size. It is this segment that all the function symbols point to. Segment 2 is data, and Segment 3 is some machine code. The Machine code takes the data in segment 2 and expands it into segment 1, so that after expandsion, the function symbols all point to the correct places with valid code in them. We will need to create a tool that takes the original .dll file, and expands it and then saves it again in a .dll format that is still runnable and functions exactly the same as the compressed version.
</body>
</html>
