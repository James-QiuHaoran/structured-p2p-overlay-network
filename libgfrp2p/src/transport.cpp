#include "transport.h"

// Destructor function body must be provided even pure virtual
Receiver::~Receiver() { }

// constructors
AsyncUDPServer::AsyncUDPServer(const std::shared_ptr<Receiver>& receiver, unsigned short port):
    receiver(receiver), io_service(), work(new boost::asio::io_service::work(io_service)), socket(io_service, udp::endpoint(udp::v4(), port)),
    buffer(new AtomicQueue<BufferItemType>()) {

    BOOST_LOG_TRIVIAL(debug) << "AsyncUDPServer::(constructor): Initialization done";
}

// member function implementation
void AsyncUDPServer::run() {
    

    BOOST_LOG_TRIVIAL(debug) << "AsyncUDPServer::run: Starting handler thread";
    this->handler = std::thread(&AsyncUDPServer::handle, this);
    BOOST_LOG_TRIVIAL(debug) << "AsyncUDPServer::run: Starting io worker thread";
    this->io_worker = std::thread(&AsyncUDPServer::io_work, this);
    BOOST_LOG_TRIVIAL(debug) << "AsyncUDPServer::run: Threads started";

    // Add first async work
    this->receive();
}

void AsyncUDPServer::stop() {
    this->handler.join();

    this->io_service.stop();
    this->work.reset();
    this->io_worker.join();
}

void AsyncUDPServer::send(const std::string& ip, unsigned short port, const std::string& data) {
    udp::endpoint endpoint(boost::asio::ip::address::from_string(ip), port);
    boost::shared_ptr<std::string> message(new std::string(data));
    boost::system::error_code error;
    this->socket.async_send_to(boost::asio::buffer(*message), endpoint,
        boost::bind(&AsyncUDPServer::handle_send, this, message,
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
    // Continue receive asap
    this->receive();

    if (!error || error == boost::asio::error::message_size) {
        // Call back to the receiver
        std::string data(this->recv_buffer.begin(), this->recv_buffer.begin() + bytes_transferred);
        
        this->buffer->enqueue_and_notify(BufferItemType(recv_endpoint.address().to_string(), recv_endpoint.port(), data));

    } else {
        BOOST_LOG_TRIVIAL(error) << "AsyncUDPServer::handle_receive: receive error, packet ignored";
    }
    
}

void AsyncUDPServer::handle_send(boost::shared_ptr<std::string> data,
    const boost::system::error_code& error,
    std::size_t bytes_transferred)  {
        
    if (error) {
        BOOST_LOG_TRIVIAL(error) << "AsyncUDPServer::handle_send: send error, packet might not be sent";
    }
}


void AsyncUDPServer::handle() {
    // Copy and unlock immediately
    while (true) {
        auto front = this->buffer->wait_for_dequeue();

        this->receiver->receive(std::get<0>(front), std::get<1>(front), std::get<2>(front));
    }
}

void AsyncUDPServer::io_work() {
    for (;;) {
        try {
            BOOST_LOG_TRIVIAL(debug) << "AsyncUDPServer::io_work: Running io_service";
            this->io_service.run();
            BOOST_LOG_TRIVIAL(debug) << "AsyncUDPServer::io_work: io_service has stopped";
        } catch (const std::exception& e) {
            BOOST_LOG_TRIVIAL(fatal) << "AsyncUDPServer::io_work: io_service fails to run";
        }
    }
}

TCPConnection::Pointer TCPConnection::Create(boost::asio::io_service& io_service,
    const std::shared_ptr<AtomicQueue<BufferItemType>>& buffer) {
    return Pointer(new TCPConnection(io_service, buffer));
}

tcp::socket& TCPConnection::get_socket() { return this->socket; }

void TCPConnection::start() {
    this->read();
}

void TCPConnection::write(const std::string& data) {
    Header length = data.length();
    char header[sizeof(Header)];
    std::memcpy(header, &length, sizeof(Header));
    boost::shared_ptr<std::string> packet(new std::string(header, sizeof(Header)));
    *packet = *packet + data;
    
    boost::asio::async_write(this->socket, boost::asio::buffer(*packet),
        boost::bind(&TCPConnection::handle_write, this->shared_from_this(), packet,
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
}

TCPConnection::TCPConnection(boost::asio::io_service& io_service,
    const std::shared_ptr<AtomicQueue<BufferItemType>>& buffer):
    socket(io_service), resolver(io_service), buffer(buffer) { }

void TCPConnection::read() {
    boost::asio::async_read(this->socket, boost::asio::buffer(this->read_buffer),
        boost::bind(&TCPConnection::handle_read, this->shared_from_this(),
        boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));    
}

void TCPConnection::handle_read(const boost::system::error_code& error,
    std::size_t bytes_transferred) {
    std::string data(this->read_buffer.begin(), this->read_buffer.begin() + bytes_transferred);
    if (!error || error == boost::asio::error::eof || error == boost::asio::error::connection_reset) {
        if (this->datagram.empty()) {
            Header header;
            std::memcpy(&header, data.data(), sizeof(Header));
            this->total_length = header;
            this->datagram += data.substr(sizeof(Header));
        } else {
            this->datagram += data;
        }

        if (this->datagram.length() >= total_length || error == boost::asio::error::eof || error == boost::asio::error::connection_reset) {
            tcp::endpoint endpoint = this->socket.remote_endpoint();
            
            // Expected length or disconnection
            boost::system::error_code error_code;
            this->socket.shutdown(tcp::socket::shutdown_both, error_code);
            this->socket.close();
            
            // Push up whatever is got
            std::string ip = endpoint.address().to_string();
            unsigned short port = endpoint.port();
            this->buffer->enqueue_and_notify(BufferItemType(ip, port, this->datagram));
            // this->datagram.clear();
        } else {
            this->read();
        }
    } else  {
        BOOST_LOG_TRIVIAL(error) << "TCPConnection::handle_read: Read error " << error;
    }
}

void TCPConnection::handle_write(boost::shared_ptr<std::string> data,
    const boost::system::error_code& error,
    std::size_t bytes_transferred) {
    
    if (error) {
        BOOST_LOG_TRIVIAL(error) << "TCPConnection::handle_write: Write error " << error;
    } else {
        tcp::endpoint endpoint = this->socket.remote_endpoint();
        BOOST_LOG_TRIVIAL(debug) << "AsyncTCPServer::handle_write: Data sent to " 
            << endpoint.address().to_string() << ':' << std::to_string(endpoint.port());
    }

    boost::system::error_code error_code;
    this->socket.shutdown(tcp::socket::shutdown_both, error_code);
    this->socket.close();
}


AsyncTCPServer::AsyncTCPServer(const std::shared_ptr<Receiver>& receiver, unsigned short port):
    receiver(receiver), io_service(), work(new boost::asio::io_service::work(io_service)),
    acceptor(io_service, tcp::endpoint(tcp::v4(), port)), resolver(io_service), buffer(new AtomicQueue<BufferItemType>()) { }


void AsyncTCPServer::run() {
    BOOST_LOG_TRIVIAL(debug) << "AsyncTCPServer::run: Starting handler thread";
    this->handler = std::thread(&AsyncTCPServer::handle, this);
    BOOST_LOG_TRIVIAL(debug) << "AsyncTCPServer::run: Starting io worker thread";
    this->io_worker = std::thread(&AsyncTCPServer::io_work, this);
    BOOST_LOG_TRIVIAL(debug) << "AsyncTCPServer::run: Threads started";
    
    this->accept();
}

void AsyncTCPServer::send(const std::string& ip, unsigned short port, const std::string& data) {
    // if  found in current thread pool
    // auto conn_iter = this->tcp_connections.find(ip + ':' + std::to_string(port));
    // if (conn_iter != this->tcp_connections.end()) {
    //     this->tcp_connections.erase(conn_iter)
    //     conn_iter->second->write(data);
    // } else {
        BOOST_LOG_TRIVIAL(debug) << "AsyncTCPServer::send: Establishing connection with " << ip << ':' << port;
        std::shared_ptr<std::string> datagram(new std::string(data));
        TCPConnection::Pointer conn = TCPConnection::Create(this->io_service, this->buffer);
        tcp::resolver::query query(ip, std::to_string(port));
        this->resolver.async_resolve(query,
            boost::bind(&AsyncTCPServer::handle_resolve, this,
            boost::asio::placeholders::error,
            boost::asio::placeholders::iterator,
            conn, datagram));
    // }
    
}

void AsyncTCPServer::accept() {
    // Create a new connection on accept
    TCPConnection::Pointer connection = TCPConnection::Create(acceptor.get_io_service(), this->buffer);

    this->acceptor.async_accept(connection->get_socket(),
        boost::bind(&AsyncTCPServer::handle_accept, this, connection,
        boost::asio::placeholders::error));
}

void AsyncTCPServer::handle_accept(TCPConnection::Pointer connection,
    const boost::system::error_code& error) {
        
    this->accept();

    if (!error) {
        // tcp::endpoint endpoint = connection->get_socket().remote_endpoint();
        // std::string conn_id = endpoint.address().to_string() + ':' + std::to_string(endpoint.port());
        // this->tcp_connections[conn_id] = connection;

        connection->start();
    } else {
        BOOST_LOG_TRIVIAL(error) << "AsyncTCPServer::handle_accept: Accept error " << error;
    }

}

void AsyncTCPServer::handle_resolve(const boost::system::error_code& error,
    tcp::resolver::iterator endpoint_iterator,
    TCPConnection::Pointer conn,
    std::shared_ptr<std::string> datagram) {

    if (!error) {
        // Attempt a connection to the first endpoint in the list. Each endpoint
        // will be tried until we successfully establish a connection.
        conn->get_socket().close();
        tcp::endpoint endpoint = *endpoint_iterator;
        conn->get_socket().async_connect(endpoint,
                boost::bind(&AsyncTCPServer::handle_connect, this,
                boost::asio::placeholders::error, ++endpoint_iterator,
                conn, datagram));
    } else {
        BOOST_LOG_TRIVIAL(error) << "AsyncTCPServer::handle_resolve: Resolve error " << error;
    }
  }

void AsyncTCPServer::handle_connect(const boost::system::error_code& error,
    tcp::resolver::iterator endpoint_iterator,
    TCPConnection::Pointer conn,
    std::shared_ptr<std::string> datagram) {
    
    if (!error) {
        // The connection was successful. 
        // Add connection to stack
        // tcp::endpoint endpoint = conn->get_socket().remote_endpoint();
        // std::string conn_id = endpoint.address().to_string() + ":" + std::to_string(endpoint.port());
        // this->tcp_connections[conn_id] = conn;

        // Active side does not read
        // conn->start();
        
        // Send data
        conn->write(*datagram);
    } else if (endpoint_iterator != tcp::resolver::iterator()) {
        // The connection failed. Try the next endpoint in the list.
        conn->get_socket().close();
        tcp::endpoint endpoint = *endpoint_iterator;
        conn->get_socket().async_connect(endpoint,
            boost::bind(&AsyncTCPServer::handle_connect, this,
            boost::asio::placeholders::error, ++endpoint_iterator,
            conn, datagram));
    } else {
        BOOST_LOG_TRIVIAL(error) << "AsyncTCPServer::handle_connect: Connect error";
    }
  }

void AsyncTCPServer::handle() {
    while (true) {
        auto front = this->buffer->wait_for_dequeue();
        this->receiver->receive(std::get<0>(front), std::get<1>(front), std::get<2>(front));

        std::string conn_id = std::get<0>(front) + ":" + std::to_string(std::get<1>(front));
        // auto conn = tcp_connections.find(conn_id);
        // if (conn != tcp_connections.end()) {
        //     tcp_connections.erase(conn);
        // }
    }
}

void AsyncTCPServer::io_work() {
    for (;;) {
        try {
            BOOST_LOG_TRIVIAL(debug) << "AsyncTCPServer::io_work: Running io_service";
            this->io_service.run();
            BOOST_LOG_TRIVIAL(debug) << "AsyncTCPServer::io_work: io_service has stopped";
        } catch (const std::exception& e) {
            BOOST_LOG_TRIVIAL(error) << "AsyncTCPServer::io_work: io_service throws exception: " << e.what();
        }
    }
}

void AsyncTCPServer::stop() {
    this->handler.join();

    this->io_service.stop();
    this->work.reset();
    this->io_worker.join();
}