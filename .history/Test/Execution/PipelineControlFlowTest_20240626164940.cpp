

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
