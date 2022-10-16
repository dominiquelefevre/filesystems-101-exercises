#include "solution.h"	
#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/limits.h>
#include <stdbool.h>

/*
#define PROC_PATH "/proc/"
#define MAX_STR_LEN 256



struct lsof_data {
	char command[MAX_STR_LEN];
	char pid[MAX_STR_LEN];
	char user[MAX_STR_LEN];
	char fd[MAX_STR_LEN];
	char type[MAX_STR_LEN];
	char device[MAX_STR_LEN];
	char size_off[MAX_STR_LEN];
	char node[MAX_STR_LEN];
	char name[MAX_STR_LEN];
};

*/

	
/*

inline static void print_lsof_data(const struct lsof_data* info)
{
	printf("%-9.9s %5.5s %16.16s %4.4s%-2.2s %7.7s %18.18s %9.9s %10.10s %s\n",
		info->command, //commmand
		info->pid, //pid
		info->user, //user
		info->fd, //fd
		"", //RWU
		info->type, //type
		info->device, //device
		"", //size/off
		info->node, //node
		info->name //name
		);
}

inline static void get_process_command(const char* pid, char* comm)
{
	char path[MAX_STR_LEN];
	strcpy(path, PROC_PATH);
	strcat(path, pid);
	strcat(path, "/comm");
	FILE* comm_file = fopen(path, "r");
	if (comm_file == NULL)
	{
		fprintf(stderr, "Error: %s\n", strerror(errno));
		return;
	}
	fscanf(comm_file, "%s", comm);
	fclose(comm_file);
}

inline static void get_process_user(const char* pid, char* user)
{
	char path[MAX_STR_LEN];
	strcpy(path, PROC_PATH);
	strcat(path, pid);
	struct stat pstat;
	int res = stat(path, &pstat);
	if (res == -1)
	{
		fprintf(stderr, "Error: %s\n", strerror(errno));
		return;
	}
	struct passwd* pwd = getpwuid(pstat.st_uid);
	if (pwd == NULL)
	{
		fprintf(stderr, "Error: %s\n", strerror(errno));
		return;
	}
	strcpy(user, pwd->pw_name);
}

*/

void lsof(void)
{
	
	//DIR* procdir = opendir(PROC_PATH);
	
	char* procdir  = "/proc";
	DIR* directory_proc = opendir(procdir );
	
	/*
	if (procdir == NULL)
	{
		fprintf(stderr, "Error: %s\n", strerror(errno));
		return -1;
	}
	*/
	
	if (directory_proc == NULL)
	{
		report_error(procdir , errno);
		return;
	}

	struct dirent* entry;
	bool bool_checker = true;
	
    	while(bool_checker){
		errno = 0;
		entry = readdir(directory_proc);
		if (entry == NULL && errno != 0)
		{
			report_error(procdir , errno);
			break;
		}
		
        	if (entry == NULL) 
		{
			break;
		}
		
		bool pid_schecker = true;
		
		//pid check und errno
		for (size_t i = 0; i < strlen(entry->d_name); i++)
		{
			if ( (entry->d_name)[i] >= '0' && (entry->d_name)[i] <= '9')
			{
				continue;
			}
			
			pid_schecker = false;
			break; 
		};
		
		if (!pid_schecker)
		{
			continue;
		}
		
		char direct_way[PATH_MAX];
		char file[PATH_MAX];
		char path_to_file[PATH_MAX * 2];
		int auth_sum = 0;
		
		sprintf(direct_way, "%s/%s/fd", procdir , entry->d_name);
		struct dirent* federal_dir;
		DIR* federal = opendir(direct_way);
		
		auth_sum += auth_sum;
		
		if (federal == NULL)
		{
			report_error(direct_way, errno);
			continue;
		}
		/*
		
		while ((file = readdir(procdir)))
		{
		if (file->d_type == DT_DIR && file->d_name[0] >= '0' && file->d_name[0] <= '9')
			print_proc_info(file->d_name);
			errno = 0;
		}
		if (errno)
		{
			fprintf(stderr, "Error: %s\n", strerror(errno));
			closedir(procdir);
			return -1;
		}
		*/
		while(bool_checker)
		{
			errno = 0;
			federal_dir = readdir(federal);
			
			if (federal_dir == NULL && errno != 0)
			{
				report_error(direct_way, errno);
				break;
			}
			if (federal_dir == NULL)
			{
				break;
			}
			/*
			printf("%-9s %5s %16s %4s%-2s %7s %18s %9s %10s %s\n", "COMMAND", "PID", "USER", "FD", "", "TYPE", "DEVICE", "SIZE/OFF", "NODE", "NAME");
			struct dirent* file;
			errno = 0;
			*/
			if(strcmp(federal_dir->d_name, ".") == 0 || strcmp(federal_dir->d_name, "..") == 0)
			{
				continue;
			}
			
			sprintf(path_to_file, "%s/%s", direct_way, federal_dir->d_name);
			
			ssize_t lenght = readlink(path_to_file, file, PATH_MAX);
			
			if(lenght == -1)
			{
				report_error(path_to_file, errno);
				continue;
			}
			file[lenght] = '\0';
			report_file(file);
		}
		
		closedir(federal);
	}
	closedir(directory_proc);
}
