#include "config.h"
#include "gtest/gtest.h"
#include "../plugin_service.h"

#ifdef ENABLE_GTEST_PLUGINSERVICE

class CGTestPluginService
{
public:
	CGTestPluginService(){}
	~CGTestPluginService(){}
	int Test( int a, int b )
	{
        return simple_add(a,b);
	};
};
/////////////////测试用例//////////////////////
TEST(CGTestPluginService, simple_add)
{
	CGTestPluginService example;
	EXPECT_EQ(2, example.Test(1,1) );	
	EXPECT_EQ(3, example.Test(1,2) );
}

#endif
