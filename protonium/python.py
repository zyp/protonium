from types import MethodType
from inspect import isawaitable

from google.protobuf import symbol_database

from .rpc_pb2 import RPCMessage

sym_db = symbol_database.Default()

class ServiceBase:
    service_number = None

    def __init__(self, transport):
        assert self.service_number is not None
        self.transport = transport

class RPCMethod:
    def __init__(self, method, request_type, response_type):
        self.method = method
        self.request_type = sym_db.GetSymbol(request_type)
        self.response_type = sym_db.GetSymbol(response_type)
    
    def __get__(self, obj, objtype = None):
        if obj is None:
            return self

        return MethodType(self, obj)

    async def async_decode_response(self, service, buf):
        return self.decode_response(service, await buf)

    def decode_response(self, service, buf):
        response = RPCMessage.FromString(buf)
        assert response.service == service.service_number
        assert response.method == self.method
        return self.response_type.FromString(response.payload)


    def __call__(self, service, **kwargs):
        request = RPCMessage(
            service = service.service_number,
            method = self.method,
            payload = self.request_type(**kwargs).SerializeToString(),
        ).SerializeToString()

        buf = service.transport.call(request)

        if isawaitable(buf):
            return self.async_decode_response(service, buf)
        else:
            return self.decode_response(service, buf)
