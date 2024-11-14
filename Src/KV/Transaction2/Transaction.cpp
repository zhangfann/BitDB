class Transaction{
    IStorage* storage_;
    int id_;
    Mode mode_;
    Snapshot snapshot_;

    Transaction* begin(IStorage* storage, Mode mode);

    Transaction* resume(IStorage* storage, int id);

    bool commit();
    bool rollback();

    bool del(vector<char> key);

    bool get(vector<char> key, vector<char> &value);

    bool scan(IRangeBounds range, Range& range_out);

    bool set(vector<char> key, vector<char> value);

    // 当value长度为0 意味删除这个key 
    bool write(vector<char> key, vector<char> value);


};