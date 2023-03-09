# Recreating the System-Wide FD Tables
@mainpage showFDtables

@author Huang Xinzi

### How did I solve the problem?

1. Divide each functionality into modules:
    1. Print the FD table for a particular process (with input PID).
    2. Loop the /proc directory to find all the processes owned by the current user.
    3. Handle the command line argument to see which FD format is going to be displayed.
2. Design detailed function for each module:
    1. For example, when designing section (b), I first write a while loop to read all subdictionaries in "/proc". But I found out not all the subdictionaries are processes (such as . and ..), so I skip those dictionaries and only take look at valid process IDs. Still, I cannot successfully read all the processes because I do not have permissions to some of the processes. Then I add a if block to check the process status (i.e. the process' owner) and only displays the FD tables for processes that are owned by the current user.
    2. Now implement section (I). Rcursively open "/proc/[pid]/fd" to find all the FDs assigned to the process with PID pid, and then stat the FD to get its inode number.
3. Add helper functions.
    1. For example, when the user use the program incorrectly, we need to show the user an error message. So I add handle_error to display error message and then terminate the program.
    2. Also, I use a function vertify_arg to validate user's input argument.
    

### An overview of the functions (including documentation)

```c
 void handle_error(char *message);
 		/* Display error message on screen. */

 void show_FD(int pid, int threshold, int *m, int per_process, int sysWide,
 	int vnode, int composite, FILE *output_txt, FILE *output_binary);
 		/* The function opens the file descriptor directory for the given process
 		and reads each file descriptor in its fd directory. Then it displays or
 		outputs those information in the requested format. */

 void find_files(int threshold, int per_process, int sysWide, int vnode,
 	int composite, FILE *output_txt, FILE *output_binary);
 		/* Loop the /proc directory and reads each subdirectory in /proc. If a
 		directory name is a number (represents a PID), the function checks
 		whether the process owner is the current user. If it is, the function
 		calls the show_FD function on that process ID, to display the FD tables
 		in a given format. */

 void show_tables(int pid, int threshold, int per_process, int sysWide,
 	int vnode, int composite, int txt, int binary)
 		/* Display the FD tables with specific formats of either a specific process
 		or all user-owned process based on the input argument flags. If no flags
 		is on (i.e. equals to 1), set default behaviour to display the composite
 		table. If a process ID is provided, the function checks whether the process
 		ID exists and displays the file descriptors of that process in the
 		requested format. If no process ID provided, the function will display the
		FD tables for all processes owned by the current user. */

 void vertify_arg(int argc, char *argv[], int *per_process, int *sysWide,
 	int *vnode, int *composite, int *output_txt, int *output_binary, int *pid,
 	int *threshold);
 		/* Validate the command line arguments user inputted.
 		Use flags to indicate whether an argument is been called. */
```

### How to run (use) my program?

1. Program purpose:
    1. The program is designed to list the file descriptors of a given process.
    2. It works by reading the `/proc` directory and retrieving information about each process. If the process is owned by the current user, the program will examine the `/proc/[PID]/fd` directory to obtain information about each open file descriptor.
    3. This program can display the output in various formats, including composite, per-process, system-wide, and vnode tables. You can also output the composite table into a text or binary file.
2. Run with `make`:
    1. `make` or `make showFDtables`: build the showFDtables executable with warning flags.
    2. `make help`: display help message
    3. `make clean`: remove the showFDtables executable and all its output files
3. The executable can take the following command line arguments:
    
    ```
    --per-process Display the process FD table only
    --systemWide  Display the system-wide FD table only
    --Vnodes      Display the Vnodes FD table only
    --composite   Display the composed table only
    --threshold=X Display the process and its number of FD assigned if its
      					  FD number exceeded the positive integer X.
    --output_TXT  Save the "composite" table in text (ASCII) format into a
      					  file named compositeTable.txt.
    --output_binary	Save the "composite" table in or binary format into a
      						file named compositeTable.bin.
    Y             A positional argument indicating a process ID (should be
      				 	  a positive integer)
    ```
    
4. Assumptions made:
    1. The default display order is:
        
        Composite table, per-process table, system-wide table, and vnode table.
        
        If no flags is given, the default behaviour is to display the composite table.
        
    2. All arguments can be used together (even with themselves) except only one positional argument can be taken.
    3. Calling "`--threshold=X`" multiple times with same input value will not result in error. But if the values are not consistent with each other, an error will occur.
