#include <string>
#include <array>
#include <exception>

#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include <boost/log/trivial.hpp>

using boost::asio::ip::udp;

// Classes that implement Receiver can be regietered to the server
// receive() will be called to handle received data
class Receiver {
public:
    virtual void receive(const std::string& ip, unsigned short port, const std::string& data) = 0;
}


// A class that implements asynchronous UDP send and receive 
class AsyncUDPServer {
public:
    // Constructor
    AsyncUDPServer(Receiver* receiver, unsigned short port);

    // run the receive loop
    void run();

    // Encapsulate low-level mechanism
    void send(const std::string& ip, unsigned short port, const std::string& data);


private:
    Receiver* receiver;

    boost::asio::io_context io_context;
    udp::socket socket;
    
    /* Only one piece of incoming data is kept
     * If Receiver::receive() does not return promptly, packets may be ignored
     * TODO: implement queue */
    std::array<char, 65536> recv_buffer;
    udp::endpoint recv_endpoint;

    void receiving();

    // boost server mechanism
    void handle_receive(const boost::system::error_code& error, std::size_t bytes_transferred);
    void handle_send(boost::shared_ptr<std::string> data, const boost::system::error_code& error, std::size_t bytes_transferred);

};


// TODO: Async TCP server to be defined