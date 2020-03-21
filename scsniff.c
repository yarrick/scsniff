#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <asm/ioctls.h>
#include <asm/termbits.h>
#include <sys/ioctl.h>

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

static void reset(int fd, struct session *session) {
    session_reset(session);
    fprintf(stderr, "Waiting for reset..  ");
    fflush(stderr);
    ioctl(fd, TIOCMIWAIT, TIOCM_CAR);
    fprintf(stderr, "Done\n");
}

static void usage(char *name) {
    fprintf(stderr, "\nUsage: %s <device> [<baudrate>]\n", name);
    exit(2);
}

void main(int argc, char **argv) {
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
    setup_serial(fd, baudrate);
    fprintf(stderr, "Opened %s at %d\n", argv[1], baudrate);

    struct session session;
    session_init(&session, setup_serial, fd, baudrate);

    while (1) {
        reset(fd, &session);
        int loops = 0;
        while (1) {
            unsigned char c;
            if (read(fd, &c, 1) >0) {
                printf("%02X ", c);
                fflush(0);
                loops = 0;
                if (session_add_byte(&session, c)) {
                    printf("\n");
                }
            }
            loops++;
            if (loops > 3000000) {
                printf("\n\n");
                fprintf(stderr, "Timeout!\n");
                break;
            }
        }
    }
}
