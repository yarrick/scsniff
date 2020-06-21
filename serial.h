#ifndef SCSNIFF_SERIAL_H
#define SCSNIFF_SERIAL_H

int serial_open(char *port);

void serial_configure(int fd, unsigned speed);

int serial_reset_active(int fd);

void serial_wait_reset(int fd);

#endif // SCSNIFF_SERIAL_H
