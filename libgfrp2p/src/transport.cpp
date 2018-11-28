#include "transport.h"

AsyncUDPServer::AsyncUDPServer(Receiver* receiver, unsigned short port): 
    receivcer(receiver), io_context(), socket(io_context, udp::endpoint(udp::v4(), port)) {

}

void AsyncUDPServer::start() {
    try {
        this->io_context.run();
    catch (const std::exception& e) {
        BOOST_LOG_TRIVIAL(fatal) << "AsyncUDPServer::start: io_context fails to run";
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
