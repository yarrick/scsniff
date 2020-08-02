#include "serial.h"

#include <fcntl.h>
#include <string.h>
#include <asm/ioctls.h>
#include <asm/termbits.h>
#include <sys/ioctl.h>

// Reset signaled with Carrier detect or Ring
#define RESET_PIN (TIOCM_CAR | TIOCM_RNG)

int serial_open(char *port) {
    return open(port, O_RDONLY | O_NOCTTY | O_NDELAY);
}

void serial_configure(int fd, unsigned speed) {
    struct termios2 tio;
    memset(&tio, 0, sizeof(tio));
    // 8 data bits, readonly, ignore carrier, even parity, 2 stop bits,
    // custom baud rate.
    tio.c_cflag = CS8|CREAD|CLOCAL|PARENB|CSTOPB|BOTHER;
    tio.c_ospeed = speed;
    ioctl(fd, TCSETS2, &tio);
}

int serial_reset_active(int fd) {
    unsigned int status;
    if (ioctl(fd, TIOCMGET, &status) != 0) {
        return -1;
    }
    return status & RESET_PIN;
}

void serial_wait_reset(int fd) {
    ioctl(fd, TIOCMIWAIT, RESET_PIN);
}
