<?php
include 'header.php'
?>

<table width="90%" class="centered">
<H2>Revenge Notes
</H2>
<p>Revenge CPU Emulator. &lt;- What should it do.</p>

<p>Recognising variable types.</p>
<p>
<p>In order to propery recognise variable types, one cannot use a local variable per CPU register. The reason for this is that at one moment during the function, the register may be used as a int32_t, but at another use in the same function might be a pointer. One has to therefore keep relabeling the local variable at each new use made of the register. This method produces problems where "join" points exist in the program flow. The register might have been relabeled differently along each branch of the program flow. This problem was considered difficult until I did some research on how algorithms were used in a different but related field; Compilers! There is extensive documentation about my method of renaming local variables even if they use the same register. Fourtunately, someone has come up with a solution very similar to the one I thought of. Instead of explaining my solution, I will simply refer to the solution used in compilers. It is called <a href="http://en.wikipedia.org/wiki/Static_single_assignment_form" >Single Static Assignment or SSA</a>
<P>
<A HREF="reference/gccsummit-2003-proceedings.pdf">See Tree-Single-Static-Assignment or Tree-SSA for short</A>
</P>
<p>SSA still needs implementing in revenge.
<p>The actual implementation in revenge is discussed here.
Initially, one goes through the instruction log and renames the variable every time the local_reg or local_stack entry is assigned a new value with a MOV instruction.
<p>There are extra considerations that are not covered here, e.g. The return values from call() instructions.
<p>Then one has to handle SSA relabling. SSA relabeling is required when there are multiple paths through a function. E.g. Due to an IF statement.
<p>In this scenario, the simple SSA labeling will be wrong. E.g. local_reg10 = local_reg8 or maybe local_reg6 depending on which path was taken to reach this point. An SSA relabling is done so that all occurances of local_reg6 will be renamed to local_reg8. A similar case for local_stackNNNN.
<p>The problem is finding these multipaths in an efficient manner!
The simplest algorithm takes N*N time where N is the amount of instructions in the function. This is therefore exponential and therefore hardly ever a good choice to make in programming. It basically does not scale at all. We really want to modify this to become a C*N where C is a constant.
<p>To handle this, create a link between when each variable is initialised, and where its value is then used.
Make this a bi-directional relationship.
This will then provide the links required to do the SSA relabeling.
This also helps one understand the program, by knowing that if variable X is an input to this instruction, where was it initialised.
It is initialised whenever the variable is assigned a new value, or its value is changed.
If the new value depends on what the old value was, e.g. ADD, MUL, then the variable is both input and output of the instruction.
This bi-directional relationship will help in possible syntax highlighting tools.

<p>Now, need to decide if going forward or working backwards is the best way to do the processing.
The simplest one to understand is the backwards one so I will implement that one.
Pick an instruction to start from. It must be a local_reg or a local_stack src value. (Worry later what to do with dst in e.g. ADD).
scanning for a local_reg is slightly different from scanning for a local_stack so handle differently.
As one steps back, looking for the dst local_reg, mark each instruction as processed. If the next instruction has already been marked "processed", terminate search. If the instruction to be processed is a join point, store all the join points to the list of start points to process, and terminate the current processing.
Then, pick the next start point off the list and continue processing backwards. Even though a new start point is used, the original instruction that one started on is still the one we are looking for.
<p>Terminate processing if:
<p>1) At start of function
<p>2) At a "join" point.
<p>3) The instruction is already marked as processed.
<p>4) The associated dst reg is found.
<p>At termination, pick the next start point off the list.
<p>If no more start points, processing has finished for this original instruction.
<p>Although this algorithm could take a long time, it has advantages.
<p>1) It is deterministic. It will always finish.
<p>2) Worst case, it will process N*N instructions, where N is the number of instructions in the log.
<p>A more normal, non-worst case is less than 1+2+...N or (1 + N)*(N/2) instructions will be processed.
But, even so, N*N/2 is still large. Is there a better way? Looking at dominance trees maybe?
Maybe build new trees with just a single local_reg or local_stack variable in it.
So, in this case a single processing of N instructions would produce a number of smaller trees with less than N elements for each unique variable. This would result is a worst case of processing N+2N (FIXME: Check this number) instructions. A considerable improvement!!! 3N is better than N*N
<p>BUT: N*N algorithm is simpler, so use that for now.

<P> This page was last updated on 10th October 2009
</P>
</table>
</table>
<?php
include 'copyright.php' 
?>
</BODY>
</html>
