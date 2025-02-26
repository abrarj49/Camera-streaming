#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>
#include <cstring>
#include <sys/mman.h>

#define WIDTH 640
#define HEIGHT 480

struct buffer {
    void* start;
    size_t length;
};

int main() {
    int fd = open("/dev/video0", O_RDWR);
    if (fd < 0) {
        perror("Error opening video device");
        return 1;
    }

    struct v4l2_format fmt;
    memset(&fmt, 0, sizeof(fmt));
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width = WIDTH;
    fmt.fmt.pix.height = HEIGHT;
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;

    if (ioctl(fd, VIDIOC_S_FMT, &fmt) < 0) {
        perror("Error setting format");
        close(fd);
        return 1;
    }

    struct v4l2_requestbuffers req;
    memset(&req, 0, sizeof(req));
    req.count = 1;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;

    if (ioctl(fd, VIDIOC_REQBUFS, &req) < 0) {
        perror("Error requesting buffer");
        close(fd);
        return 1;
    }

    struct v4l2_buffer buf;
    memset(&buf, 0, sizeof(buf));
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    buf.index = 0;

    if (ioctl(fd, VIDIOC_QUERYBUF, &buf) < 0) {
        perror("Error querying buffer");
        close(fd);
        return 1;
    }

    buffer frame;
    frame.length = buf.length;
    frame.start = mmap(NULL, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, buf.m.offset);
    
    if (frame.start == MAP_FAILED) {
        perror("Error mapping buffer");
        close(fd);
        return 1;
    }

    if (ioctl(fd, VIDIOC_QBUF, &buf) < 0) {
        perror("Error queueing buffer");
        close(fd);
        return 1;
    }

    int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (ioctl(fd, VIDIOC_STREAMON, &type) < 0) {
        perror("Error starting streaming");
        close(fd);
        return 1;
    }

    while (true) {
        if (ioctl(fd, VIDIOC_DQBUF, &buf) < 0) {
            perror("Error dequeuing buffer");
            break;
        }

        std::cout.write((char*)frame.start, buf.bytesused);  // Output frame to stdout

        if (ioctl(fd, VIDIOC_QBUF, &buf) < 0) {
            perror("Error queueing buffer");
            break;
        }
    }

    ioctl(fd, VIDIOC_STREAMOFF, &type);
    munmap(frame.start, frame.length);
    close(fd);

    return 0;
}