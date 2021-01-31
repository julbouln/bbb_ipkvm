#include "ipkvm.h"

int s_interrupted = 0;

static void s_signal_handler(int signal_value) {
	s_interrupted = 1;
}

static void s_catch_signals(void) {
	struct sigaction action;
	action.sa_handler = s_signal_handler;
	action.sa_flags = 0;
	sigemptyset(&action.sa_mask);
	sigaction(SIGINT, &action, NULL);
	sigaction(SIGTERM, &action, NULL);
}

void ipkvm_init(ipkvm_t *ipkvm, int width, int height, int fps) {
	memset(ipkvm, 0, sizeof(ipkvm_t));
	ipkvm->video_fd = -1;
	ipkvm->keyboard_fd = -1;
	ipkvm->pointer_fd = -1;
	ipkvm->frame_rate = fps;
	ipkvm->need_capture = false;
	ipkvm->new_frame = false;
	ipkvm->width = width;
	ipkvm->height = height;
	ipkvm->audio_enabled = false;
}

void *ipkvm_video_run(void *data) {
	ipkvm_t *ipkvm = (ipkvm_t *) data;

	video_set_size(ipkvm, ipkvm->width, ipkvm->height);
	video_init_mmap(ipkvm);
	video_start_capturing(ipkvm);

	while (!s_interrupted) {
		if (ipkvm->need_capture) {
			video_capture(ipkvm);
		} else {
			// do not capture if there is no client
			usleep(30000);
		}
	}

	video_stop_capturing(ipkvm);
}

#define ALIGN_AUDIO(x) ((x/AUDIO_BUFFER_ALIGN + 1) * AUDIO_BUFFER_ALIGN)

int main(int argc, char **argv) {
	char *video_path = "/dev/video0";
	char *key_path = "/dev/hidg0";
	char *pointer_path = "/dev/hidg1";
	char *audio_dev = "hw:1,0";
	char *addr = "0.0.0.0";
	unsigned int width = 1280;
	unsigned int height = 720;
	int framerate = 30;
	s_catch_signals();

	ipkvm_t *ipkvm;
	ipkvm = (ipkvm_t *) malloc(sizeof(ipkvm_t));

	int option;
	const char *opts = "f:k:p:v:a:x:y:h";
	struct option lopts[] = {{"framerate", required_argument, 0, 'f'},
		{"keyboard",  required_argument, 0, 'k'},
		{"mouse",     required_argument, 0, 'p'},
		{"video",     required_argument, 0, 'v'},
		{"width",     required_argument, 0, 'x'},
		{"height",    required_argument, 0, 'y'},
		{"bind",      required_argument, 0, 'b'},
		{"help",      no_argument,       0, 'h'},
		{0,           0,                 0, 0}};

	while ((option = getopt_long(argc, argv, opts, lopts, NULL)) != -1) {
		switch (option) {
			case 'f':
				framerate = (int) strtol(optarg, NULL, 0);
				if (framerate < 0 || framerate > 60)
					framerate = 30;
				break;
				exit(0);
			case 'k':
				key_path = optarg;
				break;
			case 'p':
				pointer_path = optarg;
				break;
			case 'v':
				video_path = optarg;
				break;
			case 'a':
				audio_dev = optarg;
				break;
			case 'x':
				width = (int) strtol(optarg, NULL, 0);
				break;
			case 'y':
				height = (int) strtol(optarg, NULL, 0);
				break;
			case '?':
			case 'h':
				fprintf(stderr, "Usage: %s [OPTIONS]\n", argv[0]);
				fprintf(stderr, "\t-h, --help\tthis help\n");
				fprintf(stderr, "\t-f, --framerate=FPS\tcapture FPS, default: 30\n");
				fprintf(stderr, "\t-v, --video=DEV\tvideo device, default: /dev/video0\n");
				fprintf(stderr, "\t-a, --audio=DEV\taudio device, default: hw:1,0\n");
				fprintf(stderr, "\t-k, --keyboard=DEV\tkeyboard hid device: default /dev/hidg0\n");
				fprintf(stderr, "\t-p, --mouse=DEV\tmouse hid device, default: /dev/hidg1\n");
				fprintf(stderr, "\t-x, --width=WIDTH\tscreen width, default: 1280\n");
				fprintf(stderr, "\t-y, --height=HEIGH\tscreen height, default: 720\n");
				fprintf(stderr, "\t-b, --bind=ADDR\tbind VNC server address, default: 0.0.0.0\n");
				//printUsage();
				exit(0);
			default:
				//
				break;
		}
	}

	usb_hid_gadget_init();
	ipkvm_init(ipkvm, width, height, framerate);

	server_open(ipkvm, addr);
	input_open(ipkvm, key_path, pointer_path);
	video_open(ipkvm, video_path);

#ifdef AUDIO_EXTENSION
	audio_open(ipkvm, audio_dev);
#endif

	pthread_t video_thread;
	pthread_create(&video_thread, NULL, ipkvm_video_run, ipkvm);

	 struct timeval t0, t1;
    
	while (!s_interrupted) {
	    gettimeofday(&t0, 0);

	    if(ipkvm->audio_enabled) {
            audio_capture(ipkvm);
        }
		server_process(ipkvm);

	    gettimeofday(&t1, 0);
		long elapsed = (t1.tv_sec-t0.tv_sec)*1000000 + t1.tv_usec-t0.tv_usec;

		// dynamic audio buffer size
	    ipkvm->audio_frames = ALIGN_AUDIO((AUDIO_RATE/1000)*(elapsed/1000) - 100);
	    if(ipkvm->audio_frames > AUDIO_BUFFER_FRAMES)
	    	ipkvm->audio_frames=AUDIO_BUFFER_FRAMES;
	}

	pthread_join(video_thread, NULL);

#ifdef AUDIO_EXTENSION
	audio_close(ipkvm);
#endif
	video_close(ipkvm);
	input_close(ipkvm);
	server_close(ipkvm);
	usb_hid_gadget_remove();

	return (0);
}
