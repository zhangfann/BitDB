#pragma once

enum Mode{
    ReadWrite,
    ReadOnly,

};

struct SnapshotMode{
    int version;
};