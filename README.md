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

| INSTR | Type | OPCD   | EXPRESSION                                  | Description                           |
| ----- | ---- | ------ | ------------------------------------------- | ------------------------------------- |
| LDA   | M    | 0b0001 | rd=Mem\[\%r2+I\]                            | Load value at \%r2 + I into rd        |
| STR   | M    | 0b0010 | Mem\[\%r2+I\]=\\%rd                         | Store \%rd into \%r2+I                |
| LDI   | I    | 0b0011 | rd=I                                        | Load I into rd                        |
| ADD   | R    | 0b0100 | rd=\\%r1+\\%r2                              | Add \%r1 and \%r2 into rd             |
| SUB   | R    | 0b0101 | rd=\%r1-\\%r2                               | Subtract \%r2 from \%r1 into rd       |
| AND   | R    | 0b0110 | rd=\%r1&\\%r2                               | Bitwise AND \%r1 and \%r2 into rd     |
| OR    | R    | 0b0111 | rd=\%r1\|\\%r2                              | Bitwise OR \%r1 and \%r2 into rd      |
| XOR   | R    | 0b1000 | rd=\%r1^\\%r2                               | Bitwise XOR \%r1 and \%r2 into rd     |
| LSL   | R    | 0b1001 | rd=\%r1<<1                                  | Logical shift left \%r1 into rd       |
| LSR   | R    | 0b1010 | rd=\%r1>>1                                  | Logical shift right \%r1 into rd      |
| ASR   | R    | 0b1011 | rd=\%r1>>1                                  | Arithmetic shift right \%r1 into rd   |
| BEQ   | I    | 0b1100 | PC = $\text{F}_\text{Zero}$ ? \%rd+I : PC+4 | Branch to \%rd+I if zero flag set     |
| BLT   | I    | 0b1101 | PC = $\text{F}_\text{Neg}$ ? \%rd+I : PC+4  | Branch to \%rd+I if negative flag set |
| JMP   | I    | 0b1110 | PC = \%rd+I                                 | Branch to \%rd+I                      |
|       |      | 0b1111 |                                             |                                       |

### M-type
Flow dependence may occur for `STR` instructions, because the decode stage sees the destination register (which holds the value to be stored) despite no data being written to that data.
_Note:_ Swapped source and destination register.

| 15-12 | 11-9            | 8-6             | 5-0 |
| ----- | --------------- | --------------- | --- |
| op    | r\_dest | r\_src1 | I   |
### R-type
_Note:_ Swapped source and destination register. As labeled below is the correct version.

| 15-12 | 11-9            | 8-6             | 5-3             |
| ----- | --------------- | --------------- | --------------- |
| op    | r\_dest | r\_src1 | r\_src2 |
### I-type

| 15-12 | 11-9            | 7-4             | 3-0            |
| :---- | --------------- | --------------- | -------------- |
| op    | r\_dest | I\_High | I\_Low |
# Assembly Translation
## Grammar
$$
\begin{align}
\text{label} & \gets (.\text{name}:)|\text{name} \\
\text{instr} & \gets \text{name\{\}\{reg|imm|label\,\{\}\}} \\
\text{comment} & \gets \text{\#}* \\
\text{name} & \gets \text{a-zA-Z}\,\text{\{ a-zA-Z0-9 \}} \\
\text{reg} & \gets \%\text{r0-9[0-9]} \\
\text{imm} & \gets \text{hex|bin|dec} \\
\text{start} & \gets \text{\{\}[instr|label]comment\,\{\}}
\end{align}
$$
## Tokenizer
Input: One line of assembly (typically without ending newline character) as null-terminated string.
Output: Array of tokens (heap allocated), ending with token of type "END".

### Patterns
Returns NULL on lines containing only a comment, denoted by \#.

Returns NULL if the input doesn't match the expected patterns declared below.

_Note:_ Any number of whitespace allowed before first character, and typically between seperate tokens.

The following are defined in POSIX-style regular expressions.

\<name\>: [:alpha:][:alnum:]*

\<register\>: %r[:digit:]+}

\<immediate\>: 0x[:xdigit:] | 0b[01] | (0d)?[:digit:]

Labels (definition): [.](name)[:]

Instructions: (name)* [(register)|(immediate)|(name),]\{,3\}

### Token Structure
Register tokens store the register index in the `reg` field.

Label and instruction tokens store the sequence of characters as a null-terminated string in the `name` field.

Immediate tokens convert the string form of their value into the corresponding integer value in the `immediate` field.

Invalid and end tokens store no value.

## Translator
Input: Path to file containing the assembly code.

Output: Bytecode structure containing

- `int num_instr`: Number of instruction lines

- `uint16_t* instr`: Array of data (heap allocated) with all instructions in machine code with big endian formatting.

### Procedure
Passes through the input line by line, tokenizing each line as it goes.
- Invokes the tokenizer on null-terminated line string.
- Stores all tokens in dynamic array, where each line is delimited by an end token.
- Found label definitions are stored and checked for multiplicity.
	- Returns NULL if a label is defined twice.
	- Label stores address (from instruction count) of following instruction in `value` field.
	- Dynamic label array.
- Counts number of lines starting with an instruction token in variable `num_instr`.
	- Instruction address is twice the value of `num_instr`, as instructions are byte addressed.
- Returns NULL if first token of line is neither an instruction or label token.

Second pass iterates through all lines of tokens and converts the tokens into a big endian two-byte instruction.
### Tokens to Machine Code
Input: Array of tokens.

Output: Little endian two-byte instruction.


Opcode is found from instruction table and string comparison.

Based on instruction type, tokens are checked to match the correct pattern, as defined by the ISA.
- I-type can accept either labels or immediates in the immediate field.
- I-type can optionally receive no register argument, in which case the zero-register is taken.
## Assembly Parsing
TODO: Change translation to use Iterator pattern.
- Only store one token at any time.
- `peek()` and `next()` to get tokens.
- Dual-pass labels:
	1. Store label definitions and label references.
	2. Patch jump instructions with correct label values.
TODO: Change tokenization to use regex patterns instead of `sscanf`.
- Token labels (as enumerator) and corresponding regex patterns.
