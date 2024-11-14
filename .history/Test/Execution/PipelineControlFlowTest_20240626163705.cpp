

TEST_F(TestPipelineControlFlow, test_two_operatories) {
    CounterPtr sourceCounter = std::make_shared<Counter>();
    CounterPtr sinkCounter = std::make_shared<Counter>();

    _pipeline_builder =[=](){
        _pipelines.clear();

        OpFactories op_factories;
        op_factories.push_back(std::make_shared<TestSourceOperatorFactory>(1,1,1,1, sourceCounter));
        op_factories.push_back(std::make_shared<TestSinkOperatorFactory>(2,2, sinkCounter));
    }
}