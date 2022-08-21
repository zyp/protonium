#pragma once

#include <protonium/rpc_pb.h>

template <typename... Services>
struct Dispatcher {
    std::tuple<Services...> services;

    Dispatcher(Services... services) : services(services...) {}

    template <std::size_t I = 0>
    auto handle(RPCMessage& request, auto& response_transport) requires (I < sizeof...(Services)) {
        auto& service = std::get<I>(services);
        
        if(service.service_number == request.service) {
            return service.handle(request, response_transport);
        }

        return handle<I + 1>(request, response_transport);
    }

    template <std::size_t I = 0>
    auto handle(RPCMessage& request, auto& response_transport) requires (I >= sizeof...(Services)) {
        return response_transport.send_error();
    }
};
