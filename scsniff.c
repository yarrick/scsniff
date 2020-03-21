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

void main(int argc, char **argv) {
    char *portname = "/dev/ttyUSB0";
    int fd = open(portname, O_RDONLY | O_NOCTTY | O_NDELAY);
    if(fd < 0) {
            fprintf(stderr, "open failed: %d\n", fd);
            return;
    }
    unsigned baudrate = 9600;
    if (argc > 1) {
        baudrate = atoi(argv[1]);
    }
    setup_serial(fd, baudrate);
    fprintf(stderr, "Opened %s at %d\n", portname, baudrate);

    struct session session;
    session_init(&session, setup_serial, fd, baudrate);

    ioctl(fd, TIOCMIWAIT, TIOCM_CAR);
    fprintf(stderr, "Got Reset\n");

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
    }
}
