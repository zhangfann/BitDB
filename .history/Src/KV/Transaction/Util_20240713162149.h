#pragma once

#include <vector>
#include <cstdint>

class Bound{
public:
    enum class Type{Included, Excluded, Unbounded};

    Bound(Type t, const std::vector<uint8_t>& v): type_(t), value_(v) {}

    Bound::Type get_type() const { return type_; }
    std::vector<uint8_t> get_value() const{ return value_;}

private:
    Type type_;
    std::vector<uint8_t> value_;
};