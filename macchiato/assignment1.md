# Assignment 1
## Another Debugger
Info.PO.22/23
ver 1.0

In this task, you need to write a set of classes that represent the instructions of a simple programming language called Macchiato. Programs in this language are a single block (see instruction description below).

The classes that are the solution should, of course, enable the execution of programs. The result of executing a program should be the execution of individual instructions (in accordance with their description below) and printing on the standard output the values of all variables from the main block of the program. This task does not require loading the program from text files. Example programs in Macchiato should be built into the Java program code from the classes implemented in your solution.

In addition to the ability to execute the program itself, we are also interested in tracking this execution (i.e., the implementation of a simple debugger). Therefore, your solution should provide two possibilities for program execution:
- execution without debugging - here, the program is executed from beginning to end (unless an execution error occurs, but even then, we only print a message and finish the execution without starting the debugger),
- execution with debugging - here, the program stops immediately (before executing the first instruction) and waits for commands from the standard input.

All debugger commands are single-character. After the name of some commands (see below), a single integer may appear after one or more spaces.

### Debugger command set:
- `c(ontinue)`
  - parameterless, the debugged program starts to continue executing (to completion). If the program has already finished, then the command only prints an appropriate message (and does nothing more).
- `s(tep) <number>`
  - the debugged program executes exactly `<number>` steps. By step, we understand the execution of a single instruction (we also count the execution of all nested instructions, regardless of nesting). After performing the given number of steps, the instruction (possibly compound) to be executed next is printed. If the execution reaches the end of the program before performing the given number of instructions, we only print an appropriate message, and the program ends normally.
- `d(isplay) <number>`
  - displays the current valuation. The parameter specifies from how many levels of the higher block the valuation of variables is to be displayed. The command `d 0` means a command to display the current valuation. If the given number is greater than the level of nesting of the current instruction of the program, only an appropriate message is printed.
- `e(xit)`
  - terminates the execution of the program and ends the debugger's operation. We do not print the final valuation of variables.

Macchiato language allows only single-letter variable names (letters of the English alphabet from `a` to `z`). All variables are of int type (the same as in Java).

Programs may contain the following instructions:
- Block
  - a block contains a sequence of variable declarations and a sequence of instructions. Each of these parts may be empty. Declarations placed in the block are visible from its end to the end of its block (and nowhere else). Local declarations can override external declarations.
- Loop `for <variable> <expression> <instructions>`
  - executes `<instructions>` `<value>` times, with each iteration `<instructions>` being performed in a block with `<variable>` taking the next value in the range 0..<value of expression>-1. The value of the expression is calculated only once, just before starting the loop execution (so even if the executed instructions change this value, the number of loop rotations does not change). The given variable is initialized with consecutive values at each loop rotation, so any assignments to it in the nested instructions of the loop do not change the course of control (number of rotations and the value of the variable at the start of subsequent rotations) of the loop. If the calculated value of the expression is less than or equal to zero, the loop does not perform a single rotation. An error in calculating the expression stops further execution of the loop (the instructions in the loop will not be executed even once).
- Conditional instruction `if <expr1> =|<>|<|>|<=|>= <expr2> then <instructions> else <instructions>`
  - standard meaning,
  - first we calculate the first, then the second expression,
  - an error in calculating an expression stops further execution of this instruction,
  - the part `else <instructions>` can be omitted.
- Assigning value to a variable `<name> <expr>`
  - assigns to the variable a value equal to the calculated value of the expression,
  - an error in calculating the expression stops further execution of this instruction (i.e., in such a situation, the variable value is not changed),
  - ends in error if there is no visible declared variable of the given name at this point in the program.
- Print value of expression `print <expr>`
  - the value of the expression is calculated, and then printed in the next line of the standard output.
  - if the calculation of the expression fails, this instruction prints nothing.

Declarations in blocks:
- Variable declaration `<name> <expr>`
  - introduces a variable to the current scope of visibility (related to the containing block) and initializes it with the calculated value of the expression,
  - it is not possible to declare two (or more) variables of the same name in one block,
  - variable names from different blocks may be the same, in particular, variables may be overshadowed (instructions and expressions always see the variable that was declared in the closest enclosing block),
  - you can only declare variables at the beginning of a block (i.e., in the sequence of variable declarations),
  - an error in calculating the expression interrupts further processing of the declaration (i.e., in such a situation, the variable is not declared).

If an error occurs during the execution of instructions, the execution of the program is interrupted, and a message containing the values of all variables visible in the block in which the error occurred (the variables may be declared in that or the surrounding blocks) and the instruction that directly caused the error is printed.

In the Macchiato language, all expressions have integer values. An expression can take one of the following forms:
- **Integer literal**
  - The value of such an expression is the value of the literal. The syntax and range of literals is the same as in Java for the int type.
- **Variable**
  - The value of such an expression is the value of the variable visible at that point in the program. If there is no visible variable of the given name at that point, calculating the value of the variable ends in error.
- **Addition `<expr1> + <expr2>`**
  - The sum of two expressions, first we calculate the first expression, then the second one, and then we perform the addition of the obtained values. In case of an overflow, the result should be the same as what Java would provide for the same values.
- **Subtraction `<expr1> - <expr2>`**
  - The difference between two expressions, first we calculate the first expression, then the second one, and then we subtract the second value from the first one. In case of an overflow, the result should be the same as what Java would provide for the same values.
- **Multiplication `<expr1> * <expr2>`**
  - The product of two expressions, first we calculate the first expression, then the second one, and then we perform the multiplication of the obtained values. In case of an overflow, the result should be the same as what Java would provide for the same values.
- **Division `<expr1> / <expr2>`**
  - The integer division of two expressions, first we calculate the first expression, then the second one, and then we divide the first value by the second value. For negative numbers or different signs, the result should be the same as what Java would provide for the same values. Division ends in error if the second expression's value is zero.
- **Modulo `<expr1> % <expr2>`**
  - The remainder of the integer division of two expressions, first we calculate the first expression, then the second one, and then we divide the first value by the second value, and the result is the remainder. For negative numbers or different signs, the result should be the same as what Java would provide for the same values. Calculation ends in error if the second expression's value is zero.

An example program in the Macchiato language written in metasyntax (we remind you that your solution does not have to load Macchiato program texts, so the specific syntax does not matter in this task):

**begin block**  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;**var** n 30  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;**for** k n-1  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;**begin block**  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;**var** p 1  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;k := k+2  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;**for** i k-2  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;i := i+2  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;**if** k % i == 0  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;p := 0  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;**if** p == 1  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;print k  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;**end block**  
**end block**


As a solution, you should submit a set of packages and classes that allow the execution and tracking of programs in Macchiato, along with the above example program in this language created in the main function.

Good luck!
