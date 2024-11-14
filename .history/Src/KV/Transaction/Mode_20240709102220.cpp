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

    int get_version() const {
        return version_;
    }
}

int main() {
    TransactionMode* rw = new ReadWrite();
    TransactionMode* ro = new ReadOnly();
    TransactionMode* snap = new Snapshot(12345);

    std::cout << "ReadWrite is mutable: " << rw->isMutable() << std::endl;
    std::cout << "ReadOnly is mutable: " << ro->isMutable() << std::endl;
    std::cout << "Snapshot is mutable: " << snap->isMutable() << std::endl;

    delete rw;
    delete ro;
    delete snap;

    return 0;
}