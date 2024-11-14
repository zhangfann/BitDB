class MVCC{
    IStorage *storage_;

    MVCC(IStorage storage){
        storage_ = storage;
    }

    bool begin();

    bool resume(int id);
};

bool MVCC::begin(){
    Transaction::begin(storage_, Mode::ReadWrite);
    return true;
}