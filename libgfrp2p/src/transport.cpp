#include "transport.h"

// Destructor function body must be provided even pure virtual
Receiver::~Receiver() { }

AsyncUDPServer::AsyncUDPServer(const std::shared_ptr<Receiver>& receiver, unsigned short port):
    receiver(receiver), buffer(new AtomicQueue<BufferItemType>()) {
    
    std::cout << "INFO: AsyncUDPServer::(constructor): Initializing to port " << port << " on any interface" << std::endl;

    udp_socket_id_ = socket(AF_INET, SOCK_DGRAM, 0);
    
    struct sockaddr_in udp_addr_;
    udp_addr_.sin_family = AF_INET;
    udp_addr_.sin_port = htons(port);
    udp_addr_.sin_addr.s_addr = INADDR_ANY;
    bzero(&(udp_addr_.sin_zero),8);

    int32_t res_bind = bind(udp_socket_id_, (struct sockaddr *)&udp_addr_, sizeof(struct sockaddr));
    if (res_bind == -1) {
        std::cerr << "ERROR: AsyncUDPServer::(constructor): Failed to bind: " << strerror(errno) << std::endl;
    }

    int32_t multi_use = 1;
    int32_t res_multi_use = setsockopt(udp_socket_id_,  SOL_SOCKET,  SO_REUSEADDR,  &multi_use,  sizeof(multi_use));  
    if(res_multi_use == -1) {
        std::cerr << "ERROR: AsyncUDPServer::(constructor): Failed to set socket address reuse option: " << strerror(errno) << std::endl;
    }
}

void AsyncUDPServer::run() {
    #ifndef NDEBUG
    std::cout << "DEBUG: AsyncUDPServer::run: Starting buffer handler thread";
    #endif
    this->buffer_handler = std::thread(&AsyncUDPServer::handle, this);
    #ifndef NDEBUG
    std::cout << "DEBUG: AsyncUDPServer::run: Starting receive worker thread";
    #endif
    this->receive_worker = std::thread(&AsyncUDPServer::receive, this);
    #ifndef NDEBUG
    std::cout << "DEBUG: AsyncUDPServer::run: Threads started";
    #endif
}

void AsyncUDPServer::stop() {
    this->receive_worker.join();
    this->buffer_handler.join();
}

void AsyncUDPServer::send(const std::string& ip, unsigned short port, const std::string& data) {
    struct sockaddr_in udp_client_addr_;
    udp_client_addr_.sin_family = AF_INET;
    udp_client_addr_.sin_port = htons(port);
    udp_client_addr_.sin_addr.s_addr = inet_addr(ip.c_str());
    bzero(&(udp_client_addr_.sin_zero),8);
    int32_t res_sendto = sendto(this->udp_socket_id_, data.data(), data.size(), 0, (struct sockaddr *)&udp_client_addr_, sizeof(struct sockaddr));
    if(res_sendto == -1) {
        std::cerr << "ERROR: AsyncUDPServer::send: Failed to send: " << strerror(errno) << std::endl;
    } else if (res_sendto < data.size()) {
        std::cerr << "WARNING: AsyncUDPServer::send: Only " << res_sendto << " out of "  << data.size() << " is sent" << std::endl;
    }
}


void AsyncUDPServer::receive() {
    int32_t addr_len = sizeof(struct sockaddr);
    char recv_data[BUFFER_SIZE];
    struct sockaddr_in udp_client_addr_;
    int32_t bytes_read;

    std::string ip;
    unsigned short port;
    std::string data;

    for (;;) {
        bytes_read = recvfrom(this->udp_socket_id_, recv_data, BUFFER_SIZE, 0, (struct sockaddr *)&udp_client_addr_, (socklen_t*)&addr_len);
        if (bytes_read == -1) {
            std::cerr << "ERROR: AsyncUDPServer::receive: Failed to recvfrom: " << strerror(errno) << std::endl;
            return;
        }
        
        ip = std::string(inet_ntoa(udp_client_addr_.sin_addr));
        port = ntohs(udp_client_addr_.sin_port);
        data = std::string(recv_data, bytes_read);

        this->buffer->enqueue_and_notify(BufferItemType(ip, port, data));

    }
}

void AsyncUDPServer::handle() {
    // Copy and unlock immediately
    for (;;) {
        auto front = this->buffer->wait_for_dequeue();
        this->receiver->receive(std::get<0>(front), std::get<1>(front), std::get<2>(front));
    }
}

AsyncUDPServer::~AsyncUDPServer() {
    close(udp_socket_id_);
}

