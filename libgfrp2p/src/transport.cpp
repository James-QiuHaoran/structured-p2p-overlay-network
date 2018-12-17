#include "transport.h"

// Destructor function body must be provided even pure virtual
Receiver::~Receiver() { }

// constructors
AsyncUDPServer::AsyncUDPServer(const std::shared_ptr<Receiver>& receiver, unsigned short port):
    receiver(receiver), io_service(), socket(io_service, udp::endpoint(udp::v4(), port)),
    buffer(new AtomicQueue<BufferItemType>()) { }

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
    boost::shared_ptr<std::string> message(new std::string(data));
    boost::system::error_code error;
    this->socket.send_to(boost::asio::buffer(*message), endpoint, 0, error);
    if (error) {
        BOOST_LOG_TRIVIAL(error) << "AsyncUDPServer::handle_send: send error, packet might not be sent";
    }
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
        
        this->buffer->enqueue_and_notify(BufferItemType(recv_endpoint.address().to_string(), recv_endpoint.port(), data));

    } else {
        BOOST_LOG_TRIVIAL(error) << "AsyncUDPServer::handle_receive: receive error, packet ignored";
    }
    this->receive();
}

void AsyncUDPServer::handle() {
    // Copy and unlock immediately
    auto front = this->buffer->wait_for_dequeue();

    this->receiver->receive(std::get<0>(front), std::get<1>(front), std::get<2>(front));
}

#ifdef NDEBUG

TCPConnection::Pointer TCPConnection::create(boost::asio::io_service& io_service) {
    return Pointer(new TCPConnection(io_service));
}

tco::socket& TCPConnection::get_socket() { return this->socket(); }

void TCPConnection::start() {
    message_ = make_daytime_string();

    boost::asio::async_write(this->socket, boost::asio::buffer(this->buffer),
        boost::bind(&TCPConnection::handle_write, shared_from_this(),
        boost::asio::placeholders::error,
        boost::asio::placeholders::bytes_transferred));
}

TCPConnection::TCPConnection(boost::asio::io_service& io_service): socket(io_service) { }

void handle_write(const boost::system::error_code& error,
    size_t bytes_transferred) {
    
    if (error) {
        BOOST_LOG_TRIVIAL(error) << "TCPConnection::handle_write: Write error";
    }
}


AsyncTCPServer::AsyncTCPServer(const std::shared_ptr<Receiver>& receiver, unsigned short port):
    receiver(receiver), io_service(), acceptor(io_service, tcp::endpoint((tcp::v4(), port)) { }


void AsyncTCPServer::run() {
    try {
        this->io_service.run();
    } catch (const std::exception& e) {
        BOOST_LOG_TRIVIAL(fatal) << "AsyncTCPServer::run: io_service fails to run";
    }
    
    this->accept();
}

void AsyncTCPServer::accept() {
    // Create a new connection on accept
    TCPConnection::Pointer connection = TCPConnection::create(acceptor.get_io_service());

    this->acceptor.async_accept(connection->get_socket(),
        boost::bind(&tcp_server::handle_accept, this, connection,
        boost::asio::placeholders::error));
}

void AsyncTCPServer::handle_accept(TCPConnection::Pointer connection,
    const boost::system::error_code& error) {
    if (!error) {
        connection->start();
    } else {
        BOOST_LOG_TRIVIAL(error) << "AsyncTCPServer::handle_accept: Accept error";
    }

    this->accept();
}

#endif
