#include "ipkvm.h"

// VIDEO

static int xioctl(int fh, int request, void *arg) {
    int r;

    do {
        r = ioctl(fh, request, arg);
    } while (-1 == r && EINTR == errno);

    return r;
}

void video_open(ipkvm_t *ipkvm, char *path) {
    int rc;
    struct v4l2_capability cap;
    struct v4l2_crop crop;
    struct v4l2_format fmt;

    if (ipkvm->video_fd >= 0) {
        return;
    }

    ipkvm->video_fd = open(path, O_RDWR);

    if (ipkvm->video_fd < 0) {
        fprintf(stderr, "Failed to open video device path=%s : %s\n", path, strerror(errno));
        return;
    }

    memset(&cap, 0, sizeof(struct v4l2_capability));
    rc = xioctl(ipkvm->video_fd, VIDIOC_QUERYCAP, &cap);
    if (rc < 0) {
        fprintf(stderr, "Failed to query video device capabilities : %s\n", strerror(errno));
    }

    if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) ||
        !(cap.capabilities & V4L2_CAP_STREAMING)) {
        fprintf(stderr, "Video device doesn't support this application\n");
    }
}

void video_start_capturing(ipkvm_t *ipkvm) {
    int rc;
    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    for (int i = 0; i < BUF_COUNT; i++) {
        struct v4l2_buffer buf;

        memset(&buf, 0, sizeof(struct v4l2_buffer));
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;

        rc = xioctl(ipkvm->video_fd, VIDIOC_QBUF, &buf);
        if (rc < 0) {
            fprintf(stderr, "Failed to queue initial buffer : %s\n", strerror(errno));
        }
    }

    rc = xioctl(ipkvm->video_fd, VIDIOC_STREAMON, &type);
    if (rc) {
        fprintf(stderr, "Failed to start streaming : %s\n", strerror(errno));
    }
}

void video_stop_capturing(ipkvm_t *ipkvm) {
    int rc;
    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    rc = xioctl(ipkvm->video_fd, VIDIOC_STREAMOFF, &type);
    if (rc) {
        fprintf(stderr, "Failed to stop streaming : %s\n", strerror(errno));
    }

    for (int i = 0; i < BUF_COUNT; ++i) {
        if (ipkvm->buffers[i].data) {
            munmap(ipkvm->buffers[i].data, ipkvm->buffers[i].size);
            ipkvm->buffers[i].data = NULL;
            ipkvm->buffers[i].queued = false;
        }
    }
}

void video_capture(ipkvm_t *ipkvm) {
    int rc;
    struct v4l2_buffer buf;

    if (ipkvm->video_fd < 0) {
        return;
    }

    fd_set fds;
    struct timeval tv;

    FD_ZERO(&fds);
    FD_SET(ipkvm->video_fd, &fds);

    tv.tv_sec = 0;
    tv.tv_usec = VIDEO_DEFER_US;

    rc = select(ipkvm->video_fd + 1, &fds, NULL, NULL, &tv);

    if (rc > 0) {
        memset(&buf, 0, sizeof(struct v4l2_buffer));
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;

        rc = xioctl(ipkvm->video_fd, VIDIOC_DQBUF, &buf);

        if (rc < 0) {
            fprintf(stderr, "Failed to dequeue buffer : %s\n", strerror(errno));
        } else {
            ipkvm->buffers[buf.index].queued = false;

            char *data = (char *) ipkvm->buffers[buf.index].data;
            if (!(buf.flags & V4L2_BUF_FLAG_ERROR) && (data[0] & 0xff) == 0xff && (data[1] & 0xff) == 0xd8 &&
                buf.bytesused > 4) {
                /*fprintf(stderr, "Valid frame idx:%d byteused:%d/%d flags:%x 0:%x 1:%x\n",
                        buf.index, buf.bytesused, ipkvm->buffers[buf.index].size, buf.flags, data[0], data[1]);
                */
                ipkvm->last_buffer_idx = buf.index;
                ipkvm->buffers[ipkvm->last_buffer_idx].payload = buf.bytesused;

                // done with frame
                ipkvm->new_frame = true;
           } else {
                fprintf(stderr, "Drop invalid frame idx:%d byteused:%d/%ld flags:%x 0:%x 1:%x\n",
                        buf.index, buf.bytesused, ipkvm->buffers[buf.index].size, buf.flags, data[0], data[1]);
                ipkvm->buffers[buf.index].payload = 0;
            }
        }

        for (unsigned int i = 0; i < BUF_COUNT; ++i) {
            if (i == (unsigned int) ipkvm->last_buffer_idx) {
                continue;
            }

            if (!ipkvm->buffers[i].queued) {
                memset(&buf, 0, sizeof(struct v4l2_buffer));
                buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                buf.memory = V4L2_MEMORY_MMAP;
                buf.index = i;

                rc = xioctl(ipkvm->video_fd, VIDIOC_QBUF, &buf);
                if (rc < 0) {
                    fprintf(stderr, "Failed to queue buffer : %s\n", strerror(errno));
                } else {
                    ipkvm->buffers[i].queued = true;
                }
            }
        }
    }
}

int video_set_size(ipkvm_t *ipkvm, uint16_t width, uint16_t height) {
    int rc;
    struct v4l2_format fmt;
    struct v4l2_streamparm sparm;

    if (ipkvm->video_fd < 0) {
        return 0;
    }

    memset(&fmt, 0, sizeof(struct v4l2_format));

    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width = width;
    fmt.fmt.pix.height = height;
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;
    fmt.fmt.pix.field = V4L2_FIELD_INTERLACED;

    rc = xioctl(ipkvm->video_fd, VIDIOC_S_FMT, &fmt);

    if (rc < 0) {
        fprintf(stderr, "Failed to force video size : %s\n", strerror(errno));
        return 0;
    }

    ipkvm->height = height;
    ipkvm->width = width;

    memset(&fmt, 0, sizeof(struct v4l2_format));
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    rc = xioctl(ipkvm->video_fd, VIDIOC_G_FMT, &fmt);

    if (rc < 0) {
        fprintf(stderr, "Failed to query video device format : %s\n", strerror(errno));
        return 0;
    }

    ipkvm->height = fmt.fmt.pix.height;
    ipkvm->width = fmt.fmt.pix.width;

    printf("set size to %dx%d\n", ipkvm->width, ipkvm->height);

    memset(&sparm, 0, sizeof(struct v4l2_streamparm));
    sparm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    rc = xioctl(ipkvm->video_fd, VIDIOC_G_PARM, &sparm);
    if (rc < 0) {
        fprintf(stderr, "Unable to query HW FPS changing : %s\n", strerror(errno));
    }

    if (!(sparm.parm.capture.capability & V4L2_CAP_TIMEPERFRAME)) {
        fprintf(stderr, "Changing HW FPS is not supported\n");
    }

    memset(&sparm, 0, sizeof(struct v4l2_streamparm));
    sparm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    sparm.parm.capture.timeperframe.numerator = 1;
    sparm.parm.capture.timeperframe.denominator = ipkvm->frame_rate;
    rc = xioctl(ipkvm->video_fd, VIDIOC_S_PARM, &sparm);
    if (rc < 0) {
        fprintf(stderr, "Failed to set video device frame rate : %s\n", strerror(errno));
    }

    return 1;
}

void video_init_mmap(ipkvm_t *ipkvm) {
    int rc;
    unsigned int i;
    struct v4l2_requestbuffers req;

    if (ipkvm->video_fd < 0) {
        return;
    }

    memset(&req, 0, sizeof(struct v4l2_requestbuffers));
    req.count = 0;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;
    rc = xioctl(ipkvm->video_fd, VIDIOC_REQBUFS, &req);
    if (rc < 0) {
        fprintf(stderr, "Failed to zero streaming buffers : %s\n", strerror(errno));
    }

    memset(&req, 0, sizeof(struct v4l2_requestbuffers));
    req.count = BUF_COUNT;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;

    rc = xioctl(ipkvm->video_fd, VIDIOC_REQBUFS, &req);
    if (rc < 0 || req.count < 2) {
        fprintf(stderr, "Failed to request streaming buffers : %s\n", strerror(errno));
    }

    for (i = 0; i < BUF_COUNT; ++i) {
        struct v4l2_buffer buf;

        memset(&buf, 0, sizeof(struct v4l2_buffer));
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;

        rc = xioctl(ipkvm->video_fd, VIDIOC_QUERYBUF, &buf);
        if (rc < 0) {
            fprintf(stderr, "Failed to query buffer : %s\n", strerror(errno));
        }
        ipkvm->buffers[i].data = mmap(NULL, buf.length, PROT_READ | PROT_WRITE,
                                      MAP_SHARED, ipkvm->video_fd, buf.m.offset);
        if (ipkvm->buffers[i].data == MAP_FAILED) {
            fprintf(stderr, "Failed to mmap buffer : %s\n", strerror(errno));
        }
        ipkvm->buffers[i].size = buf.length;
        ipkvm->buffers[i].queued = true;
    }
}

void video_resize(ipkvm_t *ipkvm) {
    video_stop_capturing(ipkvm);
    video_set_size(ipkvm, ipkvm->width, ipkvm->height);
    video_init_mmap(ipkvm);
    video_start_capturing(ipkvm);
}

void video_close(ipkvm_t *ipkvm) {
    int rc;
    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    close(ipkvm->video_fd);
    ipkvm->video_fd = -1;
}
