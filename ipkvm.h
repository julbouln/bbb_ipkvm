#ifndef IPKVM_H
#define IPKVM_H

#include <stdio.h>
#include <signal.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <getopt.h>
#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include <linux/videodev2.h>
#include <poll.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>

#include <rfb/rfb.h>
#include <rfb/keysym.h>
#include <rfb/rfbregion.h>

#include <alsa/asoundlib.h>

#include "usb_hid_keys.h"

#define BUF_COUNT 5
#define KEY_REPORT_LENGTH 8
#define PTR_REPORT_LENGTH 6

#define VIDEO_DEFER_US 500
#define SERVER_DEFER_US 500

// MS2109 claim to be 96kHz mono, but actually are 48kHz stereo
// capture it as 96000/mono, and read it as 48000/stereo on client
#define AUDIO_EXTENSION
#define AUDIO_RATE 96000
#define AUDIO_CHANNELS 1
#define AUDIO_BUFFER_ALIGN 32
#define AUDIO_BUFFER_FRAMES 256


// STRUCTURES
typedef struct {
    void *data;
    size_t size;
    size_t payload;
    bool queued;
} buffer_t;

typedef struct {
    rfbScreenInfoPtr server;
    void *v_fb;
    unsigned int frame_rate;
    unsigned int width;
    unsigned int height;
    unsigned int last_buffer_idx;
    bool new_frame;
    bool need_capture;
    buffer_t buffers[BUF_COUNT];
    int keyboard_fd;
    uint8_t keyboard_mod;
    int pointer_fd;
    uint8_t keyboard_report[KEY_REPORT_LENGTH];
    uint8_t pointer_report[PTR_REPORT_LENGTH];
    int video_fd;
    bool audio_enabled;
    snd_pcm_t *audio_handle;
    char *audio_buffer;
    uint16_t audio_size;
    uint16_t audio_frames;
} ipkvm_t;

int usb_hid_gadget_init(void);
void usb_hid_gadget_remove(void);

void video_open(ipkvm_t *ipkvm, char *path);
void video_start_capturing(ipkvm_t *ipkvm);
void video_stop_capturing(ipkvm_t *ipkvm);
void video_capture(ipkvm_t *ipkvm);
int video_set_size(ipkvm_t *ipkvm, uint16_t width, uint16_t height);
void video_init_mmap(ipkvm_t *ipkvm);
void video_resize(ipkvm_t *ipkvm);
void video_close(ipkvm_t *ipkvm);

void server_resize(ipkvm_t *ipkvm);
void server_open(ipkvm_t *ipkvm, char *address);
void server_process(ipkvm_t *ipkvm);
void server_close(ipkvm_t *ipkvm);

void input_open(ipkvm_t *ipkvm, char *keyboard_path, char *pointer_path);
void input_keyboard_event(ipkvm_t *ipkvm, rfbBool down, rfbKeySym keysym);
void input_pointer_event(ipkvm_t *ipkvm, int buttonMask, int x, int y);
void input_close(ipkvm_t *ipkvm);

void audio_open(ipkvm_t *ipkvm, char *device_name);
void audio_capture(ipkvm_t *ipkvm);
void audio_close(ipkvm_t *ipkvm);

// custom RFB audio extension
void rfb_audio_extension_send(rfbClientPtr client, char* buf, uint16_t size);
void rfb_audio_extension_register();

#endif // IPKVM_H