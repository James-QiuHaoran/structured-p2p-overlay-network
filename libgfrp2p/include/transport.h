#ifndef TRANSPORT_H
#define TRANSPORT_H

#include <string>
#include <array>
#include <queue>
#include <exception>
#include <iostream>
#include <thread>
#include <condition_variable>

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/log/trivial.hpp>

using boost::asio::ip::tcp;
using boost::asio::ip::udp;

template <typename T>
class AtomicQueue {

public:
    void enqueue_and_notify(const T&);

    T wait_for_dequeue();
    
private:
    std::queue<T> q;

    std::mutex buffer_mutex;
    std::condition_variable buffer_refreshed_cv;
};

template <typename T>
void AtomicQueue<T>::enqueue_and_notify(const T& t) {
    std::unique_lock<std::mutex> buffer_lock(buffer_mutex);
    this->q.push(t);
    buffer_refreshed_cv.notify_one();
    buffer_lock.unlock();
}

template <typename T>
T AtomicQueue<T>::wait_for_dequeue() {
    std::unique_lock<std::mutex> buffer_lock(buffer_mutex);
    while(this->q.empty()) {
        buffer_refreshed_cv.wait(buffer_lock);
    }
    T t = q.front();
    q.pop();
    buffer_lock.unlock();
    return t;
};


// Classes that implement Receiver can be registered to the server
// receive() will be called to handle received data
class Receiver: public std::enable_shared_from_this<Receiver> {
public:
    virtual void receive(const std::string& ip, unsigned short port, const std::string& data) = 0;

    // For safe destruction
    virtual ~Receiver() = 0;
};

using BufferItemType = std::tuple<std::string, unsigned short, std::string>;

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

    // For incoming packets to queue up and unblock the socket
    std::unique_ptr<AtomicQueue<BufferItemType>> buffer;
    // std::mutex buffer_mlock;
    // std::condition_variable buffer_cv;
    // std::queue<BufferItemType> buffer;
    // std::thread handler;

    /* Only one piece of incoming data is kept
     * If Receiver::receive() does not return promptly, packets may be ignored
     * TODO: implement queue */
    std::array<char, BUFFER_SIZE> recv_buffer;
    udp::endpoint recv_endpoint;

    void receive();

    // Boost server mechanism
    void handle_receive(const boost::system::error_code& error, std::size_t bytes_transferred);

    // Routine of the handling thread
    void handle();
};

#ifdef NDEBUG
class TCPBuffer {
private:

};

// A class that implements asynchronous TCP send and receive
class TCPConnection: public boost::enable_shared_from_this<TCPConnection> {
public:
    typedef boost::shared_ptr<TCPConnection> Pointer;

    static Pointer create(boost::asio::io_service& io_service);

    tcp::socket& get_socket();

    void start();
private:
    tcp::socket socket;
    std::string buffer;

    TCPConnection(boost::asio::io_service& io_service): socket(io_service){};

    void handle_write(const boost::system::error_code& error,
        size_t bytes_transferred);

};

class AsyncTCPServer {
public:
    // Constructor
    AsyncTCPServer(const std::shared_ptr<Receiver>& receiver, unsigned short port);
    
    // run the receive loop
    void run();

    // encapsulate low-level machanism
    void send(const std::string& ip, unsigned short port, const std::string& data);
    
private:
    std::shared_ptr<Receiver> receiver;

    boost::asio::io_service io_service;
    tcp::acceptor acceptor;

    void accept();
    void handle_accept(TCPConnection::Pointer connection,
        const boost::system::error_code& error);
};


#endif
#endif
