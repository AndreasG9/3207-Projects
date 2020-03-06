# myshell.c 
Project 2
Andreas Gagas tug25511@temple.edu 

## i/o redirection  
Input / Output redirection, will modify how a program takes input and/or how a program 
displays the output. Let’s say we have a program called greet, the programs reads from stdin (standard input, user will provide the input), and prints a greeting with the stdin data to stdout (standard output, he user will see the greeting and data on their screen).   <br /><br />`myshell>    ./greet`
<br />`Enter your name: "Andreas"`
<br />**`Hello there Andreas`**
<br />`Enter your name: _`

Let's say the user wants to display this greeting to a list of 50 people, instead 
of entering 50 names sequentially, the user will have a list of 50 people stored in 
a text file. The user can incorporate input redirection, by using the "<" symbol, followed
by the specified input file. 
<br /><br />`myshell>    ./greet  <  read_names_from_here.txt`
<br />**`Hello there Andreas`**
<br />**`Hello there Josef `**
... 

Oh no, the user's entire screen will display the data and while doing what was specified,
the user wants to see the output in a separate file. The user can 
incorporate output redirection, by using the ">" symbol, followed by the specified 
output file. Note: output redirection ">" will overwrite/ truncate a file if it exists. 

<br /><br />`myshell>    ./greet   <   read_names_from_here.txt   >   output_greet_to_here.txt`
<br />`myshell>    cat output_greet_to_here.txt`
<br />**`Hello there Andreas`**
<br />**`Hello there Josef`** 
...
(bottom)
 
There is also append output redirection ">>", which is identical to output redirection 
apart from the fact, the specified output file will not be truncated. This means the output
will append to the file if current exists.

For example, the user wanted to append 10 new people to provide greetings to, and 
created a separate text file with those names.  

<br /><br />`myshell>    ./greet   <   read_extra_10_names_here.txt   >>   output_greet_to_here.txt`
<br />`myshell>    cat output_greet_to_here.txt`
<br />**`Hello there Andreas`** 
<br />**`Hello there Josef`**
...
<br />**`Hello there newName1`**
<br />**`Hello there newName2`**
<br />**`(bottom)`**


## piping
The user has read about input and output redirection, with that knowledge, a pipe is simple to understand. There are two sides of a pipe, a "write to", and "read from" side, allowing for a one-way flow of data between the pipe (“|”) from two commands. For examples the input:
<br /><br />`myshell>    cat names.txt | grep Andreas` 

The first command, cat, will output the contents of names.txt to the screen, but with the pipe, will output the contents to the “write to” side of the pipe, and grep will take input from the “read from” side of the pipe to search for the instances of the names to display to the screen. Also, to be noted, commands get executed separately, so you will need to close the end of the pipe not in use. And of the two commands, once needs to normally output to stdout, and other normally take input from stdin. 

## background execution 
The user has 3 programs, the first runs a script, the second prints to a file 1 name every 
2 seconds for 10000 names, and the third just prints  the contents of current directory. 
In a single command line for the shell you would not be able to run all 3 "concurrently"

<br /><br />`myshell>   ./myscript   ./nameswait   ls`
<br />**`ERROR`** 

Unix has a bunch of control operators, including the background execution operator "&". 
This enables the shell to execute all 3 of those programs in the "background" (if needed), 
resulting in direct reusability of myshell>. Otherwise, the user will have to execute them individually. 

<br /><`br />myshell>   ./myscript  &   ./nameswait  &   ls & `
<br />**`batch.txt   help.txt   greet   greet.c    myshell    myshell.c    names.txt`** 
<br />`myshell> `

Note: myshell is bit more simpler with the use of &, better described as parallel commands instead of a a bunch of long running commands. As a result, wait() call is used, meaning it will wait for the execution of each process before returning. 

# Built in commands 
## cd 
"change directory" command takes either 0 or 1 argument. If there are no arguments passed, cd  will report the current directory the user is in. 
<br />`/home/andreas/3207-projects/3207-Projects/project2  myshell> cd` 
<br />**`cwd: /home/andreas/3207-projects/3207-Projects/project2`**  

If there is an argument passed; ex. myshell>   cd <directory>. The command will change the 
current directory (the one you are in) to the specified directory. An error will be printed 
if the directory does not exist (can't find it).  
<br /> <br />`/home/andreas/3207-projects/3207-Projects/project2  myshell> cd  dir20`
<br />**`home/andreas/3207-projects/3207-Projects/project2/dir20   myshell>`**

## clr
The "clear" command, takes 0 arguments, and will simply "clear the screen" of the terminal,
ex. You tested this shell out 
<br /><br />  (top) 
<br /> myshell> ls -la  
... 		 
<br />**` -rw--------1 andreas andreas   8383 March 2  10:30 .bashhistory`**
<br /> **`drwxr-xr-x 1 andreas andreas    512 Feb  16  16:32 3207-Projects`**
<br /> **`drwxr-xr-x 1 andreas andreas    512 March 2  18:32 something`** 
<br /> **`drwxr-xr-x 1 andreas andreas    512 March 3  18:33 something_else`**
... 
<br /> **`(bottom)`** 
<br /> myshell> clr
<br /> **`(top)`** 
<br /> myshell> 

## dir 
The dir command, takes 0 or 1 arguments. If no argument is specified dir will 
display the contents on the current working directory to the screen or to a specified file.
Otherwise, argument specified should be the <directory> whose contents the user wants
to see displayed to the screen or to a specified file. Contents include folders and files, and if the <directory> doesn’t exist, an error will display. 
<br /> `myshell> dir`
<br /> **`Dir8 Dir9 file20 file21 `**

<br /> `myshell>  dir  .. **(previous directory)` 
<br /> **`Dir6 Dir7 file4 file19`**

## environ 
The environ command, 0 arguments, and will display various environment variables to the 
screen or **redirect output** to a specified file. There are a multitude of environment variables, some which are used by many different systems or programs, some only which this shell cares about. Myshell will display some relevant strings, including: USER, PWD, HOME, and PATH (not our custom one). 
<br /> <br /> `myshell> environ`
<br /> USER=andreas
<br /> PWD=/home/andreas/3207-projects/3207-Projects/project2
<br /> HOME=/home/andreas 
<br /> PATH= … 


## echo
The echo command, echo <comment> 
Echo will display the <comment> specified and display the comment to the screen 
or to a **file if output is redirected**. 
<br /> <br /> `myshell> echo repeat after me`
<br /> **`repeat after me`**
<br /> `myshell> `

Echo will print the comment "repeat after me" to the specified output file
"output_to_this_file.txt"  

<br /> `myshell> echo "repeat after me" > output_to_this_file.txt`
<br /> `myshell> cat output_to_this_file.txt`
<br /> **`repeat after me `**

## help 
The help command, takes 0 arguments, and will print a guide/ manual to myshell to the screen 
or to a file if output is redirected. This readme.md is filled more detailed, and avaible in a more accessible reading format. 

<br /> <br /> `myshell> help`
<br /> WELCOME TO MY SHELL 
<br /> .... 

<br /> `myshell> help > output_help_to_here.txt`
<br /> `myshell> cat output_help_to_here.txt**`

<br /> `WELCOME TO MY SHEL:`
<br /> `... `


## pause 
The pause commands, takes 0 arguments, and will "pause" the shell until the
user hits the enter key ('\0'). 

<br /> `<br /> myshell> pause`
<br /> `**_ (user hits enter)`**
<br /> `myshell> _` 

## quit
The quit command, takes 0 argument, and will exit / close myshell after all malloc variables are freed, insuring no leaks. 

<br /> `myshell> myshell quit`
<br /> **`(shell closes, back to another shell)`** 

## path 
Takes 0 or more arguments, and will overwrite out custom PATH with each call. If no path if entered, the path is set to empty and only
built in commands will run, until you call path and at least add /bin 

<br /> <br />`myshell> path`
<br />**WARNING, you set the path to empty, to undo call path /bin**
<br />myshell> ls 
<br />**-myshell: error, unix command or program not found**
<br />**Check that /bin is part of path**

<br /> <br />`myshell> path /bin`
<br />**Shell Path Now ...**
<br />**/bin**
<br />`myshell> ls`
<br /> **`dir1 dir2 file10 file20`** 


