#pragma once

#include "bit_cast.h"
#include "storage_class.h"
#include "scalar.h"
#include "message.h"
#include "oneof.h"
#include "repeated.h"
#include "dispatcher.h"

template <typename T>
T decode_message(std::span<uint8_t> buf) {
    T msg;

    auto it = std::begin(buf);
    auto end = std::end(buf);
    msg.decode(it, end);

    return msg;
}
