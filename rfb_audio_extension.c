#include "ipkvm.h"

#define audioType 152

typedef struct {
	uint8_t type;
	uint16_t size;
} audio_msg_t;

rfbBool rfb_audio_enable(rfbClientPtr cl, void** data, int encoding)
{
	ipkvm_t *ipkvm = cl->screen->screenData;
	ipkvm->audio_enabled = true;
	return TRUE;
}

rfbBool rfb_audio_extension_msg_handler(struct _rfbClientRec* cl, void* data, const rfbClientToServerMsg* msg)
{
}

void rfb_audio_extension_send(rfbClientPtr client, char* buf, uint16_t size)
{
	audio_msg_t msg;

	msg.type = audioType;
	msg.size = Swap16IfLE(size);

	if(rfbWriteExact(client, (char*)&msg, sizeof(msg)) <= 0 ||
			rfbWriteExact(client, buf, size) <= 0) {
		fprintf(stderr, "audio: write error (%d: %s)", errno, strerror(errno));
	}
}

static int rfb_audio_encoding[] = { audioType, 0 };

rfbProtocolExtension audio_extension = {
	NULL,
	NULL,
	rfb_audio_encoding,
	rfb_audio_enable,
	rfb_audio_extension_msg_handler,
	NULL,
	NULL,
	NULL,
	NULL
};

void rfb_audio_extension_register() {
	rfbRegisterProtocolExtension(&audio_extension);
}