// #include "config.h"
#include "gtest/gtest.h"
#include "SysInfo.h"

// #ifdef ENABLE_GTEST_COMMON

class CGTestCommon
{
public:
	CGTestCommon(){}
	~CGTestCommon(){}
	int Test( int a, int b )
	{
        return simple_add(a,b);
	};
};
/////////////////测试用例//////////////////////
TEST(CGTestCommon, simple_add)
{
	CGTestCommon example;
	EXPECT_EQ(2, example.Test(1,1) );	
	EXPECT_EQ(3, example.Test(1,2) );
}

int main(int argc, char **argv)
{
	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}

// #endif
