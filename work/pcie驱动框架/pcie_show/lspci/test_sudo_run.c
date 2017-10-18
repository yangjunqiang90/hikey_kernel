/*
 * Enum all pci device via /proc/bus/pci/.
 */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#define PCI_MAX_BUS 255 /* 8 bits (0 ~ 255) */
#define PCI_MAX_DEV 31 /* 5 bits (0 ~ 31) */
#define PCI_MAX_FUN 7 /* 3 bits (0 ~ 7) */

/*
 * PCI Configuration Header offsets
 */
#define PCICFG_REG_VID 0x00 /* Vendor id, 2 bytes */
#define PCICFG_REG_DID 0x02 /* Device id, 2 bytes */
#define PCICFG_REG_CMD 0x04 /* Command register, 2 bytes */
#define PCICFG_REG_STAT 0x06 /* Status register, 2 bytes */
#define PCICFG_REG_RID 0x08 /* Revision id, 1 byte */
#define PCICFG_REG_PROG_INTF 0x09 /* Programming interface code, 1 byte */
#define PCICFG_REG_SUBCLASS 0x0A /* Sub-class code, 1 byte */
#define PCICFG_REG_BASCLASS 0x0B /* Base class code, 1 byte */
#define PCICFG_REG_CACHE_LINESZ 0x0C /* Cache line size, 1 byte */
#define PCICFG_REG_LATENCY_TIMER 0x0D /* Latency timer, 1 byte */
#define PCICFG_REG_HEADER_TYPE 0x0E /* Header type, 1 byte */
#define PCICFG_REG_BIST 0x0F /* Builtin self test, 1 byte */
#define PCICFG_REG_BAR0 0x10 /* Base addr register 0, 4 bytes */
#define PCICFG_REG_BAR1 0x14 /* Base addr register 1, 4 bytes */
#define PCICFG_REG_BAR2 0x18 /* Base addr register 2, 4 bytes */
#define PCICFG_REG_BAR3 0x1C /* Base addr register 3, 4 bytes */
#define PCICFG_REG_BAR4 0x20 /* Base addr register 4, 4 bytes */
#define PCICFG_REG_BAR5 0x24 /* Base addr register 5, 4 bytes */
#define PCICFG_REG_CIS 0x28 /* Cardbus CIS Pointer */
#define PCICFG_REG_SVID 0x2C /* Subsystem Vendor ID, 2 bytes */
#define PCICFG_REG_SDID 0x2E /* Subsystem ID, 2 bytes */
#define PCICFG_REG_ROMBAR 0x30 /* ROM base register, 4 bytes */
#define PCICFG_REG_CAPPTR 0x34 /* Capabilities pointer, 1 byte */
#define PCICFG_REG_INT_LINE 0x3C /* Interrupt line, 1 byte */
#define PCICFG_REG_INT_PIN 0x3D /* Interrupt pin, 1 byte */
#define PCICFG_REG_MIN_GNT 0x3E /* Minimum grant, 1 byte */
#define PCICFG_REG_MAX_LAT 0x3F /* Maximum lat, 1 byte */


void list_pci_devices()
{
    unsigned int bus, dev, fun;

    //printf("BB:DD:FF VID:DID(RID)\n");

    for (bus = 0; bus <= PCI_MAX_BUS; bus++) {
        for (dev = 0; dev <= PCI_MAX_DEV; dev++) {
            for (fun = 0; fun <= PCI_MAX_FUN; fun++) {
                char proc_name[64];
                int cfg_handle;
                uint32_t data;
                uint16_t vid, did;
                uint8_t rid;

                snprintf(proc_name, sizeof(proc_name),
                    "/proc/bus/pci/%02x/%02x.%x", bus, dev, fun);
                cfg_handle = open(proc_name, O_RDWR);
				
                if (cfg_handle <= 0) {
                    continue;
                }
                lseek(cfg_handle, PCICFG_REG_VID, SEEK_SET);
                read(cfg_handle, &data, sizeof(data));

                /* Identify vendor ID */
                if ((data != 0xFFFFFFFF) && (data != 0)) {
                    lseek(cfg_handle, PCICFG_REG_RID, SEEK_SET);
                    read(cfg_handle, &rid, sizeof(rid));
                    vid = data&0xFFFF;
                    did = data>>16;
                   
                    printf("[bus:dev:fun] {%02X:%02X:%02X}", bus, dev, fun);
                    if (rid > 0) {
                        printf(" %04X:%04X (rev %02X)\n", vid, did, rid);
                    } else {
                        printf(" %04X:%04X\n", vid, did);
                    }
                }
            }
        }
    }
}


int main(int argc, char **argv)
{
    list_pci_devices();

    return 0;
}
