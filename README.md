
### Commands used to compile and run project.
- Compile by doing the following:
  - gcc smallsh.c -o smallsh
  - ./ smallsh

 ## Overview Of Project
 - This Program was written while strictly using the C programing language.
 - The smallsh file works like a bash shell enabling users 
 - Allows for the redirection of standard input and standard output and it will support both foreground and background processes  (controllable by the command line and by      receiving signals).
 
 
 ## Specifications 
 - The colon symbol (:) is used to prompt the user for each commmand line.(fflush is called immediately after each and every time you output text).
 - General syntax for usage:  command [arg1 arg2 ...] [< input_file] [> output_file] [&] …where items in square brackets are optional.
 - Supports up to 2048 characters, and a maximum of 512 arguments.
 - Any line that starts with # is ignored and read as a comment.
 - The command line supports the following commands: ls, status, exit, mkdir, cd, $$(which expands the $$ command into the process ID of the shell).
 - Supports Background and Foreground, signals ex: CTRL-Z command from the keyboard will send a SIGSTP signal to the parent shell process and all children at the same time.
 
 ## Built with:
 * [Microsoft Visual Studio Code](https://code.visualstudio.com/) - IDE
* [C](https://en.wikipedia.org/wiki/C) - Standard Language 
 * [Linux](https://www.linux.org/) - Operating system
