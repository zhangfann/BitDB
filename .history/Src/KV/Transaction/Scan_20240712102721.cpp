#include <memory>
#include <mutex>
#include <optional>
#include <vector>

template<typename S>
class Scan{
public:
    using Storage = S;
    using BoundType = std::vector<uint8_t>;
    using Bound = std::optional<std::pair<BoundType, BoundType>>;
    using Snapshot = Snapshot;
    using StoragePtr = std::shared_ptr<Storage>;

    Scan(StoragePtr storage, Snapshot snapshot, Bound bounds)
        : storage_(std::move(storage)), snapshot_(std::move(snapshot)),
        bounds_(std::move(bounds)) {}

    static Bounds convert_range(const Bounds& range){
        
    }

private:
    StoragePtr storage_;
    Snapshot snapshot_;
    Bound bounds_;

};