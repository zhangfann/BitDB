#include "MVCC.h"

class MVCC{
    IStorage *storage_;

    MVCC(IStorage storage){
        storage_ = storage;
    }

    bool begin();

    bool begin_with_mode(Mode mode);

    bool resume(int id);
};

bool MVCC::begin(){
    Transaction::begin(storage_, Mode::ReadWrite);
    return true;
}

bool MVCC::begin_with_mode(Mode mode){
    Transaction::begin(storage_, mode);
    return true;
}

bool MVCC::resume(int id){
    Transaction::resume(storage_, id);
    return true;
}