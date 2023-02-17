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

void show_process_FD(int pid) {
	DIR *dir;
    struct dirent *dir_entry;
    struct stat finfo;
    char d_path[50];
    sprintf(d_path, "/proc/%d/fd", pid);

    if ((dir = opendir(d_path)) == NULL){
        return;
        // perror("opendir");
    }
    while ((dir_entry = readdir(dir)) != NULL) {
        if (isdigit(dir_entry -> d_name[0]) > 0) {
	        char path[50];
	    	sprintf(path, "/proc/%d/fd/%d", pid, atoi(dir_entry -> d_name));

	        if(stat(path, &finfo) != 0) {
	        	perror("stat");
	        }
	        if(finfo.st_mode & S_IRUSR) {
	        	printf("FD: %s\tInode: %ld", dir_entry -> d_name, finfo.st_ino);

		        char link[256];
	   			ssize_t r;

			  	if ((r = readlink(path, link, 50)) < 0) {
			        perror("readlink");
			    }

	   			link[50] = '\0';

	   			printf("\tFile Name: %s\n", link);
	        }

	        
    	}

    }
    closedir(dir);
}

void get_uid() {
	struct passwd *p;
  	uid_t  uid;

  	if ((p = getpwuid(uid = getuid())) == NULL) {
    	perror("getpwuid");
    } else {
    	printf("getpwuid() returned the following info for uid %d:\n", (int) uid);
	    printf("  pw_name  : %s\n", p->pw_name);
	    printf("  pw_uid   : %d\n", (int) p->pw_uid);
	    printf("  pw_gid   : %d\n", (int) p->pw_gid);
	    printf("  pw_dir   : %s\n", p->pw_dir);
	    printf("  pw_shell : %s\n", p->pw_shell);
    }
}


int main (int argc, char *argv[]) {
	// struct stat finfo;
	// fstat(fd, &finfo);
	DIR *dir;
	struct dirent *dir_entry;
    struct stat finfo;

    // Open the /proc directory
    if ((dir = opendir("/proc")) == NULL){
        perror("opendir");
    }
    while ((dir_entry = readdir(dir)) != NULL) {
    	// Check if the directory name is a number (i.e. PID)
    	if (isdigit(dir_entry -> d_name[0]) > 0) {
	    	uid_t uid = getuid();
	    	char path[50];
	    	char line[256];
	    	FILE *fp;

	    	sprintf(path, "/proc/%d/status", atoi(dir_entry -> d_name));
	    	if ((fp = fopen(path, "r")) == NULL) {
	    		perror("fp");
	    	}
	    	
	    	int uid_cur;

			// Check if the process owner is the current user
	    	while (fgets(line, sizeof(line), fp)) {
	            if (sscanf(line, "Uid: %d", &uid_cur)) {
	                if ((int) uid == uid_cur) {
	                    printf("%s\n", dir_entry -> d_name);
	                    show_process_FD(atoi(dir_entry -> d_name));
	                }
	            }
	          
	        }
	        fclose(fp);

	    	// Get entry's information.
	    	// if (stat(path, &finfo) != 0) {
	    	// 	perror("stat");
	    	// }
	    	// printf("%s\n", dir_entry -> d_name);
	    	// show_process_FD(atoi(dir_entry -> d_name));
	    	// if(chmod(path, S_IRWXU|S_IRWXG)) {
	    	// 	show_process_FD(atoi(dir_entry -> d_name));
	    	// }
	        // if (dir_entry -> d_type != DT_UNKNOWN) {
	        // 	show_process_FD(atoi(dir_entry -> d_name));
	      		// printf("%s\n", dir_entry -> d_name);
	  		// }
    	}
    }

    closedir(dir);
    // show_process_FD(atoi(argv[1]));
    return 0;
}