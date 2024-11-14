

namespace starrocks::pipeline{

struct Counter{
    size_t pull_chunk_num;
    size_t push_chunk_num;
};

using CounterPtr = std::shared_ptr<Counter>;

void assert_counter(CounterPtr counter, size_t expected_pull_chunk_num, size_t expected_push_chunk_num){
    ASSERT_EQ(counter->pull_chunk_num, expected_pull_chunk_num);
    ASSERT_EQ(counter->push_chunk_num, expected_push_chunk_num);
}

class TestSourceOperator: public SourceOperator{
    public:
    TestSourceOperator(int32_t id, int32_t plan_node_id, size_t chunk_num, size_t chunk_size, CounterPtr counter)
        : SourceOperator(id, "test_source", plan_node_id), _counter(counter){
        for(size_t i = 0; i< chunk_num; ++i){
            _chunks.push_back(std::move(PipelineTestBase::_create_and_fill_chunk(chunk_size)));
        }
    }
    ~TestSourceOperator() override = default;

    bool has_output() const override { return _index < _chunks.size(); }
    bool is_finished() const override { return !has_output(); }
    void finish(RuntimeState* state) override {}
    Status push_chunk(RuntimeState* state, const vectorized::ChunkPtr& chunk) override ;
    StatusOr<vectorized::ChunkPtr> pull_chunk(RuntimeState* state) override ;

private:
    size_t _index = 0;
    std::vector<ChunkPtr> _chunks;

}

TEST_F(TestPipelineControlFlow, test_two_operatories) {
    CounterPtr sourceCounter = std::make_shared<Counter>();
    CounterPtr sinkCounter = std::make_shared<Counter>();

    _pipeline_builder =[=](){
        _pipelines.clear();

        OpFactories op_factories;
        op_factories.push_back(std::make_shared<TestSourceOperatorFactory>(1,1,1,1, sourceCounter));
        op_factories.push_back(std::make_shared<TestSinkOperatorFactory>(2,2,sinkCounter));

        _pipelines.push_back(std::make_shared<Pipeline>(1, op_factories));
    };

    start_test();

    ASSERT_TRUE(_fragment_future.wait_for(std::chrono::seconds(3)) == std::future_status::ready);
    assert_counter(sourceCounter, 1, 0);
    assert_counter(sinkCounter, 0, 1);
}

TEST_F(TestPipelineControlFlow, test_three_operatories){
    CounterPtr sourceCounter = std::make_shared<Counter>();
    CounterPtr normalCounter = std::make_shared<Counter>();
    CounterPtr sinkCounter = std::make_shared<Counter>();

    _pipeline_builder = [=](){
        _pipelines.clear();

        OpFactories op_factories;
        op_factories.push_back(std::make_shared<TestSourceOperatorFactory>(1,1,1,1, sourceCounter));
        op_factories.push_back(std::make_shared<TestNormalOperatorFactory>(2,2, normalCounter));
        op_factories.push_back(std::make_shared<TestSinkOperatorFactory>(3,3, sinkCounter));

        _pipelines.push_back(std::make_shared<Pipeline>(1, op_factories));

        start_test();

        ASSERT_TRUE(_fragment_future.wait_for(std::chrono::seconds(3)) == std::future_status::ready);
        assert_counter(sourceCounter, 1, 0);
        assert_counter(normalCounter, 1, 1);
        assert_counter(sinkCounter, 0, 1);
    }
}

}
