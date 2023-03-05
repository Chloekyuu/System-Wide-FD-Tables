/** @file showFDtables.c
 *  @brief Recreating the System-Wide FD Tables
 *
 *  The program is designed to list the file descriptors of a given process.
 *  It works by reading the /proc directory and retrieving information about
 *  each process. If the process is owned by the current user, the program
 *  will examine the /proc/[PID]/fd directory to obtain information about
 *  each open file descriptor. This program can display the output in various
 *  formats, including composite, per-process, system-wide, and vnode tables.
 *  You can also output the composite table into a text or binary file.
 *
 *  @author Huang Xinzi
 *  @bug No known bugs.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <ctype.h>
#include <errno.h>

/** @brief Display error message and then terminate the program.
 * 	
 *  @param message - A string containing the error message.
 *  @return Void.
 */
void handle_error(char *message) {
	printf("%s\n", message);
	exit(0);
}

/** @brief Print the FD table with a specific format of a process with the given pid.
 * 
 * 	The function opens the file descriptor directory for the given process and reads
 *  each file descriptor in the directory. Then it displays or outputs those information
 *  in the requested format.
 * 	
 *  @param pid - An integer that represents the process ID of the given files
 *  @param threshold - An integer used to specify a file descriptor limit.
 * 					   If a limit is set and the number of open file descriptors
 * 					   of the process exceeds that limit, the program will only
 * 					   display the process ID and the number of open file descriptors.
 *  @param m - A pointer to an integer used to track the total number of open FDs.
 *  @param per_process - An integer flag to indicate whether the output is displayed
 * 					   in per-process format.
 *  @param sysWide - An integer flag to indicate whether the output is displayed in
 * 				       system-wide format.
 *  @param vnode - An integer flag to indicate whether the output is displayed in
 * 				       vnode format.
 *  @param composite - An integer flag to indicate whether the output is displayed in
 * 					   composite format.
 *  @param output_txt - A FILE pointer to a text file which storing the text output
 * 				   	   for the composite table.
 *  @param output_binary - A FILE pointer to a binary file which storing the binary
 * 				       output for the composite table.
 *  @return Void.
 */
void show_FD(int pid, int threshold, int *m, int per_process, int sysWide, int vnode,
	int composite, FILE *output_txt, FILE *output_binary) {
	DIR *dir;                  // A pointer to a dictionary
    struct dirent *dir_entry;  // A dictionary entry variable
    struct stat finfo;         // A variable to store file information
    int fd_num = 0;  // A integer storing the number of file descriptors of the process
    char d_path[50]; // A string indicating the path to the /proc/[PID]/fd directory
    sprintf(d_path, "/proc/%d/fd", pid);

    // If cannot open the dictionary, the function returns
    if ((dir = opendir(d_path)) == NULL){
        return;
    }

    // Loop the /proc/[PID]/fd directory to get each file descriptor's information
    while ((dir_entry = readdir(dir)) != NULL) {
    	// If the file name is a digit (not . or ..)
        if (isdigit(dir_entry -> d_name[0]) > 0) {
	        // Create a path to that subdictionary (i.e. the file descripor)
	        char path[50];
	    	sprintf(path, "/proc/%d/fd/%d", pid, atoi(dir_entry -> d_name));

	    	// Get entry's information. If error, print a message
	        if(stat(path, &finfo) != 0) {
	        	perror("stat");
	        }

	        char link[256]; // A variable storing the file name
	   		ssize_t r;      // A variable storing the link size

	   		// If r < 0, an error occurs with the readlink() function
 			if ((r = readlink(path, link, 50)) < 0) {
			    perror("readlink");
			}

			// readlink() does not append a terminating null byte to link,
			// So we manually add a terminating null to link
	   		link[50] = '\0';

	   		// Create a string storing the composite output information
			char info[1024];

	   		// Print how many output since now
	   		if (*m != -1 && threshold == -1 && !output_txt && !output_binary) {
		   		printf("%d", *m);
		   	} else if (*m != -1 && (output_txt || output_binary)) {
		   		sprintf(info, "%d\t%d\t%s\t%s\t%ld\n", *m, pid, dir_entry -> d_name,
		   			link, finfo.st_ino);
		   	} else {
		   		sprintf(info, "\t%d\t%s\t%s\t%ld\n", pid, dir_entry -> d_name, link,
		   			finfo.st_ino);
		   	}

		   	// Display the information in the request format
		   	// (including output into files)
	   		if (composite == 1) {
	   			printf("\t%d\t%s\t%s\t%ld\n", pid, dir_entry -> d_name, link, finfo.st_ino);
			}
			if (per_process == 1) {
				printf("\t%d\t%s\n", pid, dir_entry -> d_name);
			}
			if (sysWide == 1) {
				printf("\t%d\t%s\t%s\n", pid, dir_entry -> d_name, link);
			}
			if (vnode == 1) {
				printf("\t%s\t%ld\n", dir_entry -> d_name, finfo.st_ino);
			}
			if (output_txt) {
				// write to the file
				fprintf(output_txt, "%s", info);
			}
			if (output_binary) {
				// write to the file
				fwrite(info, 1, sizeof(info), output_binary);
			}

			if (*m != -1) {
				*m = *m + 1;
			}

			// Count how many file descriptors in this process
	        fd_num ++;
    	}
    }

    // Display the process and the number of FD assigned if the threshold
    // flag is not -1 and fd_num > threshold
    if (threshold != -1 && fd_num > threshold) {
    	printf("%d (%d), ", pid, fd_num);
    }

    // Close the dictionary
    closedir(dir);
}

/** @brief Loop the /proc directory to find processes owned by the current user.
 * 
 * 	The function opens the /proc directory and reads each directory in the directory.
 *  If a directory name is a number (represents a PID), the function checks whether
 *  the process owner is the current user. If it is, the function calls the show_FD
 *  function on that process ID, to display the FD tables in a given format.
 * 
 *  @param threshold - An integer used to specify a file descriptor limit.
 *  @param per_process - An integer flag to indicate whether the output is displayed
 * 				     in per-process format.
 *  @param sysWide - An integer flag to indicate whether the output is displayed in
 * 				     system-wide format.
 *  @param vnode - An integer flag to indicate whether the output is displayed in
 * 				     vnode format.
 *  @param composite - An integer flag to indicate whether the output is displayed in
 * 					 composite format.
 *  @param output_txt - A FILE pointer to a text file which storing the text output
 * 					 for the composite table.
 *  @param output_binary - A FILE pointer to a binary file which storing the binary
 * 				     output for the composite table.
 *  @return Void.
 */
void find_files(int threshold, int per_process, int sysWide, int vnode, int composite,
	FILE *output_txt, FILE *output_binary) {
	DIR *dir;                      // A pointer to a dictionary
	struct dirent *dir_entry;      // A variable to store dictionary entry
    int m = 0;                     // Track the number of total FDs

    // Open the /proc directory. If fails, print an message.
    if ((dir = opendir("/proc")) == NULL){
        perror("opendir");
    }

    // recursively opening subdirectories until you find a valid inode number
    while ((dir_entry = readdir(dir)) != NULL) {
    	// Check if the directory name is a number (i.e. PID)
    	if (isdigit(dir_entry -> d_name[0]) > 0) {
	    	int uid = getuid();      // Get the current user id
	    	int uid_cur;               // To store the user id information
	    	char path[50];             // To store path to the desired dictionary
	    	char line[256];            // To store each line when reading files
	    	FILE *fp;                  // A file pointer to "/proc/[PID]/status"

	    	sprintf(path, "/proc/%d/status", atoi(dir_entry -> d_name));

	    	// Open the "/proc/[PID]/status" file for each process
	    	if ((fp = fopen(path, "r")) == NULL) {
	    		perror("fp");
	    	}

			// Check if the process owner is the current user
	    	while (fgets(line, sizeof(line), fp)) {
	    		// Get the process owner's UID
	            if (sscanf(line, "Uid: %d", &uid_cur)) {
	            	// If the process owner is the current user, display the FD
	            	// tables for the process (i.e. call show_FD)
	                if ((int) uid == uid_cur) {
	                    show_FD(atoi(dir_entry -> d_name), threshold,
	                    	&m, per_process, sysWide, vnode, composite,
	                    	output_txt, output_binary);
	                }
	            }
	        }
	        // Close the file after we done
	        fclose(fp);
    	}
    }
    // Close the dictionary
    closedir(dir);
}

/** @brief Print the FD table with a specific format of a process with the given pid.
 * 
 * 	The function sets the default behaviour if no argument is passed to the program
 *  to display the composite table. If a process ID is provided, the function checks
 *  whether the process ID exists and displays the file descriptors of that process
 *  in the requested format. If no process ID provided, the function will display the
 *  FD tables for all processes owned by the current user in the requested format.
 * 
 *  @param pid - An integer that represents the process ID.
 * 	             If -1 is passed as an argument, the program will list the open files
 *               of all processes owned by the current user. Otherwise it will display
 *               the FD tables for the specified process.
 *  @param threshold - An integer used to specify a file descriptor limit.
 * 					   If a limit is set, the program will display the process ID if
 * 					   and the number of open file descriptors if the number of its
 * 					   number of open file descriptors exceeds that limit.
 *  @param per_process - An integer flag to indicate whether the program will display
 * 					   a per-process format table.
 *  @param sysWide - An integer flag to indicate whether the program will display a
 * 				       system-wide format table.
 *  @param vnode - An integer flag to indicate whether the program will display a
 * 				       vnode format table.
 *  @param composite - An integer flag to indicate whether the program will display a
 * 					   composite format table.
 *  @param txt - An integer flag to indicate whether the program will save a composite
 *             		   table into a text (ASCII) file
 *  @param binary - An integer flag to indicate whether the program will save a composite
 *             		   table into binary file.
 *  @return Void.
 */
void show_tables(int pid, int threshold, int per_process, int sysWide,
	int vnode, int composite, int txt, int binary) {
	int m = -1;

	// Default behaviour: if no argument is passed to the program,
	// the program will display the composite table
	if (per_process == 0 && sysWide == 0 && vnode == 0 && composite == 0) {
		composite = 1;
	}

	// Storing the divided line and the composite title
   	char *title = "\tPID\tFD\tFilename\t\tInode\n";
   	char *line = "\t========================================\n";

	if (pid != - 1) { // If the user entered a PID
		// Create a path to the process corresponding to the PID
		char path[50];
    	sprintf(path, "/proc/%d", pid);

    	// If the process id doesn't exist, print an error message
    	if (access(path, F_OK) != 0) {
        	char message[50];
    		sprintf(message, "PID '%d' entered does not exist!", pid);
        	handle_error(message);
   		}

   		// Print a title
   		printf(">>> target PID: %d\n", pid);

   		// Given the flags' value, display the FD tables in all requested formats
   		// for the process by passing different flag values to the function show_FD
		if (composite == 1) {
			printf("%s%s", title, line);
			show_FD(pid, -1, &m, 0, 0, 0, 1, NULL, NULL);
			printf("%s", line);
		}
		if (per_process == 1) {
			printf("\tPID\tFD\n%s", line);
			show_FD(pid, -1, &m, 1, 0, 0, 0, NULL, NULL);
			printf("%s", line);
		}
		if (sysWide == 1) {
			printf("\tPID\tFD\tFilename\n%s", line);
			show_FD(pid, -1, &m, 0, 1, 0, 0, NULL, NULL);
			printf("%s", line);
		}
		if (vnode == 1) {
			printf("\tFD\tInode\n%s", line);
			show_FD(pid, -1, &m, 0, 0, 1, 0, NULL, NULL);
			printf("%s", line);
		}
	} else { // If no PID is presented by the user
		// Given the flags' value, display the FD tables in all requested formats
   		// for all the process by passing different flag values to find_files
		if (composite == 1) {
			printf("\tPID\tFD\tFilename\t\tInode\n%s", line);
			find_files(-1, 0, 0, 0, 1, NULL, NULL);
			printf("%s", line);
		}
		if (per_process == 1) {
			printf("\tPID\tFD\n%s", line);
			find_files(-1, 1, 0, 0, 0, NULL, NULL);
			printf("%s", line);
		}
		if (sysWide == 1) {
			printf("\tPID\tFD\tFilename\n%s", line);
			find_files(-1, 0, 1, 0, 0, NULL, NULL);
			printf("%s", line);
		}
		if (vnode == 1) {
			printf("\tFD\tInode\n%s", line);
			find_files(-1, 0, 0, 1, 0, NULL, NULL);
			printf("%s", line);
		}
	}

	// If there is a threshold entered, print the process whose number of
	// file descriptors exceeds that limit
	if (threshold != -1) {
		printf("## Offending processes:\n");
		find_files(threshold, 0, 0, 0, 0, NULL, NULL);
		printf("\n");
	}

	// If the user wants to output the composite table as a text file
	if (txt == 1) {
		// Create a text file to store the output information
		FILE *output_txt = fopen("compositeTable.txt", "w"); // write only

		// test for files not existing (i.e. fopen fails)
		if (output_txt == NULL) {
			perror("fopen");
		}

		// Write the title to the file
		fprintf(output_txt, "%s%s", title, line);
		if (pid != -1) {
			// Pass the file pointer to show_FD to write the FD information
			show_FD(pid, -1, &m, 0, 0, 0, 0, output_txt, NULL);
		} else {
			// Pass the file pointer to find_files and let find_files loop each
			// process then call show_FD to write the FD information
			find_files(-1, 0, 0, 0, 0, output_txt, NULL);
		}
		fprintf(output_txt, "%s", line);
		// Close the file after writing
		fclose(output_txt);
	}

	// If the user wants to output the composite table as a binary file
	if (binary == 1) {
		// Create a binary file to store the output information
		FILE *output_binary = fopen("compositeTable.bin", "wb"); // write only

		// test for files not existing (i.e. fopen fails)
		if (output_binary == NULL) {
			perror("fopen");
		}

		// Write the title information to the file
		fwrite(title, 1, sizeof(title), output_binary);
		fwrite(line, 1, sizeof(line), output_binary);
		if (pid != -1) {
			// Pass the file pointer to show_FD to write the FD information
			show_FD(pid, -1, &m, 0, 0, 0, 0, NULL, output_binary);
		} else {
			// Pass the file pointer to find_files and let find_files loop each
			// process then call show_FD to write the FD information
			find_files(-1, 0, 0, 0, 0, NULL, output_binary);
		}
		fwrite(line, 1, sizeof(line), output_binary);
		// Close the file after writing
		fclose(output_binary);
	}
}

/** @brief Validate the command line arguments user gived.
 *
 *  Use flags to indicate whether an argument is been called.
 *  Set the flag variables to 1 if does, and 0 if not.
 *  If any argument call violates the assumptions (see README.txt
 *  part c "Assumptions made:"), report an error.
 *  Note that there can be at most one positional argument indicating
 *  a process id given to the excutable.
 *
 *  @param argc Number of ommand line arguments.
 *  @param argv The array of strings storing command line arguments.
 *  @param per_process - Point to an integer to indicate if "--per-process" is been called
 *  @param sysWide - Point to an integer to indicate if "--systemWide" is been called
 *  @param vnode - Point to an integer to indicate if "--Vnodes" is been called
 *  @param composite - Point to an integer to indicate if "--composite" is been called
 *  @param output_txt - Point to an integer to indicate if "--output_TXT" is been called
 *  @param output_binary - Point to an integer to indicate if "--output_binary" is been called
 *  @param pid - A pointer to an integer that represents the process ID
 *  @param threshold - A pointer to an integer used to specify a file descriptor limit.
 * 					   Set if "--threshold=X" is been called
 *  @return Void.
 */
void vertify_arg(int argc, char *argv[], int *per_process, int *sysWide, int *vnode,
	int *composite, int *output_txt, int *output_binary, int *pid, int *threshold) {
	int tmp_pid, tmp_threshold; // Store temporary pid / threshold valus
	for (int i = 1; i < argc; i ++) {
        // Loop the command line arguments for verifying
        // If a specific argument is been called, set the corresponding flag to 1
        if (strcmp(argv[i], "--per-process") == 0) {
        	*per_process = 1;
        } else if (strcmp(argv[i], "--systemWide") == 0) {
        	*sysWide = 1;
        } else if (strcmp(argv[i], "--Vnodes") == 0) {
        	*vnode = 1;
        } else if (strcmp(argv[i], "--composite") == 0) {
        	*composite = 1;
        } else if (sscanf(argv[i], "--threshold=%d", &tmp_threshold) == 1) {
        	// If user calls "--threshold=X" multiple times with different values,
        	// or if the threshold value is a negative number, report an error
        	if (*threshold != -1 && *threshold != tmp_threshold) {
        		handle_error("The value given to --threshold=X should be consistent!");
        	} else if (tmp_threshold < 0) {
        		handle_error("The value given to --threshold=X should be a positive int!");
        	}
        	// Otherwise set the threshold value to the user's input
        	*threshold = tmp_threshold;
        } else if (sscanf(argv[i], "%d", &tmp_pid) == 1) {
        	// If there are more than one positional arguments, or the positional
        	// argument is negative, report an error
        	if (*pid != -1) {
        		handle_error("Can only take one positional argument indicating a PID!");
        	} else if (tmp_pid < 0) {
        		handle_error("Can only take a positive integer indicating a PID!");
        	}
        	// Otherwise set the process id to the user's input
        	*pid = tmp_pid;
        } else if (strcmp(argv[i], "--output_TXT") == 0) {
        	*output_txt = 1;
        } else if (strcmp(argv[i], "--output_binary") == 0) {
        	*output_binary = 1;
        } else {
        	// If the statement is in other format, print an error message
            printf("Invalid arguments: \"%s\"\n", argv[i]);
            exit(0);
        }
    }
}

int main (int argc, char *argv[]) {
	// Initialize the pid and the threshold to a negative value, indicating
	// user has not set values for these two variables
	int pid = -1;
	int threshold = -1;

	// Create flags for each argument to check if they're being called
	int per_process = 0, sysWide = 0, vnode = 0, composite = 0;
	int output_txt = 0, output_binary = 0;

	// Validate the command line arguments
	vertify_arg(argc, argv, &per_process, &sysWide, &vnode, &composite,
		&output_txt, &output_binary, &pid, &threshold);

	// Print the FD tables in the requested format
	show_tables(pid, threshold, per_process, sysWide, vnode, composite,
		output_txt, output_binary);

    return 0;
}