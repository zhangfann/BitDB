class Transaction{
    IStorage storage_;
    int id_;
    Mode mode_;
    Snapshot snapshot_;

    bool begin(IStorage storage, Mode mode);

    
};