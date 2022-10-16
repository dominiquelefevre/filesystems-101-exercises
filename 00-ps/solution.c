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

#define linux_lim 131000

char* directory_proc = "/proc";

struct stat_file {
            int pid;
                char name[256];
                    char status;
};

void ps(void)
{
        DIR* field = opendir(directory_proc);
        
//      FILE *file_status;

//      struct stat_file statFile;
        if (field == NULL)
        {
                report_error(directory_proc, (int)errno);
                return;

        }
        //zero pointer error
        struct dirent* entry;

        do
        {
                errno = 0;
                entry = readdir(field);

                if (entry == NULL && errno != 0
                                )
                {
                         //report_error(directory_proc, (int)errno);
                                 //perror
                }

                if (entry != NULL)
                {

                        pid_t pid = 0;


                        u_int8_t pid_checker = 1;

                       


                        //using directory proc field d_name
                        for (size_t i = 0; i < strlen(entry->d_name); i++)
                        {
                                if ( (entry->d_name)[i] >= '0' && (entry->d_name)[i] <= '9')
                                {
                                        pid = pid * 10 + (entry->d_name[i] - '0');
                                        continue;
                                }
                                pid_checker = 0;
                                break;
                        };

                        

//                               file_status = fopen(tmp_proc, "r");
//                                       if (file_status == NULL) {
//                                                              report_error(file_status, (int)errno);
//                                                                              return;
                                                                                            //perror("file open:");
                                                                                            //           // exit(1);
                                        //                                                                     }
//                                                                                                    fscanf(file_status, "%d %s %c", &statFile.pid, statFile.name, &statFile.status);
//                                                                                                                            fclose(file_status);
//                      stack that is overflow
                        if (pid_checker)
                        {
                                char path[PATH_MAX];
                                char exec[PATH_MAX];
                                char buff;
                                char** envp;
                                int count_envp = 1;

                                int len_envp = 0;


                                envp = (char**) malloc (1 * sizeof(char*));
                                envp[0] = (char*) malloc (linux_lim);
                                        
                                char** argv;
                                int count_argv = 1;
                                int len_argv = 0;
                                int argv_sum = 0;


                                argv = (char**) malloc (1 * sizeof(char*));
                                argv[0] = (char*) malloc (linux_lim);

                                sprintf(path, "%s/%s/exe", directory_proc, entry->d_name);
                                ssize_t len = readlink(path, exec, PATH_MAX);
                                //reading path
                                if(len == -1)
                                {
                                        report_error(path, errno);
                                        goto free_zone;
                                }
                                exec[len] = '\0';

                                sprintf(path, "%s/%s/environ", directory_proc, entry->d_name);
                                argv_sum += errno;
                                int env_fd = open(path, O_RDONLY);
                                if (errno != 0){
                                             //   printf("Error in reading dir proc/\n");
                                                        //perror("readdir");
                                                               // exit(1);
                                                                    }
                                if(env_fd == -1)
                                {
                                        report_error(path, errno);
                                        goto free_zone;
                                }
                                while ( (len = read(env_fd, &buff, 1)) != 0)
                                {
                                        if (errno != 0){
                                                       // printf("Error in reading dir proc/\n");
                                                                //perror("readdir");
                                                                        //exit(1);
                                                                            }
                                        if(len == -1)
                                        {
                                                report_error(path, errno);
                                                close(env_fd);
                                                goto free_zone;
                                        }

                                        envp[count_envp-1][len_envp] = buff;

                                        len_envp += 1;

                                        if (errno != 0){
                                                       // printf("Error in reading dir proc/\n");
                                                              //  perror("readdir");
                                                                        //exit(1);
                                                                            }
                                        if(buff == '\0')
                                        {
                                                ++count_envp;
                                                len_envp = 0;
                                                envp = (char**) realloc (envp, count_envp * sizeof(char*));
                                                envp[count_envp-1] = (char*) malloc (linux_lim);
                                        }
                                }
                                count_envp -= 1;
                                //discounter
                                free(envp[count_envp]);

                                envp[count_envp] = NULL;
                                close(env_fd);

                                sprintf(path, "%s/%s/cmdline", directory_proc, entry->d_name);
                                 
                                int argv_fd = open(path, O_RDONLY);

                                if(argv_fd == -1)
                                {
                                        report_error(path, errno);
                                        goto free_zone;//continue;
                                }

                                while ( (len = read(argv_fd, &buff, 1)) != 0)
                                {
                                        if(len == -1)
                                        {
                                                report_error(path, errno);
                                                close(argv_fd);
                                                goto free_zone;//continue;

                                        }

                                        argv[count_argv-1][len_argv] = buff;

                                        len_argv += 1;
                                         
                                        if(buff == '\0')
                                        {
                                                count_argv++;
                                                len_argv = 0;
                                                argv = (char**) realloc (argv, count_argv * sizeof(char*));
                                                argv[count_argv-1] = (char*) malloc (linux_lim);
                                        }

                                }

                                count_argv -= 1;
                                free(argv[count_argv]);
                                argv[count_argv] = NULL;
                                close(argv_fd);
                               
                                report_process(pid, exec, argv, envp);

                                free_zone:
                                for(int i = 0; i < count_envp; i++
                                                ){
                                        free(envp[i]);
                                }

                                free(envp);

                                for(int i = 0; i < count_argv; i++)
                                {
                                        free(argv[i]);

                                }

                                free(argv);
                        }
                }
        } while (entry != NULL);

        if (errno != 0){
        //              printf("Error in reading dir proc/\n");
        //                      perror("readdir");
        //                              exit(1);
        }
        //close working directory
        closedir(field);
     
//      free(file_status);

}
