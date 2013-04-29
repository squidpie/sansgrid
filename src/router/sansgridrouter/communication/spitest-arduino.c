#include <wiringPi.h>
#include <wiringPiSPI.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>

#define KHZ(freq) (1000*freq)
#define MHZ(freq) (1*KHZ(freq))

#define SPI_SPEED_MHZ	500

#define WRITE_CYCLE	1

#define WRITE_MAX_BYTES	1

ssize_t write_noconst(int fd, void *buf, size_t count)
{
	return write(fd, buf, count);
}

int spiTransfer(char *buffer, int size, ssize_t(*op)(int, void*, size_t))
{
	int i;
	int fd;
	// only 32 bytes can be written at a time; see below
	int bounded_size = (size > WRITE_MAX_BYTES ? WRITE_MAX_BYTES : size);
	// prepend the command and address to the data
	//char newbuffer[bounded_size];



	wiringPiSPIDataRW(0, buffer, bounded_size);
	// Wait for the write to cycle
	usleep(WRITE_CYCLE);
	if (size > WRITE_MAX_BYTES) {
		// Only one page (of 32 bytes) can be written
		// at a time. If more than 32 bytes are being written,
		// break the line into multiple pages
		spiTransfer(&buffer[WRITE_MAX_BYTES], size-WRITE_MAX_BYTES, op);
	}

	return 0;
}



int main(void) {
	int i;
	int fd;
	char buffer[70];
	int toggle = 0;

	// Writing to the EEPROM chip
	//snprintf(buffer, 64, "This is a much longer sentence. It is bigger than 32?");
	//spiTransfer(buffer, 64, write_noconst);

	// Set up SPI
	if ((fd = wiringPiSPISetup (0, MHZ(SPI_SPEED_MHZ))) < 0)
		fprintf(stderr, "SPI Setup failed: %s\n", strerror (errno));
	printf("fd = %i\n", fd);

	//snprintf(buffer, 32, "Well, it's working");
	while (1) {
		buffer[0] = 0x55;
		spiTransfer(buffer, 1, write_noconst);
	}

	close(fd);

	// Read using wraparound
	memset(buffer, 0x0, 32*sizeof(char));
	//spiTransfer(buffer, 4, read);
	printf("%s\n", buffer);
	for (i=0; i<4; i++)
		printf("%.2x", buffer[i]);
	printf("\n");
	/*
	printf("%.2x\n", buffer[0]);
	for (i=1; i<17; i++)
		printf("%.2x", buffer[i]);
	printf("\n");
	for (i=17; i<64; i++) {
		printf("%.2x", buffer[i]);
	}
	printf("\n");
	*/

	// Read the second datum
	//spiTransfer(buffer, 32, read);
	//printf("%s\n", buffer);
	//printf("%x\n", buffer[0]);

	return 0;
}

