#include "transport.h"

// constructors
AsyncUDPServer::AsyncUDPServer(Receiver* receiver, unsigned short port):
    receiver(receiver), io_service(), socket(io_service, udp::endpoint(udp::v4(), port)) {
}


// member function implementation
void AsyncUDPServer::run() {
    try {
        this->io_service.run();
    } catch (const std::exception& e) {
        BOOST_LOG_TRIVIAL(fatal) << "AsyncUDPServer::run: io_service fails to run";
    }
    
    this->receive();
}


void AsyncUDPServer::send(const std::string& ip, unsigned short port, const std::string& data) {
    udp::endpoint endpoint(boost::asio::ip::address::from_string(ip), port);
    this->socket.async_send_to(boost::asio::buffer(data), endpoint,
        boost::bind(&AsyncUDPServer::handle_send, this, data,
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
} 

void AsyncUDPServer::receive() {
    this->socket.async_receive_from(boost::asio::buffer(this->recv_buffer), this->recv_endpoint,
        boost::bind(&AsyncUDPServer::handle_receive, this,
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
}

void AsyncUDPServer::handle_receive(const boost::system::error_code& error,
    std::size_t bytes_transferred) {
    
    if (!error || error == boost::asio::error::message_size) {
        // Call back to the receiver
        std::string data(this->recv_buffer.begin(), this->recv_buffer.end());
        receiver->receive(recv_endpoint.address().to_string(), recv_endpoint.port(), data);
    } else {
        BOOST_LOG_TRIVIAL(error) << "AsyncUDPServer::handle_receive: receive error, packet ignored";
    }
    this->receive();
}

void AsyncUDPServer::handle_send(const std::string& data,
    const boost::system::error_code& error,
    std::size_t bytes_transferred)  {
        
    if (error) {
        BOOST_LOG_TRIVIAL(error) << "AsyncUDPServer::handle_send: send error, packet might not be sent";
    }
}

//TCP server code, need debug
#ifdef NDEBUG
AsyncTCPServer::AsyncTCPServer(Receiver* receiver, unsigned short port):
    receiver(receiver), io_service(), socket(io_service, tcp::endpoint(tcp::v4(), port)) {
}

void AsyncTCPServer::run() {
    try {
        this->io_service.run();
    } catch (const std::exception& e) {
        BOOST_LOG_TRIVIAL(fatal) << "AsyncTCPServer::run: io_service fails to run";
    }
    
    this->receive();
}

void AsyncTCPServer::send(const std::string& ip, unsigned short port, const std::string& data) {
    tcp::endpoint endpoint(boost::asio::ip::address::from_string(ip), port);
    this->socket.async_send_to(boost::asio::buffer(data), endpoint,
        boost::bind(&AsyncTCPServer::handle_send, this, data,
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
} 

void AsyncTCPServer::receive() {
    this->socket.async_receive_from(boost::asio::buffer(this->recv_buffer), this->recv_endpoint,
        boost::bind(&AsyncTCPServer::handle_receive, this,
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
}

void AsyncTCPServer::handle_receive(const boost::system::error_code& error,
    std::size_t bytes_transferred) {
    if (!error || error == boost::asio::error::message_size) {
        // call back to the receiver
        std::string data(this->recv_buffer.begin(), this->recv_buffer.end());
        receiver->receive(recv_endpoint.address().to_string(), recv_endpoint.port(), data);
    } else {
        BOOST_LOG_TRIVIAL(error) << "AsyncTCPServer::handle_receive: receive error, packet ignored";
    }
    this->receive();
}

void AsyncTCPServer::handle_send(boost::shared_ptr<std::string> data,
    const boost::system::error_code& error,
    std::size_t bytes_transferred)  {
    
    if (error) {
        BOOST_LOG_TRIVIAL(error) << "AsyncTCPServer::handle_send: send error, packet might not be sent";
    }
}

#endif