class MVCC{
    IStorage *storage_;

    MVCC(IStorage storage){
        storage_ = storage;
    }
};