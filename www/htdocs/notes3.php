<?php
include 'header.php'
?>

<table width="90%" class="centered">
<H2>Revenge Notes
</H2>
<p>Revenge CPU Emulator. &lt;- What should it do.</p>

<p>Recognising variable types.</p>
<p>
<p>In order to propery recognise variable types, one cannot use a local variable per CPU register. The reason for this is that at one moment during the function, the register may be used as a int32_t, but at another use in the same function might be a pointer. One has to therefore keep relabeling the local variable at each new use made of the register. This method produces problems where "join" points exist in the program flow. The register might have been relabeled differently along each branch of the program flow. This problem was considered difficult until I did some research on how algorithms were used in a different but related field; Compilers! The is extensive documentation about my method of renaming local variables even if they use the same register. Fourtunately, someone has come up with a solution very similar to the one I thought of. Instead of explaining my solution, I will simply refer to the solution used in compilers. It is called <a href="http://en.wikipedia.org/wiki/Static_single_assignment_form" >Single Static Assignment or SSA</a>
<p>SSA still needs implementing in revenge.

<P> This page was last updated on 13th April 2009
</P>
</table>
</table>
<?php
include 'copyright.php' 
?>
</BODY>
</html>
