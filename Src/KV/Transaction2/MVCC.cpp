#include "MVCC.h"

class MVCC{
    IStorage *storage_;

    MVCC(IStorage* storage){
        storage_ = storage;
    }

    Transaction*  begin();

    Transaction* begin_with_mode(Mode mode);

    Transaction* resume(int id);
};

Transaction* MVCC::begin(){
    return Transaction::begin(storage_, Mode::ReadWrite);
}

Transaction* MVCC::begin_with_mode(Mode mode){
    return Transaction::begin(storage_, mode);
}

Transaction* MVCC::resume(int id){
    return Transaction::resume(storage_, id);
}