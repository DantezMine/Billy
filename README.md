# Billy
## Pipeline
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
#### Fetch
```
if !D/R.stall & !D/R.branch & !E.branch then
	IR.write(input)
```
#### Decode/Registers
```
Dependence:
D/R.dest == r1 | D/R.dest == r2 | E.dest == r1 | E.dest == r2

Forwarding:
M.dest == r1 | M.dest = r2

if !Dependence & !Forwarding then
	if R-type then
		A.write($r1)
		B.write($r2)
	if I-type then
		A.write($r1)
		B.writew(I)
	if J-type then
		A.write($r2)
		B.write(I)
if Dependence then
	stall
if Forwarding then	
	if R-type then
		if M.dest != r1 then
			A.write($r1)
		if !M.dest != r2 then
			B.write($r2)
	if I-type then
		if M.dest != r1 then
			A.write($r1)
		B.writew(I)
	if J-type then
		if M.dest != r2 then
			A.write($r2)
		B.write(I)
```
#### Execute
```
if J-type then
	if condition then
		Out.write({1.output})
	if !condition then
		Out.write(0)
if R-type | I-type then
	Out.write({0,output})
	Flags.write()
```
#### Memory
```
if LDA then
	addr <- input
	out.write(data)
if STR then
	addr <- input
	mem.write($r1)
Out.write(input | data)
if D.r1 == M.dest then
	A.write(input | data)
if D.r2 == M.dest then
	B.write(input | data)
```
#### Writeback
```
W.dest.write(input)
```
### Considerations
- Use static branch prediction, e.g. always taken.
	- Pipeline flushing upon wrong prediction
	- Storing previous PC-state to revert to upon flush
- Decode-units seperated between stages or five central decoding units in shift registers.
## ISA

| INSTR | Type | OPCD   | EXPRESSION                                 | Description                          |
| ----- | ---- | ------ | ------------------------------------------ | ------------------------------------ |
| LDA   | I    | 0b0001 | r1=Mem\[$r2+I\]                            | Load value at $r2 + I into r1        |
| STR   | I    | 0b0010 | Mem\[$r2+I\]=\$r1                          | Store $r1 into $r2+I                 |
| ADD   | R    | 0b0011 | rd=\$r1+\$r2                               | Add $r1 and $r2 into rd              |
| SUB   | R    | 0b0100 | rd=$r1-\$r2                                | Subtract $r2 from $r1 into rd        |
| AND   | R    | 0b0101 | rd=$r1&\$r2                                | Bitwise AND $r1 and $r2 into rd      |
| OR    | R    | 0b0110 | rd=$r1\|\$r2                               | Bitwise OR $r1 and $r2 into rd       |
| XOR   | R    | 0b0111 | rd=$r1^\$r2                                | Bitwise XOR $r1 and $r2 into rd      |
| LSL   | R    | 0b1000 | rd=$r1<<1                                  | Logical shift left $r1 into rd       |
| LSR   | R    | 0b1001 | rd=$r1>>1                                  | Logical shift right $r1 into rd      |
| ASR   | R    | 0b1010 | rd=$r1>>1                                  | Arithmetic shift right $r1 into rd   |
| BEQ   | J    | 0b1011 | PC = $\text{F}_\text{Zero}$ ? $r1+I : PC+4 | Branch to $r1+I if zero flag set     |
| BLT   | J    | 0b1100 | PC = $\text{F}_\text{Neg}$ ? $r1+I : PC+4  | Branch to $r1+I if negative flag set |
| JMP   | J    | 0b1101 | PC = $r1+I                                 | Branch to $r1+I                      |
|       |      | 0b1110 |                                            |                                      |
|       |      | 0b1111 |                                            |                                      |
