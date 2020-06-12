#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <asm/ioctls.h>
#include <asm/termbits.h>
#include <sys/ioctl.h>
#include <sys/time.h>

#include "result.h"
#include "session.h"

static void setup_serial(int fd, unsigned speed) {
    struct termios2 tio;
    memset(&tio, 0, sizeof(tio));
    // 8 data bits, readonly, ignore carrier, even parity, 2 stop bits,
    // custom baud rate.
    tio.c_cflag = CS8|CREAD|CLOCAL|PARENB|CSTOPB|BOTHER;
    tio.c_ospeed = speed;
    ioctl(fd, TCSETS2, &tio);
}

static int reset_active(int fd) {
    unsigned int status;
    if (ioctl(fd, TIOCMGET, &status) != 0) {
        fprintf(stderr, "Connection lost\n");
        exit(1);
    }
    return status & TIOCM_CAR;
}

static struct timeval reset_time;

static void wait_reset(int fd) {
    fprintf(stderr, "== Waiting for reset..  ");
    fflush(stderr);
    ioctl(fd, TIOCMIWAIT, TIOCM_CAR);
    fprintf(stderr, "Done\n");
}

static void usage(char *name) {
    fprintf(stderr, "\nUsage: %s <device> [<baudrate>]\n", name);
    exit(2);
}

static void handle_packet(struct packet *packet) {
    struct timeval diff;
    unsigned i;
    timersub(&packet->time, &reset_time, &diff);
    printf("+%ld.%06lds | ", diff.tv_sec, diff.tv_usec);
    switch (packet->result) {
        case NOISE:             printf("NOISE??"); break;
        case PACKET_TO_CARD:    printf("CARD<<<"); break;
        case PACKET_FROM_CARD:  printf("CARD>>>"); break;
        case PACKET_UNKNOWN:    printf("CARD<?>"); break;
        default:                printf("ERROR!!"); break;
    }
    printf(" |");
    for (i = 0; i < packet->data_length; i++) {
        printf(" %02X", packet->data[i]);
    }
    printf("\n");
}

static void log_message(const char *message) {
    fprintf(stderr, "== %s\n", message);
}

int main(int argc, char **argv) {
    if (argc < 2) usage(argv[0]);
    int fd = open(argv[1], O_RDONLY | O_NOCTTY | O_NDELAY);
    if(fd < 0) {
        fprintf(stderr, "Opening %s ", argv[1]);
        perror("failed");
        usage(argv[0]);
    }
    int baudrate = 9600;
    if (argc > 2) {
        baudrate = atoi(argv[2]);
    }
    if (baudrate <= 0) {
        fprintf(stderr, "Failed to parse baudrate '%s'\n", argv[2]);
        usage(argv[0]);
    }
    fprintf(stderr, "== Opened %s\n", argv[1]);

    struct session session;
    session_init(&session, handle_packet, setup_serial, log_message,
                 fd, baudrate);

    while (1) {
        fprintf(stderr, "== Speed: %d baud\n", baudrate);
        if (!reset_active(fd)) wait_reset(fd);
        gettimeofday(&reset_time, NULL);
        while (reset_active(fd)) {
            // Eat noise while reset active.
            unsigned char c;
            read(fd, &c, 1);
        }
        int loops = 0;
        int resets = 0;
        while (1) {
            unsigned char c;
            if (read(fd, &c, 1) >0) {
                loops = 0;
                if (resets < 15) session_add_byte(&session, c);
            }
            if (reset_active(fd)) {
                resets++;
                if (resets > 50) {
                    fprintf(stderr, "\n========================="
                                    "\n== Got warm reset or deactivate\n");
                    break;
                }
            } else {
                resets = 0;
            }
            loops++;
            if (loops > 3000000) {
                fprintf(stderr, "\n=========================\n== Timeout!\n");
                break;
            }
        }
        session_reset(&session);
    }
    return 0;
}
