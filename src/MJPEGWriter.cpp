#include "MJPEGWriter.hpp"
#include <fstream>

int MJPEGWriter::_write(int sock, char *s, int len) {
  if (len < 1) {
    len = strlen(s);
  }
  {
    try {
      int retval = send(sock, s, len, 0x4000);
      return retval;
    } catch (int e) {
      cout << "An exception occurred. Exception Nr. " << e << '\n';
    }
  }
  return -1;
}

int MJPEGWriter::_read(int socket, char *buffer) {
  int result;
  result = recv(socket, buffer, 4096, MSG_PEEK);
  if (result < 0) {
    cout << "An exception occurred. Exception Nr. " << result << '\n';
    return result;
  }
  string s = buffer;
  buffer = (char *)s.substr(0, (int)result).c_str();
  return result;
}

void MJPEGWriter::Listener() {

  // send http header
  std::string header;
  header += "HTTP/1.0 200 OK\r\n";
  header += "Cache-Control: no-cache\r\n";
  header += "Pragma: no-cache\r\n";
  header += "Connection: close\r\n";
  header +=
      "Content-Type: multipart/x-mixed-replace; boundary=mjpegstream\r\n\r\n";
  const int header_size = header.size();
  char *header_data = (char *)header.data();
  fd_set rread;
  int maxfd;
  this->open();
  pthread_mutex_unlock(&mutex_writer);
  while (true) {
    rread = master;
    struct timeval to = {0, timeout};
    maxfd = sock + 1;
    if (sock == INVALID_SOCKET) {
      return;
    }
    int sel = select(maxfd, &rread, NULL, NULL, &to);
    if (sel > 0) {
      for (int s = 0; s < maxfd; s++) {
        if (FD_ISSET(s, &rread) && s == sock) {
          int addrlen = sizeof(struct sockaddr);
          struct sockaddr_in address = {0};
          int client =
              accept(sock, (struct sockaddr *)&address, (socklen_t *)&addrlen);
          if (client == SOCKET_ERROR) {
            cerr << "error : couldn't accept connection on sock " << sock
                 << " !" << endl;
            return;
          }
          maxfd = (maxfd > client ? maxfd : client);
          pthread_mutex_lock(&mutex_cout);
          cout << "new client " << client << endl;
          char headers[4096] = "\0";
          int readBytes = _read(client, headers);
          cout << headers;
          pthread_mutex_unlock(&mutex_cout);
          pthread_mutex_lock(&mutex_client);
          _write(client, header_data, header_size);
          clients.push_back(client);
          pthread_mutex_unlock(&mutex_client);
        }
      }
    }
    usleep(1000);
  }
}

void MJPEGWriter::Writer() {
  pthread_mutex_lock(&mutex_writer);
  pthread_mutex_unlock(&mutex_writer);
  const int milis2wait = 16666;
  while (this->isOpened()) {
    pthread_mutex_lock(&mutex_client);
    int num_connected_clients = clients.size();
    pthread_mutex_unlock(&mutex_client);
    if (!num_connected_clients) {
      usleep(milis2wait);
      continue;
    }
    pthread_t threads[NUM_CONNECTIONS];
    int count = 0;

    std::vector<uchar> outbuf;
    std::vector<int> params;
    params.push_back(IMWRITE_JPEG_QUALITY);
    params.push_back(quality);
    pthread_mutex_lock(&mutex_writer);
    imencode(".jpg", lastFrame, outbuf, params);
    pthread_mutex_unlock(&mutex_writer);
    int outlen = outbuf.size();

    pthread_mutex_lock(&mutex_client);
    std::vector<int>::iterator begin = clients.begin();
    std::vector<int>::iterator end = clients.end();
    pthread_mutex_unlock(&mutex_client);
    std::vector<clientPayload *> payloads;
    for (std::vector<int>::iterator it = begin; it != end; ++it, ++count) {
      if (count > NUM_CONNECTIONS)
        break;
      struct clientPayload *cp = new clientPayload(
          {(MJPEGWriter *)this, {outbuf.data(), outlen, *it}});
      payloads.push_back(cp);
      pthread_create(&threads[count], NULL, &MJPEGWriter::clientWrite_Helper,
                     cp);
    }
    for (; count > 0; count--) {
      pthread_join(threads[count - 1], NULL);
      delete payloads.at(count - 1);
    }
    usleep(milis2wait);
  }
}

void MJPEGWriter::ClientWrite(clientFrame &cf) {
  stringstream head;
  head << "--mjpegstream\r\nContent-Type: image/jpeg\r\nContent-Length: "
       << cf.outlen << "\r\n\r\n";
  string string_head = head.str();
  pthread_mutex_lock(&mutex_client);
  _write(cf.client, (char *)string_head.c_str(), string_head.size());
  int n = _write(cf.client, (char *)(cf.outbuf), cf.outlen);
  if (n < cf.outlen) {
    std::vector<int>::iterator it;
    it = find(clients.begin(), clients.end(), cf.client);
    if (it != clients.end()) {
      cerr << "kill client " << cf.client << endl;
      clients.erase(std::remove(clients.begin(), clients.end(), cf.client));
      ::shutdown(cf.client, 2);
    }
  }
  pthread_mutex_unlock(&mutex_client);
  pthread_exit(NULL);
}

MJPEGWriter::MJPEGWriter(int port)
    : sock(INVALID_SOCKET), timeout(TIMEOUT_M), quality(90), port(port) {
  signal(SIGPIPE, SIG_IGN);
  FD_ZERO(&master);
  // if (port)
  //     open(port);
}

MJPEGWriter::~MJPEGWriter() { release(); }

bool MJPEGWriter::release() {
  if (sock != INVALID_SOCKET)
    shutdown(sock, 2);
  sock = (INVALID_SOCKET);
  return false;
}

bool MJPEGWriter::open() {
  sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

  struct sockaddr_in address;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_family = AF_INET;
  address.sin_port = htons(port);
  if (::bind(sock, (struct sockaddr *)&address, sizeof(struct sockaddr_in)) ==
      SOCKET_ERROR) {
    cerr << "error : couldn't bind sock " << sock << " to port " << port << "!"
         << endl;
    return release();
  }
  if (listen(sock, NUM_CONNECTIONS) == SOCKET_ERROR) {
    cerr << "error : couldn't listen on sock " << sock << " on port " << port
         << " !" << endl;
    return release();
  }
  FD_SET(sock, &master);
  return true;
}

bool MJPEGWriter::isOpened() { return sock != INVALID_SOCKET; }

void MJPEGWriter::start() {
  pthread_mutex_lock(&mutex_writer);
  pthread_create(&thread_listen, NULL, this->listen_Helper, this);
  pthread_create(&thread_write, NULL, this->writer_Helper, this);
}

void MJPEGWriter::stop() {
  this->release();
  pthread_join(thread_listen, NULL);
  pthread_join(thread_write, NULL);
}

void MJPEGWriter::write(Mat frame) {
  pthread_mutex_lock(&mutex_writer);
  if (!frame.empty()) {
    lastFrame.release();
    lastFrame = frame.clone();
  }
  pthread_mutex_unlock(&mutex_writer);
}