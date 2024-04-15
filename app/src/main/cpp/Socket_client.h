//
// Created by Simon on 25/01/2024.
//

#ifndef MATHIAS_SOCKET_CLIENT_H
#define MATHIAS_SOCKET_CLIENT_H

#include <opencv2/core.hpp>

class SocketClient {
public:
    SocketClient(const char* hostname, int port);
    void ConnectToServer();
    void SendImageDims(const int image_rows, const int image_cols);
    void SendImage(cv::Mat& image);
    void SendImageH264(uint8_t *outbuf, int size);
private:
    const char* hostname_;
    int port_;
    int pic_num_;
    int socket_fdesc_;
};

#endif //MATHIAS_SOCKET_CLIENT_H




