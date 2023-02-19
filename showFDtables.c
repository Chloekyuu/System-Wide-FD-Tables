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

int show_FD(int pid, int threshold, int *m, int per_process, int sysWide, int vnode, int composite) {
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
        return -1;
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
    return fd_num;
}

void find_files(int threshold, int per_process, int sysWide, int vnode, int composite) {
	DIR *dir;
	struct dirent *dir_entry;
    struct stat finfo;
    int m = 0;

    // Open the /proc directory
    if ((dir = opendir("/proc")) == NULL){
        perror("opendir");
    }

    // printf("\t========================================\n");

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
	                    int num_fd = show_FD(atoi(dir_entry -> d_name), threshold, &m,
	                    	per_process, sysWide, vnode, composite);
	                }
	            }
	        }
	        fclose(fp);
    	}
    }

    closedir(dir);

    // printf("\t========================================\n");
}

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
			int num_fd = show_FD(pid, -1, &m, 0, 0, 0, 1);
			printf("\t========================================\n");
		}
		if (per_process == 1) {
			printf("\tPID\tFD\n");
			printf("\t========================================\n");
			int num_fd = show_FD(pid, -1, &m, 1, 0, 0, 0);
			printf("\t========================================\n");
		}
		if (sysWide == 1) {
			printf("\tPID\tFD\tFilename\n");
			printf("\t========================================\n");
			int num_fd = show_FD(pid, -1, &m, 0, 1, 0, 0);
			printf("\t========================================\n");
		}
		if (vnode == 1) {
			printf("\tFD\tInode\n");
			printf("\t========================================\n");
			int num_fd = show_FD(pid, -1, &m, 0, 0, 1, 0);
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