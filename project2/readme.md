#myshell.c 
##i/o redirection  
Input / Output redirection, will modify how a program takes input and/or how a program 
displays the output. Let’s say we have a program called greet, the programs reads from stdin (standard input, user will provide the input), and prints a greeting with the stdin data to stdout (standard output, he user will see the greeting and data on their screen). 
myshell>    ./greet
Enter your name: "Andreas" 
Hello there Andreas 
Enter your name: _

Let's say the user wants to display this greeting to a list of 50 people, instead 
of entering 50 names sequentially, the user will have a list of 50 people stored in 
a text file. The user can incorporate input redirection, by using the "<" symbol, followed
by the specified input file. 
myshell>    ./greet  <  read_names_from_here.txt
Hello there Andreas
Hello there Josef 
... 

Oh no, the user's entire screen will display the data and while doing what was specified,
the user wants to see the output in a separate file. The user can 
incorporate output redirection, by using the ">" symbol, followed by the specified 
output file. Note: output redirection ">" will overwrite/ truncate a file if it exists. 

myshell>    ./greet   <   read_names_from_here.txt   >   output_greet_to_here.txt
myshell>    cat output_greet_to_here.txt
Hello there Andreas
Hello there Josef 
...
(bottom)
 
There is also append output redirection ">>", which is identical to output redirection 
apart from the fact, the specified output file will not be truncated. This means the output
will append to the file if current exists.

For example, the user wanted to append 10 new people to provide greetings to, and 
created a separate text file with those names.  

myshell>    ./greet   <   read_extra_10_names_here.txt   >>   output_greet_to_here.txt
myshell>    cat output_greet_to_here.txt
Hello there Andreas 
Hello there Josef
...
Hello there newName1
Hello there newName2
(bottom)


##piping
The user has read about input and output redirection, with that knowledge, a pipe is simple to understand. There are two sides of a pipe, a "write to", and "read from" side, allowing for a one-way flow of data between the pipe (“|”) from two commands. For examples the input:
 myshell>    cat names.txt | grep Andreas 

The first command, cat, will output the contents of names.txt to the screen, but with the pipe, will output the contents to the “write to” side of the pipe, and grep will take input from the “read from” side of the pipe to search for the instances of the names to display to the screen. Also, to be noted, commands get executed separately, so you will need to close the end of the pipe not in use. And of the two commands, once needs to normally output to stdout, and other normally take input from stdin. 

##background execution 
The user has 3 programs, the first runs a script, the second prints to a file 1 name every 
2 seconds for 10000 names, and the third just prints  the contents of current directory. 
In a single command line for the shell you would not be able to run all 3 "concurrently"

Ex. myshell>   ./myscript   ./nameswait   ls
myshell> ERROR 

Unix has a bunch of control operators, including the background execution operator "&". 
This enables the shell to execute all 3 of those programs in the "background" (if needed), 
resulting in direct reusability of myshell>. Otherwise, the user will have to execute them individually. 

Ex. myshell>   ./myscript  &   ./nameswait  &   ls & 
batch.txt   help.txt   greet   greet.c    myshell    myshell.c    names.txt 
myshell> 

Note: myshell is bit more simpler with the use of &, better described as parallel commands instead of a a bunch of long running commands. As a result, wait() call is used, meaning it will wait for the execution of each process before returning. 

# Built in commands 
##cd 
"change directory" command takes either 0 or 1 argument. If there are no arguments passed, cd  will report the current directory the user is in. 
/home/andreas/3207-projects/3207-Projects/project2  myshell> cd 
cwd: /home/andreas/3207-projects/3207-Projects/project2  

If there is an argument passed; ex. myshell>   cd <directory>. The command will change the 
current directory (the one you are in) to the specified directory. An error will be printed 
if the directory does not exist (can't find it).  
**/home/andreas/3207-projects/3207-Projects/project2  myshell> cd  dir20**
** home/andreas/3207-projects/3207-Projects/project2/dir20   myshell> **

##clr
The "clear" command, takes 0 arguments, and will simply "clear the screen" of the terminal,
ex. You tested this shell out 
(top) 
myshell> ls -la  
... 		 
-rw--------1 andreas andreas   8383 March 2  10:30 .bashhistory
drwxr-xr-x 1 andreas andreas    512 Feb  16  16:32 3207-Projects 
drwxr-xr-x 1 andreas andreas    512 March 2  18:32 something 
drwxr-xr-x 1 andreas andreas    512 March 3  18:33 something_else
... 
(bottom) 
myshell> clr
(top) 
myshell> 

##dir 
The dir command, takes 0 or 1 arguments. If no argument is specified dir will 
display the contents on the current working directory to the screen or to a specified file.
Otherwise, argument specified should be the <directory> whose contents the user wants
to see displayed to the screen or to a specified file. Contents include folders and files, and if the <directory> doesn’t exist, an error will display. 
**myshell> dir** 
Dir8 Dir9 file20 file21 

**myshell>  dir  .. **(previous directory) 
Dir6 Dir7 file4 file19

##environ 
The environ command, 0 arguments, and will display various environment variables to the 
screen or **redirect output** to a specified file. There are a multitude of environment variables, some which are used by many different systems or programs, some only which this shell cares about. Myshell will display some relevant strings, including: USER, PWD, HOME, and PATH (not our custom one). 
**myshell> environ**
USER=andreas
PWD=/home/andreas/3207-projects/3207-Projects/project2
HOME=/home/andreas 
PATH= … 


##echo
The echo command, echo <comment> 
Echo will display the <comment> specified and display the comment to the screen 
or to a **file if output is redirected**. 
Ex. myshell> echo repeat after me
repeat after me 
myshell> 

Echo will print the comment "repeat after me" to the specified output file
"output_to_this_file.txt"  

**myshell> echo "repeat after me" > output_to_this_file.txt
myshell> cat output_to_this_file.txt**
repeat after me 

## help 
The help command, takes 0 arguments, and will print a guide/ manual to myshell to the screen 
or to a file if output is redirected. This readme.md is filled more detailed, and avaible in a more accessible reading format. 

**myshell> help**
WELCOME TO MY SHELL 
.... 

**myshell> help > output_help_to_here.txt
myshell> cat output_help_to_here.txt**

WELCOME TO MY SHELL
... 


##pause 
The pause commands, takes 0 arguments, and will "pause" the shell until the
user hits the enter key ('\0'). 

**myshell> pause**
_ (user hits enter) 
**myshell> _**  

##quit
The quit command, takes 0 arguments (fix), and will exit / close myshell after all malloc variables are freed, insuring no leaks. 

**myshell> myshell quit**
(shell closes, back to another shell) 
