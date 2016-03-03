#include <sys/types.h>
#include <sys/pciio.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>

int main(int argc, char **argv) {
	struct pci_io pcio;
	struct pcisel pcisel;

	int fd = open("/dev/pci0", O_RDWR | O_CLOEXEC);
	if (fd < 0) {
		if (errno == EACCES)
			perror("open /dev/pci0");
		else
			perror("open /dev/pci0: try securelevel 0 OR machdep.allowaperture>0");
		return -1;
	}

	// .. for an Intel Mac Mini
	// setpci -s 0:1f.0 0xa4.b=0
	// ... for an NVidia Mac Mini
	// setpci -s 00:03.0 0x7b.b=0x19
	// ... for a Unibody Mac Mini
	// setpci -s 0:3.0 -0x7b=20

	pcisel.pc_bus = 0;
	pcisel.pc_dev = 3;	/* device on this bus */
	pcisel.pc_func = 0;	/* function on this device */

	/*
	 pi_sel - A pcisel structure - specifies the bus, slot and function
	 pi_reg - The PCI configuration register the user wants to access.
	 pi_width  The width, in bytes, of the data the user would like to
                       read.  This value can be only 4.
	 pi_data   The data returned by the kernel.
	*/
	pcio.pi_sel = pcisel;
	pcio.pi_reg = 0x78;	/* must be 32-bit aligned */
	//pcio.pi_reg = 0xa4;	/* must be 32-bit aligned */
	pcio.pi_width = 4;	/* "This value can only be 4" */

	int ret = ioctl(fd, PCIOCREAD, &pcio);
	if (ret < 0) {
		perror("PCIOCREAD");
		return -1;
	}
	int word = htonl(pcio.pi_data);

	printf("%d:%d:%d offset 0x%x read ret=%d, data %08x", 
		pcisel.pc_bus, pcisel.pc_dev, pcisel.pc_func,
		pcio.pi_reg,
		ret, word);

	// If we get this far, change pcio_pi_data and write it back.
	int nword = (word & 0xffffff00) | 0x19;
	//int nword = word & 0x00ffffff;
	pcio.pi_data = ntohl(nword);

	printf("-> %08x\n", nword);
	ret = ioctl(fd, PCIOCWRITE, &pcio);
	if (ret < 0) {
		perror("PCIOCWRITE");
		return -1;
	}

	return 0;
}
