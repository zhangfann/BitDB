class MVCC{
    IStorage *storage;

    MVCC(){
        storage = new Storage();
    }
};