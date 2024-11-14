// Copyright (c) 2018 The atendb Authors. All rights reserved.

#ifndef ATENDB_DB_DB_IMPL_H_
#define ATENDB_DB_DB_IMPL_H_

#include <atomic>
#include <algorithm>

#include "KV/Include/Db.h"
#include "KV/Include/Options.h"
#include "KV/Include/Comparator.h"
#include "KV/Include/Skiplist.h"
#include "KV/util/Arena.h"
#include "KV/file/File.h"
#include "KV/util/Lock.h"

namespace atendb {

const std::string IndexFileName = "/INDEX-";
const std::string DataFileName = "/DATA-";

typedef SkipList<Index, Compare> Table;

class DBImpl : public DB {
 public:
  DBImpl(const std::string& dbname, const Options& options);

  virtual ~DBImpl();

  virtual bool Put(const std::string& key, const std::string& value);

  virtual bool Get(const std::string& key, std::string* value);

  virtual bool Delete(const std::string& key);

  void IndexCallback(File* file);

  bool Recover();

 private:
  friend DB;

  std::string dbname_;
  Options options_;
  std::atomic<uint32_t> cursor_;

  uint32_t max_file_;
  std::vector<File* > data_files_;
  std::vector<File* > index_files_;


  uint32_t table_size_;
  std::vector<Table *> tables_;

  // No copying allowed
  void operator=(const DBImpl&);
  DBImpl(const DBImpl&);
};


} // namespace atendb

#endif