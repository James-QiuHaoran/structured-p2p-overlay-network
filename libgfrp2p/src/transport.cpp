#include "transport.h"

AsyncUDPServer::AsyncUDPServer(Receiver* receiver, unsigned short port): 
    receivcer(receiver), io_context(), socket(io_context, udp::endpoint(udp::v4(), port)) { }

void AsyncUDPServer::start_receiving() {
    this->socket.async_receive_from(boost::asio::buffer(this->recv_buffer), this->recv_endpoint,
        boost::bind(&AsyncUDPServer::handle_receive, this,
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
}

void AsyncUDPServer::send(const std::string& ip, unsigned short port, const std::string& data) {
    udp::endpoint endpoint(boost::asio::ip::address::from_string(ip), port);
    this->socket.async_send_to(boost::asio::buffer(data), endpoint,
        boost::bind(&AsyncUDPServer::handle_send, this, data,
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
} 

void AsyncUDPServer::continue_receiving() {
    start_receiving()
}

void AsyncUDPServer::handle_receive(const boost::system::error_code& error,
    std::size_t bytes_transferred) {
    
    // Call back to the receiver
    std::string data(this->recv_buffer.begin(), this->recv_buffer.end());
    receiver->receive(recv_endpoint.address().to_string(), recv_endpoint.port(), data);
    
    this->continue_receiving();
}

void AsyncUDPServer::handle_send(const std::string& data,
    const boost::system::error_code& error,
    std::size_t bytes_transferred)  { }
