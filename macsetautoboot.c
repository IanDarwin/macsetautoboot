#include <sys/types.h>
#include <sys/pciio.h>
#include <fcntl.h>
#include <stdio.h>

int main(int argc, char **argv) {
	struct pci_io pcio;
	struct pcisel pcisel;

	int fd = open("/dev/pci0", O_RDWR | O_CLOEXEC);
	if (fd < 0) {
		perror("open /dev/pci0: try securelevel 0 OR machdep.allowaperture>0");
		return -1;
	}

	pcisel.pc_bus = 0;
	pcisel.pc_dev = 3;	/* device on this bus */
	pcisel.pc_func = 0;	/* function on this device */

	/*
	pi_sel    A pcisel structure which specifies the bus, slot and
                       function the user would like to query.

	 pi_reg    The PCI configuration register the user would like to
                       access.

	 pi_width  The width, in bytes, of the data the user would like to
                       read.  This value can be only 4.

	 pi_data   The data returned by the kernel.
	*/
	pcio.pi_sel = pcisel;
	pcio.pi_reg = 0x78;	/* must be 32-bit aligned */
	pcio.pi_width = 4;	/* "This value can only be 4" */

	int ret = ioctl(fd, PCIOCREAD, &pcio);
	if (ret < 0) {
		perror("PCIOCREAD");
		return -1;
	}

	printf("read returned %d, data %08x\n", ret, htonl(pcio.pi_data));

	// If we get this far, change pcio_pi_data and write it back.
	pcio.pi_data = ntohl(htonl(pcio.pi_data & 0x1fffffff));

	printf("About to write updated data %08x\n", htonl(pcio.pi_data));
	ret = ioctl(fd, PCIOCWRITE, &pcio);
	if (ret < 0) {
		perror("PCIOCWRITE");
		return -1;
	}

	return 0;
}
