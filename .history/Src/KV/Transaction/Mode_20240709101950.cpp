#include <cstdint>
#include <iostream>

class Mode{
public:
    virtual ~Mode() = default;
    virtual bool mutable() const = 0;
}

class ReadOnly : public Mode{
public:
    bool mutable() const override {
        return false;
    }
}

class ReadWrite : public Mode{
public:
    bool mutable() const override {
        return true;
    }
}

class Snapshot: public Mode{
    int version_;

pubilc:
    Snapshot(int version) : version(version_) {}

    bool mutable() const override {
        return false;
    }
}