/*!
    \file tcp_session.h
    \brief TCP session definition
    \author Ivan Shynkarenka
    \date 14.12.2016
    \copyright MIT License
*/

#ifndef CPPSERVER_ASIO_TCP_SESSION_H
#define CPPSERVER_ASIO_TCP_SESSION_H

#include "service.h"

#include "system/uuid.h"

namespace CppServer {
namespace Asio {

template <class TServer, class TSession>
class TCPServer;

//! TCP session
/*!
    TCP session is used to read and write data from the connected TCP client.

    Thread-safe.
*/
template <class TServer, class TSession>
class TCPSession : public std::enable_shared_from_this<TCPSession<TServer, TSession>>
{
    template <class TSomeServer, class TSomeSession>
    friend class TCPServer;

public:
    //! Initialize the session with a given server
    /*!
        \param server - Connected server
        \param socket - Connected socket
    */
    explicit TCPSession(std::shared_ptr<TCPServer<TServer, TSession>> server, asio::ip::tcp::socket&& socket);
    TCPSession(const TCPSession&) = delete;
    TCPSession(TCPSession&&) = default;
    virtual ~TCPSession() = default;

    TCPSession& operator=(const TCPSession&) = delete;
    TCPSession& operator=(TCPSession&&) = default;

    //! Get the session Id
    const CppCommon::UUID& id() const noexcept { return _id; }

    //! Get the Asio service
    std::shared_ptr<Service>& service() noexcept { return _server->service(); }
    //! Get the session server
    std::shared_ptr<TCPServer<TServer, TSession>>& server() noexcept { return _server; }
    //! Get the session socket
    asio::ip::tcp::socket& socket() noexcept { return _socket; }

    //! Get the number of bytes sent by this session
    uint64_t bytes_sent() const noexcept { return _bytes_sent; }
    //! Get the number of bytes received by this session
    uint64_t bytes_received() const noexcept { return _bytes_received; }

    //! Is the session connected?
    bool IsConnected() const noexcept { return _connected; }

    //! Disconnect the session
    /*!
        \return 'true' if the section was successfully disconnected, 'false' if the section is already disconnected
    */
    bool Disconnect() { return Disconnect(false); }

    //! Send data into the session
    /*!
        \param buffer - Buffer to send
        \param size - Buffer size
        \return Count of pending bytes in the send buffer
    */
    size_t Send(const void* buffer, size_t size);
    //! Send a text string into the session
    /*!
        \param text - Text string to send
        \return Count of pending bytes in the send buffer
    */
    size_t Send(const std::string& text) { return Send(text.data(), text.size()); }

protected:
    //! Handle session connected notification
    virtual void onConnected() {}
    //! Handle session disconnected notification
    virtual void onDisconnected() {}

    //! Handle buffer received notification
    /*!
        Notification is called when another chunk of buffer was received
        from the client.

        Default behavior is to handle all bytes from the received buffer.
        If you want to wait for some more bytes from the client return the
        size of the buffer you want to keep until another chunk is received.

        \param buffer - Received buffer
        \param size - Received buffer size
        \return Count of handled bytes
    */
    virtual size_t onReceived(const void* buffer, size_t size) { return size; }
    //! Handle buffer sent notification
    /*!
        Notification is called when another chunk of buffer was sent
        to the client.

        This handler could be used to send another buffer to the client
        for instance when the pending size is zero.

        \param sent - Size of sent buffer
        \param pending - Size of pending buffer
    */
    virtual void onSent(size_t sent, size_t pending) {}

    //! Handle error notification
    /*!
        \param error - Error code
        \param category - Error category
        \param message - Error message
    */
    virtual void onError(int error, const std::string& category, const std::string& message) {}

private:
    // Session Id
    CppCommon::UUID _id;
    // Session server & socket
    std::shared_ptr<TCPServer<TServer, TSession>> _server;
    asio::ip::tcp::socket _socket;
    std::atomic<bool> _connected;
    // Session statistic
    size_t _bytes_sent;
    size_t _bytes_received;
    // Receive & send buffers
    std::mutex _send_lock;
    std::vector<uint8_t> _recive_buffer;
    std::vector<uint8_t> _send_buffer;
    bool _reciving;
    bool _sending;

    static const size_t CHUNK = 8192;

    //! Connect the session
    void Connect();
    //! Disconnect the session
    /*!
        \param dispatch - Dispatch flag
        \return 'true' if the session was successfully disconnected, 'false' if the session is already disconnected
    */
    bool Disconnect(bool dispatch);

    //! Try to receive new data
    void TryReceive();
    //! Try to send pending data
    void TrySend();

    //! Clear receive & send buffers
    void ClearBuffers();
};

} // namespace Asio
} // namespace CppServer

#include "tcp_session.inl"

#endif // CPPSERVER_ASIO_TCP_SESSION_H
