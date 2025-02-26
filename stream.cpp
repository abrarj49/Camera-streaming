#include <iostream>
#include <cstring>
#include <cstdlib>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <netinet/in.h>
#include <linux/videodev2.h>

#define DEVICE "/dev/video0"
#define PORT 8080
#define WIDTH 640
#define HEIGHT 480

struct Buffer {
    void* start;
    size_t length;
};

int init_camera(int fd, Buffer& buffer) {
    v4l2_format format;
    format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    format.fmt.pix.width = WIDTH;
    format.fmt.pix.height = HEIGHT;
    format.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;
    format.fmt.pix.field = V4L2_FIELD_NONE;

    if (ioctl(fd, VIDIOC_S_FMT, &format) == -1) {
        perror("Error setting format");
        return -1;
    }

    v4l2_requestbuffers req;
    req.count = 1;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;

    if (ioctl(fd, VIDIOC_REQBUFS, &req) == -1) {
        perror("Error requesting buffer");
        return -1;
    }

    v4l2_buffer buf;
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    buf.index = 0;

    if (ioctl(fd, VIDIOC_QUERYBUF, &buf) == -1) {
        perror("Error querying buffer");
        return -1;
    }

    buffer.length = buf.length;
    buffer.start = mmap(NULL, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, buf.m.offset);

    if (buffer.start == MAP_FAILED) {
        perror("Error mapping buffer");
        return -1;
    }

    return 0;
}

void start_streaming(int client_socket, int camera_fd, Buffer& buffer) {
    const char* header =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: multipart/x-mixed-replace; boundary=frame\r\n\r\n";

    send(client_socket, header, strlen(header), 0);

    v4l2_buffer buf;
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    buf.index = 0;

    while (true) {
        ioctl(camera_fd, VIDIOC_QBUF, &buf);
        ioctl(camera_fd, VIDIOC_STREAMON, &buf.type);
        ioctl(camera_fd, VIDIOC_DQBUF, &buf);

        char boundary[] = "\r\n--frame\r\n"
                          "Content-Type: image/jpeg\r\n\r\n";
        send(client_socket, boundary, strlen(boundary), 0);
        send(client_socket, buffer.start, buf.bytesused, 0);
    }
}

int main() {
    int camera_fd = open(DEVICE, O_RDWR);
    if (camera_fd == -1) {
        perror("Error opening video device");
        return 1;
    }

    Buffer buffer;
    if (init_camera(camera_fd, buffer) == -1) {
        return 1;
    }

    int server_fd, client_socket;
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    bind(server_fd, (struct sockaddr*)&address, sizeof(address));
    listen(server_fd, 5);

    std::cout << "Server running at http://localhost:8080" << std::endl;

    while (true) {
        client_socket = accept(server_fd, (struct sockaddr*)&address, &addrlen);
        if (client_socket >= 0) {
            start_streaming(client_socket, camera_fd, buffer);
        }
    }

    close(server_fd);
    close(camera_fd);
    return 0;
}