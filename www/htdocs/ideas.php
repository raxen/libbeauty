<?php
include 'header.php'
?>

<h2>
      Ideas
    </h2>
    <h3>
       1. Some ideas
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

