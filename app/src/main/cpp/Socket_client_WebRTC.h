//
// Created by Simon on 26/02/2024.
//

#ifndef MATHIAS_SOCKET_CLIENT_WEBRTC_H
#define MATHIAS_SOCKET_CLIENT_WEBRTC_H

#include <opencv2/core.hpp>

using namespace cv;
using namespace std;

class SocketClientWebRTC {
public:
    SocketClientWebRTC(const char *hostname, int port);
    void ConnectToServer();
    void SendImageH264(uint8_t *outbuf, int size);

private:
    const char *hostname_;
    int port_;
    int pic_num_;
    int socket_fdesc_;

};





#endif //MATHIAS_SOCKET_CLIENT_WEBRTC_H
