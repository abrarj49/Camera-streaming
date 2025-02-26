🔥 What This Project Does (Big Picture)
Your project takes video from your webcam and shows it on a webpage using only low-level system calls (without fancy libraries like OpenCV). It’s like building your own webcam streaming app from scratch. 🎥➡️🌐

The project has 3 important files:
1️⃣ capture.cpp → Takes frames (images) from the webcam.
2️⃣ stream.cpp → Sends those frames to a browser over the internet (or your local network).
3️⃣ index.html → A web page that displays the video.

🛠 Understanding Each File in Detail
📸 1. capture.cpp (Getting Video from Webcam)
👉 This file talks directly to your webcam (/dev/video0) and captures video frames (images).

🛠 How It Works:
Step 1: Open the webcam device (/dev/video0).
Step 2: Ask the webcam to send video in MJPEG format.
Step 3: Create a memory buffer (a storage space in RAM) to hold the video frames.
Step 4: Keep getting new frames and print them to stdout (which means outputting raw image data).
🖥️ Simple Code Breakdown
cpp
Copy
Edit
int fd = open("/dev/video0", O_RDWR); 
Opens the webcam for reading and writing. If it fails, it prints an error.
cpp
Copy
Edit
struct v4l2_format fmt;  
fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;  
fmt.fmt.pix.width = WIDTH;  
fmt.fmt.pix.height = HEIGHT;  
fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;
ioctl(fd, VIDIOC_S_FMT, &fmt);
Tells the webcam:
"Hey, give me video!" 📸
"I want it in 640x480 size."
"Use the MJPEG format (compressed images like a video GIF)."
cpp
Copy
Edit
buffer frame;  
frame.start = mmap(NULL, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, buf.m.offset);
Maps the camera’s memory into our program, so we can directly access video frames.
cpp
Copy
Edit
std::cout.write((char*)frame.start, buf.bytesused);
Sends the raw video data to the output (which stream.cpp will later use).
🛠 What This File Does in One Sentence?
✅ It grabs video from your webcam and sends it out as raw data.

🌐 2. stream.cpp (Sending Video Over the Internet)
👉 This file creates a mini web server that streams video frames over port 8080.

🛠 How It Works:
Step 1: Opens the webcam (same way as capture.cpp).
Step 2: Starts a web server using Linux socket programming.
Step 3: Listens for people who visit http://localhost:8080/stream.
Step 4: Sends video frames to those users using MJPEG format (same as a video file).
🖥️ Simple Code Breakdown
cpp
Copy
Edit
int server_fd = socket(AF_INET, SOCK_STREAM, 0);
Creates a web server socket (basically, an internet door for communication).
cpp
Copy
Edit
bind(server_fd, (struct sockaddr*)&address, sizeof(address));
listen(server_fd, 5);
Binds the server to port 8080 so people can connect.
Listens for up to 5 users at a time.
cpp
Copy
Edit
client_socket = accept(server_fd, (struct sockaddr*)&address, &addrlen);
Waits for a web browser to connect (when someone visits http://localhost:8080/stream).
cpp
Copy
Edit
const char* header = "HTTP/1.1 200 OK\r\nContent-Type: multipart/x-mixed-replace; boundary=frame\r\n\r\n";
send(client_socket, header, strlen(header), 0);
Tells the browser:
"Hey, I’m sending you video!" 🎥
"This is MJPEG format, so expect a continuous stream of images."
cpp
Copy
Edit
send(client_socket, buffer.start, buf.bytesused, 0);
Sends video frames to the browser (it keeps doing this forever).
🛠 What This File Does in One Sentence?
✅ It acts like a YouTube Live server, sending video frames to anyone who visits http://localhost:8080/stream.

🎭 3. index.html (Displaying the Video in a Browser)
👉 This file is just a nice webpage that shows your webcam stream in a cool 3D style.

🛠 How It Works:
Step 1: Loads the webcam video using this line:

html
Copy
Edit
<img id="videoStream" src="http://localhost:8080/stream">
Tells the browser: "Hey, grab the video from http://localhost:8080/stream and show it!"
Step 2: Uses jQuery to hide/show the video when you click buttons.

html
Copy
Edit
<button onclick="startCamera()">Start Camera</button>
<button onclick="stopCamera()">Stop Camera</button>
Clicking “Start Camera” shows the video.
Clicking “Stop Camera” hides it.
Step 3: Adds a 3D effect so the video looks cool. 😎

🛠 What This File Does in One Sentence?
✅ It displays the webcam video on a stylish webpage with buttons for start/stop.

🎯 How to Explain This to Your Friends
Imagine you’re on a Zoom call, but instead of Zoom, you built your own video streaming app! 🎥🚀

capture.cpp: Asks the webcam for images.
stream.cpp: Acts like a mini YouTube Live, sending the video online.
index.html: Shows the video on a webpage with some cool effects.
🏁 How to Run the Project
1️⃣ Compile the files:

bash
Copy
Edit
g++ -o capture capture.cpp
g++ -o stream stream.cpp
2️⃣ Start the streaming server:

bash
Copy
Edit
./stream
3️⃣ Open the web interface:

Open index.html in a browser.
Or go to http://localhost:8080/stream directly.
🎉 Final Thoughts
This project is a mini webcam live-streaming system built with pure system calls.
No OpenCV, no external APIs – just raw, low-level Linux magic! 🏆
You can modify it (change resolution, port number, UI design, etc.).
