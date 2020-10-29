#ifndef ERROR_H
#define ERROR_H
#include <stdexcept>
#include "address.h"

namespace tcp
{
class Error : public std::runtime_error {
public: 
    using std::runtime_error::runtime_error;
};

class AddressError : public Error {
private:
    const Address _addr;
public:
    AddressError(const std::string& what, const Address& addr);

    Address address() const;
};

} // namespace tcp

#endif // ERROR_H