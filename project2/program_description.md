# Program Description 

## Overview 
myshell.c is a fully functionally "lite" unix based shell. myshell runs in two modes, user and batch mode. In user mode, myshell infinitely run (taking input, execute), until quit is called. In batch mode, will takes input from a file line by line, and execute each line until EOF. 
<br/><br/>The shell has direct implementation of a small number of "custom" built-in commands which have precedence over their unix similar. For example, dir <directory> is equal to ls <directory>. 
Output and append output redirection if supported for 4 of those built ins (dir, echo,
environ, and help), allowing for output to go to specified file instead of the screen. 
<br/><br/>Any other unix command, or user made program, will execute properly/ as normal as it would in a bash shell, if the path is set correctly. For these commands / programs, the shell has the functionality to redirection input, output, append output, and both (input and output/ append output). Also, piping is supported, between TWO processes (single pipe), and background execution allows for the user to run multiple commands/ programs parallel / seemingly at the same time. Throughout its course, myshell error checks every call, so the loop will not prematurely exit. 

## Design 
./myshell will run in interactive mode (takes user input) 
./myshell batch.txt will run in batch mode (take each input from a line in batch.txt)

Start of the loop, all variables are reset, including all the input / output files, parsed input, etc … 
If ./myshell is run without another arg, it is in user mode and will call get_user_input(). The use of getline() allows us to read to stdin, and the use of strok allows for the input to be split into tokens, every time a space / tab is encountered. The tokens are stored as the **input, best to think of as an array of pointers to strings. If ./myshell batch.txt is run with a specified batch file, it is in batch mode and getline() will read a single line from the file, and use same function to store the tokens. 
At this point we have the input, a short function, quick_error(), will check that the leading and trailing string doesn’t contain a symbol, (ex. ls >>). If an error is found, continue is used to “reset” the loop / skip the rest of the body, and get more input (if EOF not reached in batch mode). 
<br/><br/>Next, the first string is input is checked to see if it’s a built in command, which have direct implementation of their functionality. If found, their handler function is called, and input is passed, which may contain the commands args / options, or invalid strings later to be caught. The quit command will exit loop, but not before freeing every variable that was malloced, in my case I had 3 double ptr variables to free. Regardless if the functional call was successful (custom message printed if not), continue is used to reset. 

At this point, the input contains an external command / program and I call check_for_input_redirection() and check_for_output_redirection (checks both > and >>). If redirection is present, the output / input (or both) is stored in a string, and both the symbol and file is set to NULL. check_for_background() and check_for_pipe() follows, and similarly to checking for redirection functions, the symbol is set to null. In the case for check_for_pipe() there are two variables *read_side[] and *write_side[], which fill contain the set of commands and args if present. Otherwise, for the other functions, the parse() function will store the command and args if present in **parsed,  the previous added NULL will indicate when to stop. 
However, there is a possibility that no redirection, piping, or background execution is present, the input is still parsed and stored, for simplicity. 

Next, the parsed data (located in parsed or read_side && write_side), is checked that the command present is available, with the use of the system call access, and some string manipulation to test again all possible entries in path. For example, ifconfig is the input, and the path contains /sbin, path_check() will concatenate /sbin with / and ifconfig to see that’s its executable is available with our current path.
Finally, we want to run out parsed input, and run_external_command() has a few params, one which specifies the redirection, if both checks for redirection returned true, than a 4 is passed, indicating that both redirection was present and to make two dup2() calls inside . Once called, the function may call run_external_cmd_pipe() if piping is present, as splitting made viewing more simplified. If piping is present, two fork() calls were required, to then execvp the write_side and read_side parsed input, with the parent waiting for both children to finish before returing to the myshell> prompt. Otherwise, the dup2() calls are dependent on the presence of redirections. For example if only input redirection is present, we use open() to open the specified input file for writing to, and replace stdin with that file_des from the open() call. That way when exevp is called with the parsed data, the input is taken from the specified input file. “Reset” the loop. 
If background execution is present, after the first call to run_external_command() the input is then parsed and executed again in a loop, until we run out of input, then the loop is reset. 
The loop will continue until either quit is called in user mode, or an EOF is reached in batch mode. Other than that no matter the error (from what I tested), the shell will continue to run. 


## Testing ## Testing 
### Various issues 
It didn’t make much difference, but I focused solely on interactive mode when first getting all the pieces to work. As described in detail above, I first got user input and stored it. 
I know the data is stored differently, but I treated the use of double pointers as an array of ptrs to string. For example, input[1] would contain the second string. I used a test function called print_this(char **input_to_print), no matter the number of strings passed, it will display the all the strings. For example, I used this function for the input passed and parsed, which would contain the parsed input, to ensure I manipulated the input correctly. 
<br/><br/>Functionality for the built-in commands was implemented and the correct handler function was called with the use of strcmp(). Each function had its own error testing, and direct implementation of output redirection if it needed support. At first I thought cd would be a bit difficult to test, but I used the getcwd() function, and displayed its output before myshell>, as you normally would see in a bash shell. As long as the directory was present to change to, the function worked, if not myshell would display too many args present or directory not here. 
<br/><br/>If for any reason a built-in had an error, a custom error message was used to indicate the specific error. For example, in quick_error_check() I had two sets of an array of strings, both containing symbols the input cannot start or end with (can end with &, but none of the others). The error message would display myshell: syntax near unexpected token: [symbol would be here]. 
<br/><br/>At this point, the so called “easy stuff” was concluded, and the bulk of the issues I had was getting the rest to work at once.  
I used to have a single function that contained the checking for all possible redirections, but I ran into an issue when both redirections were present (> or >> with <), and adding arbitrary global variables was not the best solution. I split the checking into two separate functions and based their return values on if both redirections were present. This made the idea that it did not matter if the input symbol came before the output symbol or vice versa when both redirections are present. 
<br/><br/>For this specific implementation, I focused on a single example:
 <br/> .`/greet < names.txt > output_names.txt`
Greet was simple program  that read from stdin and printed the input with a greeting message, and names.txt contained a list of names. I tested input redirection with ./greet < names.txt,  and tested output redirection with cat names.txt > output.txt && cat names.txt >> output.txt to see if output.txt would contain names 2x. When both redirections were present, the output and input file was stored, allowing for dup2() to be called twice, and the example worked after a iterations / hours. 

<br/>I will say the biggest / most frequent error was a segmentation fault (later go on and fault somewhere) when strcmp() was used and some string was compared against a NULL entry. I later added guards if the input was null to just continue to the next iteration of the loop. This was likely the result of me not properly counting the size of the input. 
Checking for a pipe itself was simple, but for some reason I used to have a shift function, instead of a quality parser, and tried to store the data into a single array of pointers to strings, and shift after execution again. I think at one point it worked, but everything else for some reason was not complete. Instead, I used two sets of arrays of pointers to strings, to signify a write to and read from side, with the use of separate parser to do the work. Noted earlier, print_this() , such a simple function, made testing the strings it stored a whole lot easier. 
<br/>The implementation of the pipe, was direct as it had its own function, but when an error was present in one of the commands (before I had my path_check()), execvp was say its was not successful, but the child would somehow still run. I resolved the issue with exit(EXIT_FAILURE), allowing for the shell to reset / keep going. The other small issue here, even when it was successful, the parent would not wait for the child’s, and I would get output like myshell> dir2 dir3 file20, EVEN with the use of waitpid(). The issue ended up being, because I split the functions, when the parent returned to original run_external_command(), I had to use wait(NULL) again, before it would return 0, indicating success. The addition of error checking every system call, alleviated probable headaches. 
Checking for background execution, I had no issues as I only needs to replace the “&” symbols with NULL, for the parser to correctly work, n amount of times. 

### Forcing Errors  
Despite doing the built-in first, the last segment I did and tested, was adding functionality for the path command. Stored in an array of strings, and initialized with the string “/bin”, this allowed for path_check() to find any unix command found in that directory (ex. ls). If it was a user program executable, that was still checked with access(parsed[0], X_OK) to make sure its exe was available. The path function itself is very powerful, if no args were passed, the path is empty and only built-ins will run, and each call to path overwrites the array of strings with those passed. I added a warning message if the path was set to empty. And despite the instructions stating only start with /bin present, I allowed /sbin commands (ifconfig) as it will probably be tested without adjusting the path.
I should have implemented this function first, instead of allowing for a non-command to go through execvp(). I created a batch.txt of non-commands / non-programs, and every output had the same message that the command or program is not found, check path if its a command or program. 
<br/>As described above, each built-in command has its own error checks, was tested by passing in invalid arguments. The messages would be similar to would bash would respond with if an error was encountered with the unix-similar command. 
## Testing success 
After each supported functionality was tested individually, I created a batch file that tested all probable cases of success. For the built-ins that supported output redirections, I added input that would test its functionality. 
<br/><br/>For example … 
<br/>./greet < names.txt 
<br/>./greet < names.txt > output.txt
<br/>echo Hello there > names_here.txt 
<br/>cat names.txt > names_here.txt 
<br/>cat names.txt >> names_here.txt 
<br/>cat names.txt | grep Andreas
<br/>./print_stuff & ls & ifconfig & ./print_more_stuff &

Check the specified output files, or look at the screen 
## Testing Errors and Success 
<br/>Like what I added to the batch file above, I just littered errors that would not be caught by path_check. 
<br/>…
<br/>cat file_doesn’t_exist.txt 
<br/>…
<br/>wc -c < test.txt 
<br/>wc -c | test.txt 
<br/>… 
The result was combination of successful outputs and the specific error if an error was encountered. For example, wc -c | test.txt resulted in an error as text.txt is not a valid command or program. wc-c < test.txt was successful and displayed a count, as word count 
took input from the file, to determine a specific count. 

To be noted, there was an issue with the command, wc -c could not be found. I used the whereis wc and 
location was /usr/bin/wc         /usr/share/man/man1/wc.1.gz. I added both to the path and I got back a specific number, indicating the count of something, success! 

Overall, lab was bit longer than the previous, required more planning and tweaking to get all the parts moving and working at once. If I had to do the over again, would take less time, and would have a few less global variables. 
