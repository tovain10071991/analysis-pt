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

struct pt_config config;
struct pt_packet_decoder* decoder;

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
    pt_config_init(&config);
    pt_cpu_errata(&config.errata, &config.cpu);
    decoder = pt_pkt_alloc_decoder(&config);

    void* buffer = malloc(4UL << 19);
    if(buffer == NULL) {
        printf("allocate buffer failed\n");
        return -1;
    }
    
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
        if(size == 0)
            continue;

        config.begin = buffer;
        config.end = buffer + size;

        uint64_t offset = 0;
        struct pt_packet packet;
        while(1) {
            pt_pkt_sync_forward(decoder);
            pt_pkt_get_offset(decoder, &offset);
            if(pt_pkt_next(decoder, &packet, sizeof(packet)) == -pte_eos)
                break;
            switch (packet.type) {
                case ppt_tip: {
                    printf("get tip packet\n");
                    break;
                }
                case ppt_tnt_8: {
                }
                case ppt_tnt_64: {
                    printf("get tnt_64 packet\n");
                    break;
                }
                default: {
                    break;
                }
            }
        }
        break;

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