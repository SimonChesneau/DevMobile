//
// Created by Simon on 26/02/2024.
//
#include "webrtc/api/peer_connection_interface.h"
#include "webrtc/rtc_base/physical_socket_server.h"
#include "webrtc/rtc_base/ssladapter.h"
#include "webrtc/rtc_base/win32socketinit.h"
#include "Socket_client_WebRTC.h"


SocketClientWebRTC::SocketClientWebRTC(const char* hostname, int port) :
        hostname_ (hostname),
        port_(port),
        pic_num_(0),
        socket_fdesc_(0) {}

void SocketClientWebRTC::ConnectToServer(){

    rtc::InitializeSSL();

    worker_thread_ = rtc::Thread::Create();
    worker_thread_->Start();

    network_thread_ = rtc::Thread::Create();
    network_thread_->Start();

    signaling_thread_ = rtc::Thread::Create();
    signaling_thread_->Start();

    // Créer une PeerConnectionFactory
    peer_connection_factory_ =
            webrtc::CreatePeerConnectionFactory(worker_thread_, network_thread_, signaling_thread_, nullptr,
                                                nullptr, nullptr);

    // Créer une configuration pour RTCPeerConnection
    rtc::PeerConnectionInterface::RTCConfiguration rtc_config;
    rtc_config.sdp_semantics = webrtc::SdpSemantics::kUnifiedPlan;

    // Ajouter le serveur STUN avec l'adresse IP et le port spécifiés
    rtc_config.servers.push_back(webrtc::PeerConnectionInterface::IceServer("stun:" + std::string(hostname_) + ":" + std::to_string(port_)));

    // Créer RTCPeerConnection
    peer_connection_ =
            peer_connection_factory_->CreatePeerConnection(rtc_config, nullptr, nullptr, nullptr, nullptr);

    if (!peer_connection_) {
        std::cerr << "Erreur lors de la création de la connexion RTCPeerConnection\n";
        return;
    }
}

void SocketClientWebRTC::SendImageH264(uint8_t *outbuf, int size){
    if(send(socket_fdesc_, &size, sizeof(int), MSG_MORE) == -1){
        perror("Error sending size");
    }

    int num_bytes = send(socket_fdesc_, outbuf, size, 0);
    if(num_bytes < 0){
        perror("tqt");
    }
    printf("Sent %d bytes of %d-byte image to port %d\n",
           num_bytes, outbuf, port_);
}