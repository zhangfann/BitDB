#include <cstdint>
#include <iostream>

class Mode{
public:
    virtual ~Mode() = default;
    virtual bool mutable() const = 0;
}