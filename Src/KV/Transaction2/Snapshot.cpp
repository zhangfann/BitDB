class Snapshot{
    int version;
    set<int> invisible;

    bool take(IStorage session, int version);

    bool restore(IStorage session, int version);

    bool is_visible(int version);
};

