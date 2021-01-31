#include "ipkvm.h"

#define IDVENDOR "/sys/kernel/config/usb_gadget/g_multi/idVendor"
#define IDPRODUCT "/sys/kernel/config/usb_gadget/g_multi/idProduct"
#define BCDDEVICE "/sys/kernel/config/usb_gadget/g_multi/bcdDevice"
#define BCDUSB "/sys/kernel/config/usb_gadget/g_multi/bcdUSB"
#define BMAXPACKERSIZE0 "/sys/kernel/config/usb_gadget/g_multi/bMaxPacketSize0"
#define SERIALNUMBER "/sys/kernel/config/usb_gadget/g_multi/strings/0x409/serialnumber"
#define MANUFACTURER "/sys/kernel/config/usb_gadget/g_multi/strings/0x409/manufacturer"
#define PRODUCT "/sys/kernel/config/usb_gadget/g_multi/strings/0x409/product"
#define MAXPOWER "/sys/kernel/config/usb_gadget/g_multi/configs/c.1/MaxPower"
#define USB0 "/sys/kernel/config/usb_gadget/g_multi/functions/hid.0"
#define USB1 "/sys/kernel/config/usb_gadget/g_multi/functions/hid.1"
#define CONF0 "/sys/kernel/config/usb_gadget/g_multi/configs/c.1/hid.0"
#define CONF1 "/sys/kernel/config/usb_gadget/g_multi/configs/c.1/hid.1"
#define K_PROROCOL "/sys/kernel/config/usb_gadget/g_multi/functions/hid.0/protocol"
#define K_SUBCLASS "/sys/kernel/config/usb_gadget/g_multi/functions/hid.0/subclass"
#define K_REPORTLENGTH "/sys/kernel/config/usb_gadget/g_multi/functions/hid.0/report_length"
#define K_REPORTDESC "/sys/kernel/config/usb_gadget/g_multi/functions/hid.0/report_desc"
#define M_PROROCOL "/sys/kernel/config/usb_gadget/g_multi/functions/hid.1/protocol"
#define M_SUBCLASS "/sys/kernel/config/usb_gadget/g_multi/functions/hid.1/subclass"
#define M_REPORTLENGTH "/sys/kernel/config/usb_gadget/g_multi/functions/hid.1/report_length"
#define M_REPORTDESC "/sys/kernel/config/usb_gadget/g_multi/functions/hid.1/report_desc"
#define CONFIGURATION "/sys/kernel/config/usb_gadget/g_multi/configs/c.1/strings/0x409/configuration"
#define UDC "/sys/kernel/config/usb_gadget/g_multi/UDC"

#define KB_DEV "/dev/hidg0"
#define MS_DEV "/dev/hidg1"
#define USB_DEV_NAME "musb-hdrc.0"

#define WO "w"
#define RO "r"
#define RW "rw"

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

#define DESC(_path, _p, _s) \
{                       \
.path = (_path),    \
.ptr = (_p),        \
.size = (_s),       \
}

struct hid_init_desc {
	char *path;
	const void *ptr;
	int size;
};

static const unsigned char hid_report_mouse[] =
{
    0x05, 0x01,         /* Usage Page (Generic Desktop) */
    0x09, 0x02,         /* Usage (Mouse) */
    0xA1, 0x01,         /* Collection (Application) */
    0x09, 0x01,         /*   Usage (Pointer) */
    0xA1, 0x00,         /*     Collection (Physical) */
    0x05, 0x09,         /*     Usage Page (Buttons) */
    0x19, 0x01,         /*     Usage Minimum (1) */
    0x29, 0x03,         /*     Usage Maximum (3) */
    0x15, 0x00,         /*     Logical Minimum (0) */
    0x25, 0x01,         /*     Logical Maximum (1) */
    0x95, 0x08,         /*     Report Count (8) */
    0x75, 0x01,         /*     Report Size (1) */
    0x81, 0x02,         /*     Input (Data, Variable, Absolute) */
    0x05, 0x01,         /*     Usage Page (Generic Desktop) */
    0x09, 0x30,         /*     Usage (X) */
    0x09, 0x31,         /*     Usage (Y) */
    0x35, 0x00,         /*     PHYSICAL_MINIMUM (0) */
    0x46, 0xff, 0x7f,   /*     PHYSICAL_MAXIMUM (32767) */
    0x15, 0x00,         /*     LOGICAL_MINIMUM (0) */
    0x26, 0xff, 0x7f,   /*     LOGICAL_MAXIMUM (32767) */
    0x65, 0x11,         /*     UNIT (SI Lin:Distance) */
    0x55, 0x0e,         /*     UNIT_EXPONENT (-2) */
    0x75, 0x10,         /*      REPORT_SIZE (16) */
    0x95, 0x02,         /*      REPORT_COUNT (2) */
    0x81, 0x02,         /*     INPUT (Data,Var,Abs) */
    0x09, 0x38,         /*     Usage (Wheel) */
    0x15, 0xff,         /*     LOGICAL_MINIMUM (-1) */
    0x25, 0x01,         /*     LOGICAL_MAXIMUM (1) */
    0x35, 0,            /*     PHYSICAL_MINIMUM (-127) */
    0x45, 0,            /*     PHYSICAL_MAXIMUM (127) */
    0x75, 0x08,         /*     REPORT_SIZE (8) */
    0x95, 0x01,         /*     REPORT_COUNT (1) */
    0x81, 0x06,         /*     INPUT (Data,Var,Rel) */
    0xC0,
    0xC0};

/* keyboard Report descriptor */
static const unsigned char hid_report_keyboard[] =
{
	0x05, 0x01,       // Usage Page (Generic Desktop Control)
	0x09, 0x06,       // Usage (Keyboard)
	0xA1, 0x01,       // Collection (Application)
	0x05, 0x07,       // Usage Page (Keyboard)
	0x19, 0xE0,       // Usage Minimum (224) (Left Control)
	0x29, 0xE7,       // Usage Maximum (231) (Right Control)
	0x15, 0x00,       // Logical Minimum (0)
	0x25, 0x01,       // Logical Maximum (1)
	0x75, 0x01,       // Report Size (1)
	0x95, 0x08,       // Report Count (8)
	0x81, 0x02,       // Input (Data, Variable, Absolute, Bit Field)
	0x95, 0x01,       // Report Count (1)
	0x75, 0x08,       // Report Size (8)
	0x81, 0x01,       // Input (Constant, Array, Absolute, Bit Field)
	0x95, 0x05,       // Report Count (5)
	0x75, 0x01,       // Report Size (1)
	0x05, 0x08,       // Usage Page (LEDs)
	0x19, 0x01,       // Usage Minimum (1) (Num Lock)
	0x29, 0x05,       // Usage Maximum (5) (Kana)
	0x91, 0x02,       // Output (Data, Value, Absolute, Non-volatile, Bit Field)
	0x95, 0x01,       // Report Count (1)
	0x75, 0x03,       // Report Size (3)
	0x91, 0x01,       // Output (Constant, Array, Absolute, Non-volatile, Bit Field)
	0x95, 0x06,       // Report Count (6)
	0x75, 0x08,       // Report Size (8)
	0x15, 0x00,       // Logical Minimum (0)
	0x26, 0xFF, 0x00, // Logical Maximum (255)
	0x05, 0x07,       // Usage Page (Keyboard)
	0x19, 0x00,       // Usage Minimum (0)
	0x2A, 0xFF, 0x00, // Usage Maximum (255)
	0x81, 0x00,       // Input (Data, Array, Absolute, Bit Field)
	0xC0              // End Collection
};

static struct hid_init_desc _hid_init_desc[] = {
	DESC(IDVENDOR, "0x046d", 8),
	DESC(IDPRODUCT, "0xc077", 8),
	DESC(BCDDEVICE, "0x7200", 8),
	DESC(BCDUSB, "0x0200", 8),
	DESC(BMAXPACKERSIZE0, "0x08", 4),
	DESC(SERIALNUMBER, "0x00", 4),
	DESC(MANUFACTURER, "Beagleboard", 11),
	DESC(PRODUCT, "BBB virtual input", 17),
	DESC(MAXPOWER, "0x01", 4),
	DESC(K_PROROCOL, "1", 4),
	DESC(K_SUBCLASS, "1", 1),
	DESC(K_REPORTLENGTH, "8", 1),
	DESC(K_REPORTDESC, &hid_report_keyboard, sizeof(hid_report_keyboard)),
	DESC(M_PROROCOL, "2", 1),
	DESC(M_SUBCLASS, "1", 1),
	DESC(M_REPORTLENGTH, "6", 1),
	DESC(M_REPORTDESC, &hid_report_mouse, sizeof(hid_report_mouse)),
	DESC(CONFIGURATION, "Conf 1", 6),
};

static int hid_f_write(char *path, const void *ptr, size_t size) {
	FILE *pFile;
	pFile = fopen(path, WO);
	if (pFile < 0) {
		printf("failed to open %s \n", path);
		return -1;
	}
	fwrite(ptr, size, 1, pFile);
	
	fclose(pFile);
	return 0;
}

int usb_hid_gadget_init(void) {
	int rc;
	int i = 0;
	int nr_set = ARRAY_SIZE(_hid_init_desc);
	struct hid_init_desc *desc = _hid_init_desc;
	struct stat st = {0};

	if (stat("/sys/kernel/config/usb_gadget", &st) >= 0) {

		hid_f_write(UDC, "\n", 1);

		if (stat("/sys/kernel/config/usb_gadget/g_multi", &st) == -1) {
			mkdir("/sys/kernel/config/usb_gadget/g_multi", 0755);
			mkdir("/sys/kernel/config/usb_gadget/g_multi/strings/0x409", 0755);
			mkdir("/sys/kernel/config/usb_gadget/g_multi/configs/c.1", 0755);
			mkdir("/sys/kernel/config/usb_gadget/g_multi/configs/c.1/strings/0x409", 0755);
		}

		if (stat("/sys/kernel/config/usb_gadget/g_multi/functions", &st) == -1) {
			mkdir("/sys/kernel/config/usb_gadget/g_multi/functions", 0755);
		}

		if (stat(USB0, &st) == -1) {
			mkdir(USB0, 0755);
		}

		if (stat(USB1, &st) == -1) {
			mkdir(USB1, 0755);
		}

		for (i = 0; i < nr_set; i++) {
			hid_f_write(desc->path, desc->ptr, desc->size);
			desc++;
		}

		rc = symlink(USB0, CONF0);
		rc = symlink(USB1, CONF1);

		hid_f_write(UDC, USB_DEV_NAME, strlen(USB_DEV_NAME));
	}

	return 0;
}

void usb_hid_gadget_remove(void) {
	struct stat st = {0};
	if (stat("/sys/kernel/config/usb_gadget", &st) >= 0) {
		hid_f_write(UDC, "", strlen(USB_DEV_NAME));

		remove(CONF0);
		remove(CONF1);

		remove(USB0);
		remove(USB1);

		hid_f_write(UDC, USB_DEV_NAME, strlen(USB_DEV_NAME));
	}
}
