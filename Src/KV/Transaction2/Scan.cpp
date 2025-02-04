#include <memory>
#include <mutex>
#include <optional>
#include <vector>



template<typename S>
class Scan{
public:
    using Storage = S;
    // using BoundType = std::vector<uint8_t>;
    using RangeBounds = std::pair<Bound, Bound>;
    using Snapshot = Snapshot;
    using StoragePtr = std::shared_ptr<Storage>;



    Scan(StoragePtr storage, Snapshot snapshot, RangeBounds range)
        : storage_(std::move(storage)), snapshot_(std::move(snapshot)),
        next_candidate_(std::nullopt), next_back_returned_(std::nullopt) {
            bounds_ = convert_range(range);
            
        }

    static RangeBounds convert_range(const RangeBounds& range){
        Bound::Type start_type, end_type;
        BoundType start_value, end_value;

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
                end_type = Bound::Type::Excluded;
                auto key = Key(range.second->value, 0);
                end_value = Key::encode(key);
            } else{
                end_type = Bound::Type::Included;
                auto key = Key(range.second->value, std::numeric_limits<uint64_t>::max());
                end_value = Key::encode(key);
            }
            
        }else{
            end_type = Bound::Type::Unbounded;
            end_value = {};
        }

        return {{start_type, start_value}, {end_type, end_value}}
    }

    Result try_next(){
        auto session = storage_.read();
        if (!session){
            return std::nullopt;
        }

        auto range = session->scan(bounds_);
        // 遍历bounds_范围内的值
        while(auto item = range.next()){
            if(!item){
                return std::nullopt;
            }

            // 解析出 kv
            auto [k, v] = *item;
            bounds_.lower_bound = Bound::Excluded(k);

            // 从k中解析出key version
            auto [key, version] = Key::decode(k);
            if(!is_visible(version)){
                continue;
            }

            Result ret;
            if (next_candidate_ && next_candidate_->first != key){
                ret = *next_candidate_;
                next_candidate_ = {};
            }
            next_candidate_ = {key, deserialize(v)};

            if (ret.has_value()){
                return ret;
            }
        }

        if(next_candidate_){
            auto result = *next_candidate_;
            next_candidate_ = {};
            return result;
        }
        return std::nullopt;
    }

private:
    StoragePtr storage_;
    Snapshot snapshot_;
    RangeBounds bounds_;
    std::optional<std::vector<uint8_t>,std::optional<std::vector<uint8_t>>> next_candidate_;
    std::optional<std::vector<uint8_t>> next_back_returned_;

};