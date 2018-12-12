#ifndef TRANSPORT_H
#define TRANSPORT_H

#include <string>
#include <array>
#include <exception>
#include <iostream>

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/log/trivial.hpp>

using boost::asio::ip::tcp;
using boost::asio::ip::udp;

// Classes that implement Receiver can be registered to the server
// receive() will be called to handle received data
class Receiver: public std::enable_shared_from_this<Receiver> {
public:
    virtual void receive(const std::string& ip, unsigned short port, const std::string& data) = 0;

    // For safe destruction
    virtual ~Receiver() = 0;
};

// A class that implements asynchronous UDP send and receive 
class AsyncUDPServer {
public:
    static const std::size_t BUFFER_SIZE = 65536;

    // Constructor
    AsyncUDPServer(const std::shared_ptr<Receiver>& receiver, unsigned short port);

    // run the receive loop
    void run();

    // Encapsulate low-level mechanism
    void send(const std::string& ip, unsigned short port, const std::string& data);

private:
    std::shared_ptr<Receiver> receiver;

    boost::asio::io_service io_service;
    udp::socket socket;
    
    /* Only one piece of incoming data is kept
     * If Receiver::receive() does not return promptly, packets may be ignored
     * TODO: implement queue */
    std::array<char, BUFFER_SIZE> recv_buffer;
    udp::endpoint recv_endpoint;

    void receive();

    // boost server mechanism
    void handle_receive(const boost::system::error_code& error, std::size_t bytes_transferred);
    void handle_send(boost::shared_ptr<std::string> data, const boost::system::error_code& error, std::size_t bytes_transferred);
};

#ifdef NDEBUG
// A class that implements asynchronous TCP send and receive
class AsyncTCPServer {
public:
    // Constructor
    AsyncTCPServer(const std::shared_ptr<Receiver>& receiver, unsigned short port);
    
    // run the receive loop
    void run();

    // encapsulate low-level machanism
    void send(const std::string& ip, unsigned short port, const std::string& data);
    
private:
    Receiver* receiver;
    boost::asio::io_service io_service;
    tcp::socket socket;

    std::array<char, 65536> recv_buffer; 
    // boost::asio::streambuf receive_buffer;
    tcp::endpoint recv_endpoint;

    void receiv();

    // boost server mechanism
    void handle_receive(const boost::system::error_code& error, std::size_t bytes_transferred);
    void handle_send(boost::shared_ptr<std::string> data, const boost::system::error_code& error, std::size_t bytes_transferred);
};
#endif

#endif