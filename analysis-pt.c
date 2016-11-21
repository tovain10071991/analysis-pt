#include "pt-module.h"

#include <intel-pt.h>

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>

const char* pt_out_path = "./pt.out";

const char* pt_dev = "/dev/pt-module";

int main(int argc, char** argv) {
    // check pt start
    int pt_fd = open(pt_dev, O_RDONLY | O_CLOEXEC);
    if(pt_fd == -1) {
        perror("open pt_dev failed");
        return -1;
    }
    int status = 0;
    do {
        if(ioctl(pt_fd, PT_GET_STATUS, &status)) {
            perror("get pt status failed");
            return -1;
        }
    } while(status == 2);

    // init pt
    // struct pt_config config;
    // pt_config_init(&config);
    // pt_cpu_arrata(&config.errata, $config.cpu);

    void* buffer = malloc(4UL << 19);
    
    int pt_out = open(pt_out_path, O_RDONLY);
    if(pt_out == -1) {
        perror("open pt.out failed");
        return -1;
    }
    struct stat st;
    off_t size;
    if(fstat(pt_out, &st) == -1) {
        perror("open pt.out failed");
        return -1;
    }
    size = st.st_size;
    while(1) {
        // analysis pt
        printf("size: %lx\n", size);

        if(ioctl(pt_fd, PT_GET_STATUS, &status)) {
            perror("get pt status failed");
            return -1;
        }
        if(status == 2)
            break;
        do {
            if(fstat(pt_out, &st) == -1) {
                perror("open pt.out failed");
                return -1;
            }
        } while(st.st_size == size);
        size = st.st_size;
    }
    do {
        if(fstat(pt_out, &st) == -1) {
            perror("open pt.out failed");
            return -1;
        }
    } while(st.st_size == size);
    size = st.st_size;
    printf("size: %lx\n", size);
    return 0;
}