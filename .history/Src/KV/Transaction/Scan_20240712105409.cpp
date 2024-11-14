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
        if(range.first.has_value()){
            if (range.first->type == Bound::Type::Excluded){
                start_type = Bound::Type::Excluded;
                auto key = Key(range.first->value, std::numeric_limits<uint64_t>::max()>);
                start_value = Key::encode(key);
            } else{
                start_type = Bound::Type::Included;
                auto key = Key(range.first->value, 0);
                start_value = Key::encode(key);
            }
        } else{
            start_type = Bound::Type::Included;
            auto key = Key({}, 0);
            start_value = Key::encode(key);
        }

        if(range.second.has_value()){
            if (range.second->type == Bound::Type::Excluded){

            } else{

            }
            
        }else{

        }

        return {{start_type, start_value}, {end_type, end_value}}
    }

private:
    StoragePtr storage_;
    Snapshot snapshot_;
    Bound bounds_;

};