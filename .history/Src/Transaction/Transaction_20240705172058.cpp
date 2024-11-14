class Transaction{
    IStorage storage_;
    int id_;
    Mode mode_;
    Snapshot snapshot_;

    bool begin(IStorage storage, Mode mode);

    bool resume(IStorage storage, int id);

    bool commit();
    bool rollback();

    bool delete(vector<char> key);

    bool get(vector<char> key, vector<char> &value);
};