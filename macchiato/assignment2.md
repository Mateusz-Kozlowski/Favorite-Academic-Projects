# Assignment 2
## Macchiato 1.1
Info.PO.22/23, v1.2

In this task, you are required to implement a new version of the Macchiato language, which will include the following new language features and improvements to its ecosystem:

1. **Procedures**  
Macchiato version 1.1 has procedures. Procedures can be thought of as functions that have zero or more parameters and do not return any value (in other words, they are of void type). They can be declared and then called within the sequence of instructions with the appropriate arguments.

A procedure declaration includes:
- A header, which contains information about the procedure's name and its parameters (all of which are of integer type). The procedure name is a non-empty string of letters from 'a' to 'z', while parameter names are subject to the same restrictions as variable names, meaning they are single-letter. All parameter names must be different. Parameters are passed by value. Procedure declarations work similarly to variable declarations:
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;- They are located in a block at the same place as variable declarations, meaning at the beginning of a block in the sequence of variable and procedure declarations,
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;- They are visible until the end of their block,
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;- They can be overshadowed,
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;- Within the same block, a procedure cannot be declared twice with the same name,
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;- The body of the procedure, which consists of a sequence of declarations followed by a sequence of instructions executed at the time of the procedure call.
- A procedure call is an instruction that contains:
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;- The procedure name,
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;- Arguments, which are expressions of the Macchiato language.
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Arguments are evaluated at the time of calling the procedure (in the order they are written, from first to last) and in its body, they are available as values of variables corresponding to the procedure parameters. Calling a procedure results in the execution of its contents' instructions, in accordance with the dynamic binding of variables. That means if there is a reference to a variable in the procedure's body, determining which variable it refers to (if there is more than one in the program with the same name and whether such a variable is available at all) happens during the execution of the procedure. The variable currently visible in the procedure's execution environment is used. Note: dynamic variable binding is easier to implement (and therefore chosen in Macchiato), but is practically not used. In languages such as C, Java, or Python, static variable binding occurs.

2. **New Debugger Command**  
The debugger for Macchiato version 1.1 includes support for the new dump command, which allows for the memory dump of the program to a file. This command has the symbol 'm' and requires one parameter being the path to the file. The result of the command should be the saving of the program's memory dump in text form in the specified file. The program's memory dump consists of:
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;- Visible procedure declarations, i.e., their names along with parameter names (without content),
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;- Current valuation of variables (like in the command d 0).

3. **Convenient Creation of Macchiato Programs in Java**  
Macchiato programs in version 1.1 can be created much more conveniently than in the previous version. A set of classes that is a small SDK for Macchiato provides the ability to create programs and their individual parts in a manner similar to a DSL, allowing for the addition of successive declarations and instructions through calls to appropriate methods (see the "builder" design pattern), as well as creating expressions using clear, static functions (see the "factory" design pattern). Creating a program with the following metasyntax:  

begin block  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;var x 101  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;var y 1  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;proc out(a)  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;print a+x  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;end proc  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;x := x - y  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;out(x)  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;out(100)  // this should print 200  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;begin block  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;var x 10  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;out(100) // statically still 200, dynamically 110  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;end block
end block  

could proceed as follows:  

```java
var program = new ProgramBuilder()
    .declareVariable('x', Constant.of(101))
    .declareVariable('y', Constant.of(1))
    .declareProcedure('out', List.of('a'), new BlockBuilder()
         .print(Addition.of(Variable.named('a'), Variable.named('x')))
         .build()
    )  
    .assign('x', Subtraction.of(Variable.named('x'), Variable.named('y')))
    .invoke('out', List.of(Variable.named('x')))
    .invoke('out', List.of(Constant.of(100)))
    .block(new BlockBuilder()
        .declareVariable('x', Constant.of(10))
        .invoke('out', List.of(Constant.of(100)))
        .build() 
    )
    .build();
```
### Tests
The project solution should be supplemented with JUnit tests. Each syntactic construction of the Macchiato 1.1 language should be accompanied by one test.

### Form of the Assignment Solution
The task involves implementing Macchiato version 1.1 according to the given specification. The solution should be in the form of a project named `po_macchiato` created on the faculty's GitLab. You should start by creating an empty project and placing your solution to the first part of the task in the form of the first commit on the master branch. When working on the solution to the second part, you should adhere to good version control practices related to, for example, commit names and their structure (for the purposes of this task, it is considered that the work should be visible in the form of smaller commits, not one huge commit with the entire solution to the second part of the task). Moreover, you should use branches (so-called feature branches) when adding individual functionalities.

As a solution, you should submit a set of packages and classes that allow the execution and tracking of programs in Macchiato 1.1 along with the example program given in point 3 in this language, created in the main function. Therefore, only the necessary `.java` files and `.gitignore` can be found in the Git repository. However, it is not allowed to include binary, temporary, or IDE-specific files/folders.

The final solution should be as a commit on the master branch, which must be pushed to the repository (git push) before the deadline stated in Moodle. The identifier of this commit (e.g., `518507a7e9ea50e099b33cb6ca3d3141bc1d6638`) along with the repository address in the form of `https://gitlab.mimuw.edu.pl/login/po_macchiato.git` should be submitted on Moodle. Remember to set the GitLab project to private visibility and to include the course instructor as a collaborator in the project with Developer-level access.

Those who do not have their own solution to the first task may:
- Write only the first task now, which will be graded on a scale of 0-10, as if it were the submission of the second task alone.
- Write the necessary parts of the first task and the entire second task, then points will be awarded only for the second task.

### Modification History
- June 10, 2023, v. 1.2: Added a new, recommended version of the example (required in the code). This version illustrates the difference between dynamic and static binding of variables. We will also accept static binding (the text refers to dynamic), please just clearly mark in the code that you have implemented it.
- The old version of the example, although not recommended, is still accepted (without affecting the score).

Here is the content (only for the completeness of the change history) of the old version:

begin block  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;var x 57  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;var y 15  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;proc out(a)  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;print a  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;end proc  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;x := x - y  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;out(x)  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;out(125)  
end block  

and the old sample code

```java
var program = new ProgramBuilder()
    .declareVariable('x', Constant.of(57))
    .declareVariable('y', Constant.of(15))
    .declareProcedure(
      'out', List.of('a'),
      new BlockBuilder().print(Variable.named('a')).buildProc()
    )  
    .assign('x', Subtraction.of(Variable.named('x'), Variable.named('y')))
    .invoke('out', List.of(Variable.named('x')))
    .invoke('out', List.of(Constant.of(125)))
    .build();
