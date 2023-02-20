/** @file showFDtables.c
 *  @brief Recreating the System-Wide FD Tables
 *
 *  The program is designed to list the file descriptors of a given process.
 *  It works by reading the /proc directory and retrieving information about
 *  each process. If the process is owned by the current user, the program
 *  will examine the /proc/[PID]/fd directory to obtain information about
 *  each open file descriptor. This program can display the output in various
 *  formats, including composite, per-process, system-wide, and vnode tables.
 *
 *  @author Huang Xinzi
 *  @bug No known bugs.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <ctype.h>
#include <errno.h>

/** @brief Print the FD table with a specific format of a process with the given pid.
 * 
 * 	The function opens the file descriptor directory for the given process and reads
 *  each file descriptor in the directory. Then it displays those information in the
 *  requested format.
 * 	
 *  @param pid - An integer that represents the process ID of the given files
 *  @param threshold - An integer used to specify a file descriptor limit.
 * 					   If a limit is set and the number of open file descriptors
 * 					   of the process exceeds that limit, the program will only
 * 					   display the process ID and the number of open file descriptors.
 *  @param m - A pointer to an integer used to track the number of open file descriptors.
 *  @param per_process - An integer flag to indicate whether the output is displayed
 * 					   in per-process format.
 *  @param sysWide - An integer flag to indicate whether the output is displayed in
 * 				       system-wide format.
 *  @param vnode - An integer flag to indicate whether the output is displayed in
 * 				       vnode format.
 *  @param composite - An integer flag to indicate whether the output is displayed in
 * 					   composite format.
 *  @return Void.
 */
void show_FD(int pid, int threshold, int *m, int per_process, int sysWide, int vnode, int composite) {
	DIR *dir;
    struct dirent *dir_entry;
    struct stat finfo;
    int fd_num = 0;
    char d_path[50];
    sprintf(d_path, "/proc/%d/fd", pid);

    if (access(d_path, F_OK) != 0) {
        printf("PID '%d' entered does not exist!\n", pid);
        exit(0);
    }

    if ((dir = opendir(d_path)) == NULL){
        return;
        // perror("opendir");
    }
    while ((dir_entry = readdir(dir)) != NULL) {
        if (isdigit(dir_entry -> d_name[0]) > 0) {
	        char path[50];
	    	sprintf(path, "/proc/%d/fd/%d", pid, atoi(dir_entry -> d_name));

	    	// Get entry's information.
	        if(stat(path, &finfo) != 0) {
	        	perror("stat");
	        }
	        char link[256];
	   		ssize_t r;

			if ((r = readlink(path, link, 50)) < 0) {
			    perror("readlink");
			}

	   		link[50] = '\0';

	   		if (*m != -1 && threshold == -1) {
		   		printf("%d", *m);
		   		*m = *m + 1;
		   	}

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

	        fd_num ++;
    	}
    }
    if (threshold != -1 && fd_num > threshold) {
    	printf("%d (%d), ", pid, fd_num);
    }
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
 *  @return Void.
 */
void find_files(int threshold, int per_process, int sysWide, int vnode, int composite) {
	DIR *dir;
	struct dirent *dir_entry;
    struct stat finfo;
    int m = 0;

    // Open the /proc directory
    if ((dir = opendir("/proc")) == NULL){
        perror("opendir");
    }

    while ((dir_entry = readdir(dir)) != NULL) {
    	// Check if the directory name is a number (i.e. PID)
    	if (isdigit(dir_entry -> d_name[0]) > 0) {
	    	uid_t uid = getuid();
	    	int uid_cur;
	    	char path[50];
	    	char line[256];
	    	FILE *fp;

	    	sprintf(path, "/proc/%d/status", atoi(dir_entry -> d_name));

	    	if ((fp = fopen(path, "r")) == NULL) {
	    		perror("fp");
	    	}

			// Check if the process owner is the current user
	    	while (fgets(line, sizeof(line), fp)) {
	            if (sscanf(line, "Uid: %d", &uid_cur)) {
	                if ((int) uid == uid_cur) {
	                    show_FD(atoi(dir_entry -> d_name), threshold, &m,
	                    	per_process, sysWide, vnode, composite);
	                }
	            }
	        }
	        fclose(fp);
    	}
    }

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
 *  @return Void.
 */
void show_tables(int pid, int threshold, int per_process, int sysWide,
	int vnode, int composite) {
	int m = -1;

	// Default behaviour: if no argument is passed to the program,
	// the program will display the composite table
	if (per_process == 0 && sysWide == 0 && vnode == 0 && composite == 0) {
		composite = 1;
	}

	if (pid != - 1) {
		char path[50];
    	sprintf(path, "/proc/%d", pid);

    	if (access(path, F_OK) != 0) {
        	printf("PID '%d' does not exist!\n", pid);
        	exit(0);
   		}

   		printf(">>> target PID: %d\n", pid);


		if (composite == 1) {
			printf("\tPID\tFD\tFilename\t\tInode\n");
			printf("\t========================================\n");
			show_FD(pid, -1, &m, 0, 0, 0, 1);
			printf("\t========================================\n");
		}
		if (per_process == 1) {
			printf("\tPID\tFD\n");
			printf("\t========================================\n");
			show_FD(pid, -1, &m, 1, 0, 0, 0);
			printf("\t========================================\n");
		}
		if (sysWide == 1) {
			printf("\tPID\tFD\tFilename\n");
			printf("\t========================================\n");
			show_FD(pid, -1, &m, 0, 1, 0, 0);
			printf("\t========================================\n");
		}
		if (vnode == 1) {
			printf("\tFD\tInode\n");
			printf("\t========================================\n");
			show_FD(pid, -1, &m, 0, 0, 1, 0);
			printf("\t========================================\n");
		}
		
	} else {
		if (composite == 1) {
			printf("\tPID\tFD\tFilename\t\tInode\n");
			printf("\t========================================\n");
			find_files(-1, 0, 0, 0, 1);
			printf("\t========================================\n");
		}
		
		if (per_process == 1) {
			printf("\tPID\tFD\n");
			printf("\t========================================\n");
			find_files(-1, 1, 0, 0, 0);
			printf("\t========================================\n");
		}
		if (sysWide == 1) {
			printf("\tPID\tFD\tFilename\n");
			printf("\t========================================\n");
			find_files(-1, 0, 1, 0, 0);
			printf("\t========================================\n");
		}
		if (vnode == 1) {
			printf("\tFD\tInode\n");
			printf("\t========================================\n");
			find_files(-1, 0, 0, 1, 0);
			printf("\t========================================\n");
		}
	}

	if (threshold != -1) {
		printf("## Offending processes:\n");
		find_files(threshold, 0, 0, 0, 0);
		printf("\n");
	}
	
}

void vertify_arg(int argc, char *argv[], int *per_process, int *sysWide,
	int *vnode, int *composite, int *pid, int *threshold) {
	int tmp_pid, tmp_threshold;
	for (int i = 1; i < argc; i ++) {
        if (strcmp(argv[i], "--per-process") == 0) {
        	*per_process = 1;
        } else if (strcmp(argv[i], "--systemWide") == 0) {
        	*sysWide = 1;
        } else if (strcmp(argv[i], "--Vnodes") == 0) {
        	*vnode = 1;
        } else if (strcmp(argv[i], "--composite") == 0) {
        	*composite = 1;
        } else if (sscanf(argv[i], "--threshold=%d", &tmp_threshold) == 1) {
        	if (*threshold != -1 && *threshold != tmp_threshold) {
        		printf("The value given to --threshold=X should be consistent!\n");
        		exit(0);
        	} else if (tmp_threshold < 0) {
        		printf("The value given to --threshold=X should be a positive int!\n");
        		exit(0);
        	}
        	*threshold = tmp_threshold;
        } else if (sscanf(argv[i], "%d", &tmp_pid) == 1) {
        	if (*pid != -1) {
        		printf("Can only take one positional argument indicating a PID!\n");
        		exit(0);
        	} else if (tmp_pid < 0) {
        		printf("Can only take a positive integer indicating a PID!\n");
        		exit(0);
        	}
        	*pid = tmp_pid;
        } else {
        	// If the statement is in other format, print an error message
            printf("Invalid arguments: \"%s\"\n", argv[i]);
            exit(0);
        }
    }
}

int main (int argc, char *argv[]) {
	int pid = -1;
	int threshold = -1;

	int per_process = 0, sysWide = 0, vnode = 0, composite = 0;
	vertify_arg(argc, argv, &per_process, &sysWide, &vnode, &composite, &pid, &threshold);
	show_tables(pid, threshold, per_process, sysWide, vnode, composite);

    return 0;
}