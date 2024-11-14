#pragma once

#include <vector>
#include <cstdint>

class Bound{
public:
    enum class Type{Included, Excluded, Unbounded};

    Bound(Type t, const std::vector<uint8_t>& v): type_(t), value_(v) {}

private:
    Type type_;
    std::vector<uint8_t> value_;
};