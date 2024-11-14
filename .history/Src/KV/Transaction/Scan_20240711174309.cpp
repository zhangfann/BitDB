template<typename S>
class Scan{
public:
    using Storage = S;
    using BoundType = std::vector<uint8_t>;
    using Bound = std::optional<std::pair<BoundType, BoundType>>;
    using Snapshot = Snapshot;
    using StoragePtr = std::shared_ptr<Storage>;
};