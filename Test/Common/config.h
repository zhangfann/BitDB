//
//  "$Id$"
//
//  Copyright (c)1992-2010, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//

#ifndef TEST_CONFIG_H__
#define TEST_CONFIG_H__

//#include "Infra/PrintLog.h"

//#define TestPrintf	Dahua::Infra::debugf
#define TestPrintf	printf
#define ReturnFalse	do{ TestPrintf("File:%s,Line:%d return error!\n",__FILE__,__LINE__); return false;}while(0)


#define ENABLE_GTEST_BITCASK

#define ENABLE_GTEST_COMMON
//#define ENABLE_GTEST_EXAMPLE
//#define ENABLE_GTEST_PLUGINSERVICE
//#define ENABLE_GTEST_FILES_MAP
//#define ENABLE_GTEST_OSDMANAGER
//#define ENABLE_GTEST_OBJECT
//#define ENABLE_GTEST_INODEFILE
//#define ENABLE_GTEST_MDSUID
//#define ENABLE_GTEST_DATANODES_MAP
//#define ENABLE_GTEST_LEASE_MANAGER
//#define ENABLE_GTEST_FILES_UNDERCONSTRUCTION
//#define ENABLE_GTEST_FILES_BROKEN
//#define ENABLE_GTEST_FILES_NEEDTORECOVERED
//#define ENABLE_GTEST_FILES_NEEDTOBEDELETED
//#define ENABLE_GTEST_DATANODE_DESCRIPTOR
//#define ENABLE_GTEST_DATANODE_CHOOSER
//#define ENABLE_GTEST_EFSIMAGE
//#define ENABLE_GTEST_EDITLOG
//#define ENABLE_GTEST_HA_SLAVE_SESSION
//#define ENABLE_GTEST_EDITLOG_LIST
//#define ENABLE_GTEST_MEMORY_ALLOC
//#define ENABLE_GTEST_READ_BACK_THREAD
//#define ENABLE_GTEST_EDITLOG_PARSER
//#define ENABLE_GTEST_EDITLOGCACHE
//#define ENABLE_GTEST_ASYNC_EDITLOG_LOADER
//#define ENABLE_BUCKETS_TEST
//#define ENABLE_GTEST_BUCKET_TREE
//#define ENABLE_BUCKETS_LIST_TEST
//#define ENABLE_GTEST_BUCKET_MAP_4_SERIALIZE
//#define ENABLE_GTEST_CHashMap
//#define ENABLE_GTEST_INNER_VERSION
//#define TEST_LEASE
//#define ENABLE_GTEST_MERGE_TOOL
//#define ENABLE_GTEST_ERASURECODECONFIG
//#define ENABLE_GTEST_SYSTEM_CAPACITY
//#define ENABLE_GTEST_METADATA_STORAGE_DIR
//#define ENABLE_GTEST_EDITLOG_INDEXES
//#define ENABLE_GTEST_LOOP_EDITLOG
//#define ENABLE_GTEST_MDS_USER
//#define ENABLE_GTEST_LOGIN

//#define ENABLE_GTEST_FILE_RECOVER

#endif // TEST_CONFIG_H__
