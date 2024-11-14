// Copyright (c) 2018 The atendb Authors. All rights reserved.

#ifndef ATENDB_INCLUDE_OPTIONS_H_
#define ATENDB_INCLUDE_OPTIONS_H_

#include "KV/Include/comparator.h"
#include "KV/Include/env.h"

namespace atendb {

class Options {
 public:
  Options() :
    comparator_(nullptr) { }

  ~Options() {}

  Comparator* comparator_;
};

} // namespace atendb

#endif // ATENDB_INCLUDE_OPTIONS_H_