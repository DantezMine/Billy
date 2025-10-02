
## Pipeline
### Memory
Billy uses a Harvard Architecture, where Instruction memory is seperate from Data memory.
- Instruction Memory
	- Word size: 2 bytes
	- Big endian, i.e. MSB has lower address
### Overview
- Fetch
	- If a branch instruction exists in an instruction register on the Decode or Execute stage, stall the fetching.
- Decode/Registers
	- Two identical copies of the registers for dual read, single write.
	- In case of a flow dependence, stall this and Fetch stage.
	- In case of forwarding, don't pass forwarded register but continue otherwise.
- Execute
	- Controls input to PC, specifically whether to jump or not. Setting its output to $\{ 1,\text{address} \}$, will set the PC to address. Otherwise, $\{ 0,\text{x} \}$ doesn't do anything to the PC. The MSB is just a control line not controlled by any decoder unit.
	- Conditional branches depend on previous operations setting the flags. No comparisons occur during any branch instructions.
- Memory
	- Memory requires a third register copy for writing data.
	- Can also write to the interstage registers ahead of the Execute stage. Requires comparison between Decode-/Register-Stage decoding unit for register addresses.
- Writeback
	- Writes to all three register copies on __falling-edge__. Allows same cycle write-read
### Considerations
- Use static branch prediction, e.g. always taken.
	- Pipeline flushing upon wrong prediction
	- Storing previous PC-state to revert to upon flush
- Decode-units seperated between stages or five central decoding units in shift registers.
## ISA

| INSTR | Type | OPCD   | EXPRESSION                                 | Description                          |
| ----- | ---- | ------ | ------------------------------------------ | ------------------------------------ |
| LDA   | M    | 0b0001 | rd=Mem\[$r2+I\]                            | Load value at $r2 + I into rd        |
| STR   | M    | 0b0010 | Mem\[$r2+I\]=\$rd                          | Store $rd into $r2+I                 |
| LDI   | I    | 0b0011 | rd=I                                       | Load I into rd                       |
| ADD   | R    | 0b0100 | rd=\$r1+\$r2                               | Add $r1 and $r2 into rd              |
| SUB   | R    | 0b0101 | rd=$r1-\$r2                                | Subtract $r2 from $r1 into rd        |
| AND   | R    | 0b0110 | rd=$r1&\$r2                                | Bitwise AND $r1 and $r2 into rd      |
| OR    | R    | 0b0111 | rd=$r1\|\$r2                               | Bitwise OR $r1 and $r2 into rd       |
| XOR   | R    | 0b1000 | rd=$r1^\$r2                                | Bitwise XOR $r1 and $r2 into rd      |
| LSL   | R    | 0b1001 | rd=$r1<<1                                  | Logical shift left $r1 into rd       |
| LSR   | R    | 0b1010 | rd=$r1>>1                                  | Logical shift right $r1 into rd      |
| ASR   | R    | 0b1011 | rd=$r1>>1                                  | Arithmetic shift right $r1 into rd   |
| BEQ   | I    | 0b1100 | PC = $\text{F}_\text{Zero}$ ? $r1+I : PC+4 | Branch to $rd+I if zero flag set     |
| BLT   | I    | 0b1101 | PC = $\text{F}_\text{Neg}$ ? $r1+I : PC+4  | Branch to $rd+I if negative flag set |
| JMP   | I    | 0b1110 | PC = $rd+I                                 | Branch to $rd+I                      |
|       |      | 0b1111 |                                            |                                      |

### M-type
Flow dependence may occur for `STR` instructions, because the decode stage sees the destination register (which holds the value to be stored) despite no data being written to that data.

| 15-12 | 11-9            | 8-6             | 5-0 |
| ----- | --------------- | --------------- | --- |
| op    | r$_\text{src1}$ | r$_\text{dest}$ | I   |
### R-type

| 15-12 | 11-9            | 8-6             | 5-3             |
| ----- | --------------- | --------------- | --------------- |
| op    | r$_\text{src1}$ | r$_\text{dest}$ | r$_\text{src2}$ |
### I-type

| 15-12 | 11-9            | 7-4             | 3-0            |
| :---- | --------------- | --------------- | -------------- |
| op    | r$_\text{dest}$ | I$_\text{High}$ | I$_\text{Low}$ |
