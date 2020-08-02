
### Commands used to compile and run project.
- Compile by doing the following:
  1. gcc smallsh.c -o smallsh
  2. ./ smallsh

 ## Overview Of Project
 - This Program was written while strictly using the C programing language.
 - The smallsh file works like a bash shell.
 - Allows for the redirection of standard input and standard output and it will support both foreground and background processes  (controllable by the command line and by      receiving signals).
 
 
 ## Specifications 
 - The colon symbol (:) is used to prompt the user for each commmand line.(fflush is called immediately after each and every time you output text).
 - General syntax for usage:  command [arg1 arg2 ...] [< input_file] [> output_file] [&] …where items in square brackets are optional.
 - Supports up to 2048 characters, and a maximum of 512 arguments.
 - Any line that starts with # is ignored and read as a comment.
 - The command line supports the following general commands: ls, status, exit, mkdir, cd, $$(which expands the $$ command into the process ID of the shell).....and more.
 - Supports Background and Foreground, signals ex: CTRL-Z command from the keyboard will send a SIGSTP signal to the parent shell process and all children at the same time.
 
 ## Built with:
 * [Microsoft Visual Studio Code](https://code.visualstudio.com/) - IDE
* [C](https://en.wikipedia.org/wiki/C) - Standard Language 
 * [Linux](https://www.linux.org/) - Operating system

# Preview of the Dungeon Crawler game 
About: Each of the games in this repository were built only to run on in the terminal. The goal of these projects were to improve my skills in object oriented programing.  

![Game](https://user-images.githubusercontent.com/51139158/74704553-a234d500-51c5-11ea-8da5-0e01b5d93408.gif)

### Contributor

* **Sean Murphy** - [SeanMurphy355](https://github.com/Seanmurphy355)

##
