#include <sys/socket.h>
#include <arpa/inet.h>
#include <cstring>
#include <netinet/ip.h>
#include <unistd.h>
#include "error.h"
#include "descripter.h"
#include "connection.h"

using namespace tcp;

static void throw_error(const std::string& what) { throw tcp::Error(what); }

Connection::Connection() : c_addr({}, 0) { setSocket(); }
Connection::~Connection() { c_sockfd.close(); }
Connection::Connection(const Address& addr)
    : c_addr(addr)
{
    setSocket();
    connect(addr);
}
Connection::Connection(const int client_fd, const Address& addr)
    : c_addr(addr)
{
    c_sockfd.set_fd(client_fd);
}

Connection::Connection(Connection && other)
    : c_addr(std::move(other.c_addr))
    , c_sockfd(std::move(other.c_sockfd))
{}

void Connection::connect(const Address& addr)
{
    int error; 
    if(!c_sockfd.valid()) {
        setSocket();
    }

    sockaddr_in sock_addr{};
    sock_addr.sin_family = AF_INET;
    sock_addr.sin_port   = ::htons(addr.port());
    error = ::inet_aton(addr.address().data(), &sock_addr.sin_addr);
    if(error == 0)
        throw AddressError("Incorrect address!", addr);
    
    error = ::connect(c_sockfd.fd(), reinterpret_cast<sockaddr*>(&sock_addr), sizeof(sock_addr));
    if(error == -1)
        throw AddressError(std::strerror(errno), addr);
}

size_t Connection::write(const void* data, size_t len)
{
	ssize_t size = ::write(c_sockfd.fd(), data, len);
	if(size == -1)
		throw tcp::Error(std::strerror(errno));

	return static_cast<size_t> (size);
}

void   Connection::writeExact(const void* data, size_t len)
{
	size_t counter = 0u;
	const char* ch_data = static_cast<const char*> (data);
	while(counter < len) 
		counter += write(ch_data + counter, len - counter);
}
size_t Connection::read(void* data, size_t len)
{
	ssize_t size = ::read(c_sockfd.fd(), data, len);
	if (size == -1)
		throw_error(std::strerror(errno));
	
	return static_cast<size_t> (size);
}
void   Connection::readExact(void* data, size_t len)
{
	size_t counter = 0u;
	size_t current = 0u;
	char* ch_data = static_cast<char*> (data);
	while(counter < len) {
		current  = read(ch_data + counter, len - counter);
		if(current == 0u)
			throw_error("readExact pid failure");
		counter += current;
    }
}

void Connection::setSocket()
{
    c_sockfd.set_fd(::socket(AF_INET, SOCK_STREAM, 0));
    if(!c_sockfd.valid())
        throw_error(std::strerror(errno));
}

void Connection::close()
{
    c_sockfd.close();
}
void Connection::set_timeout(long sec, long usec) const
{
    timeval timeout = { sec, usec };
    if(setsockopt(c_sockfd.fd(), SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout)) == -1)
        throw_error(std::strerror(errno));
}

Connection& Connection::operator= (Connection && other)
{
    c_addr   = std::move(other.c_addr);
    c_sockfd = std::move(other.c_sockfd);
    return *this;
}
