#ifndef NET_SERVICE_H
#define NET_SERVICE_H
#include "BaseService.h"
#include "listener.h"

namespace net {

class Service : public BaseService {
 public:
    Service(const tcp::Address& addr, IServiceListener* listener);

    void setListener(IServiceListener* listener);
    IServiceListener* getListener();

 public:
    void open(const tcp::Address& addr) override;
    void run() override;
    void close() override;
};

}  // namespace net

#endif  // NET_SERVICE_H
