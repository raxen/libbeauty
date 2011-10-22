<?php
include 'header.php'
?>

<h2>
      Ideas
    </h2>
    <h3>
       1. Packet CPU
    </h3>
        <p>
           CPU that processes packets instead of instructions.
        </p>
        <p>
	   No packets have a length, they only have a start escape char, and break esc char, and an end esc char.
	   Start esc/priority/packet/CRC32/End esc.
	   At any time during the transmission of a packet, a higher priority packet can do an esc break, and start a new higher priority packet. Only when this higher priority packet has finished, does the lower priority packet complete.
	   A CPU element can send out a request packet, and whatever external device handles the packet, will reply with the reply packet.
	   A device needs to send a packet to the CPU, it just sends it, and does not bother interrupting the CPU.
	   Due to the packet priorities, the device needing quick processing will automatically get processed, even if that processing is in the middle of a different packet being processed. Thus, no acknoledges to the device are needed, and thus much faster transfers. The CPU can, it is wishes, send a packet back to the device, to tell it that it has processed the packet, but that is optional.
	   All data paths will be 1 bit serial data, with the clock signal included on the same data line as the data. Thus, if no packets are send, no clocking signal reaches the CPU, and the CPU can switch itself off, until a new packet arrives, thus saving power.
	   As only two pins are used per data path (A differential signal), the chip pin count can be kept low. As a result, a single Chip could contain hundreds of CPUs.
	   With so many CPUs, the problem then arises regarding the links between the CPUs.
	   The need for dedicated memory chips will be removed, in faviour of combined CPU/Memory packet processing units.
	   As there are very few calculations that require serial processing, they can instead be handled using massively parralell CPUs which do not necessaryly need high clock speeds.
	   As all data will be serial data, all CPUs can be 1 bit, and process the data one bit at a time as it arrives.
	   This will result in the removal of bytes and words, and one will just be left with bits being processed one bit at a time.
	   The CPU will contain a buffer, so that data can be shifted in serially, and then let the CPU rewind the stream at any point if it needs to go back to any earlier data. The amount of rewind will of course be limited to the memory on the CPU chip.

	   
        </p>
	<p>
	   An alternative approach could be having normal packets with packet lengths, and so one packet would simply be processed after the previous one. Priorities could then be implemented using different approaches. For example, using packet queues based on priorities.

        </p>
    <h3>
       2. Memory Processing Units
    </h3>
        <p>
           Memory modules that have Processing Units on them.
        </p>
        <p>
	   Most conventional CPU and RAM combination read some value from memory, do a calculation on it, and then return it to memory.
Some operations could instead be done on the RAM chip itself. This would reduce the round trip time and increase efficiency. Due to the way that RAM chips are wired up as a sort of matrix, some operations could then be done on the memory in a massively parallel way. For example, XOR, AND, OR between two large blocks of memory.
For example, if the RAM was accessed by sending a packet of data to it, one could for example, XOR an entire section of memory against the data contained in the packet and either store the result in RAM or return the result in a packet of data from it.
	  This idea could be further expanded to a design where RAM and Processing Unit are always located on the same chip. And in order to expand RAM, you also expand the amount of PUs you have. These PUs could not really be called "Central Processing Units" as they will be distributed. One of the major bottlenecks in current CPU designs in the RAM to CPU link. With the RAM and CPU being on the same chip, this bottleneck can be overcome.
	  This design would also move to a architecture that did not contain a large RAM that was accessible by all CPUs, but rather a more distributed CPU design with each CPU able to access a smaller RAM, and if they need to access more RAM, they make requests to other CPUs rather than access the RAM directly. Programming these types of machines is likely to be more difficult than the shared RAM machines, but the performance and power efficiencies would be worth the extra effort.

        </p>



