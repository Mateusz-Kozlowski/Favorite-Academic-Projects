/**
 * This file is for implementation of mimpirun program.
 * */

#include "mimpi_common.h"
#include "channel.h"

void set_n_in_env(int n) {
    char n_str[6];
    sprintf(n_str, "%d", n);
    setenv("MIMPI_n", n_str, 1);
}

// creating channels:
void adjust_desc_to_range(
    int read_fd,
    int new_read_fd,
    int write_fd,
    int new_write_fd
) {
    ASSERT_SYS_OK(dup2(read_fd, new_read_fd) == new_read_fd ? 0 : -1);
    ASSERT_SYS_OK(dup2(write_fd, new_write_fd) == new_write_fd ? 0 : -1);

    // Close the old file descriptors if they are no longer needed:
    if (read_fd != new_read_fd) {
        ASSERT_SYS_OK(close(read_fd));
    }
    if (write_fd != new_write_fd) {
        ASSERT_SYS_OK(close(write_fd));
    }
}

void create_progs_all_types_channels(int n) {
    int descs_cnt = 4 * n * (n - 1);

    for (int i = 0; i < descs_cnt / 2; i++) {
        int channel_dscs[2];
        ASSERT_SYS_OK(channel(channel_dscs));

        const int NEW_READ_FD = FIRST_FD_NO + 2 * i;

        adjust_desc_to_range(
            channel_dscs[0],
            NEW_READ_FD,
            channel_dscs[1],
            NEW_READ_FD + 1
        );
    }
}

void map_prog_in_env(int prog_idx) {
    char env_name[40];
    char env_value[10];

    // Create a string like "MIMPI_<PID>"
    sprintf(env_name, "MIMPI_%d", getpid());

    // Convert prog_idx to string
    sprintf(env_value, "%d", prog_idx);

    // Set the environment variable
    setenv(env_name, env_value, 1);
}

// closing:

void close_useless_child_descs(int child_idx, int n) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (i == j) {
                continue;
            }

            int base = FIRST_FD_NO + 4 * i * (n - 1) + 4 * j;
            
            if (i < j) {
                base -=4;
            }

            if (i != child_idx && j != child_idx) {
                //printf("kekchild %d closes %d %d %d %d\n", child_idx, base, base + 1, base + 2, base + 3);
                ASSERT_SYS_OK(close(base));
                ASSERT_SYS_OK(close(base + 1));
                ASSERT_SYS_OK(close(base + 2));
                ASSERT_SYS_OK(close(base + 3));
            }
            if (i != child_idx && j == child_idx) { // child writes in that cell:
                //printf("child %d closes %d %d\n", child_idx, base, base + 2);
                // so we close reading fds:
                ASSERT_SYS_OK(close(base));
                //ASSERT_SYS_OK(close(base + 1));
                ASSERT_SYS_OK(close(base + 2));
                //ASSERT_SYS_OK(close(base + 3));
            }
            if (i == child_idx && j != child_idx) { // child reads in this cell:
                //printf("child %d closes %d %d\n", child_idx, base + 1, base + 3);
                // so we close writing fds:
                // so we close reading fds:
                //ASSERT_SYS_OK(close(base));
                ASSERT_SYS_OK(close(base + 1));
                //ASSERT_SYS_OK(close(base + 2));
                ASSERT_SYS_OK(close(base + 3));
            }
        }
    }
}

void parent_close_all_descs(int n) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (i != j) {
                int base = FIRST_FD_NO + 4 * i * (n - 1) + 4 * j;

                if (i < j) {
                    base -= 4;
                }
                
                ASSERT_SYS_OK(close(base));
                ASSERT_SYS_OK(close(base + 1));
                ASSERT_SYS_OK(close(base + 2));
                ASSERT_SYS_OK(close(base + 3));
            }
        }
    }
}


int main(int argc, char* argv[]) {
    int n = atoi(argv[1]);
    
    char* prog = argv[2];

    set_n_in_env(n);

    create_progs_all_types_channels(n);

    // run n copies of prog, each in a different process:
    for (int i = 0; i < n; i++) {
        pid_t pid;
        ASSERT_SYS_OK(pid = fork());
        if (!pid) {
            // child:
            map_prog_in_env(i);
            close_useless_child_descs(i, n);

            execvp(prog, &(argv[2])); // we don't use ASSERT_SYS_OK

            // prog is NOT in PATH:
            // Ensure this buffer is large enough #YOLOLO #sus?
            char exec_path[4100];

            sprintf(exec_path, "./%s", prog); // Concatenate "./" with prog
            
            execvp(exec_path, &(argv[2]));
            
            // If we arrived to this place in code it means both exec failed
            // so we return -1 bo inform the parent that there is a fail.
            // Parent will be able to extract the info by checking what wait
            // returns.
            exit(-1);
        }
    }

    // Only now we can use it, or alternatively we can tune up 
    // children clean up after they are born - i-th each child doesn't close 
    // descs related to children with id < i, because i-th child inherited
    // less descs since parent before creating j-th child 
    // closes (j-1)-th child desc.
    parent_close_all_descs(n);

    // wait until all of the childs end:
    for (int i = 0; i < n; i++) {
        ASSERT_SYS_OK(wait(NULL));
    }

    return 0;
}