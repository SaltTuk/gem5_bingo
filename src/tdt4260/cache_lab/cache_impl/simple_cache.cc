#include "tdt4260/cache_lab/cache_impl/simple_cache.hh"

#include <bits/stdc++.h>

#include "base/trace.hh"
#include "debug/AllAddr.hh"
#include "debug/AllCacheLines.hh"
#include "debug/TDTSimpleCache.hh"

namespace gem5
{

SimpleCache::SimpleCache(int size, int blockSize, int associativity,
                         statistics::Group *parent, const char *name)
    : size(size), blockSize(blockSize), associativity(associativity), cacheName(name),
      stats(parent, name)
{
    numEntries = this->size / this->blockSize;
    numSets = this->numEntries / this->associativity;

    // allocate entries for all sets and ways
    for (int i = 0; i < this->numSets; i++) {
      // TODO: Associative: Allocate as many entries as there are ways
      std::vector<Entry *> vec; //
      for(int i = 0; i < this->associativity; i++) //
        vec.push_back(new Entry()); //
      entries.push_back(vec);
    }
}

SimpleCache::SimpleCacheStats::SimpleCacheStats(
    statistics::Group *parent, const char *name)
    : statistics::Group(parent, name),
    ADD_STAT(reqsReceived, statistics::units::Count::get(),
        "Number of requests received from cpu side"),
    ADD_STAT(reqsServiced, statistics::units::Count::get(),
        "Number of requests serviced at this cache level"),
    ADD_STAT(respsReceived, statistics::units::Count::get(),
        "Number of responses received from mem side") {}

void
SimpleCache::recvReq(Addr req, int size)
{
    ++stats.reqsReceived;
    int index = calculateIndex(req);
    int tag = calculateTag(req);
    ctr++;

    DPRINTF(TDTSimpleCache, "Debug: Addr: %#x, index: %d, tag: %d, in %s\n",
            req, index, tag, cacheName);
    DPRINTF(AllAddr, "%#x\n", req);
    DPRINTF(AllCacheLines, "%#x\n", req >> ((int) std::log2(blockSize)));
    if (hasLine(index, tag)) {
        ++stats.reqsServiced;
        int way = lineWay(index, tag);
        DPRINTF(TDTSimpleCache, "Hit: way: %d\n", way);
        // TODO: Associative: Update LRU info for line in entries
        for(int i = 0; i < associativity; i++) //  
          entries.at(index).at(i)->lastUsed++; //  
        entries.at(index).at(way)->lastUsed = 0; //
        sendResp(req);
    } else{
        sendReq(req, size);
    }
}

void
SimpleCache::recvResp(Addr resp)
{
    ++stats.respsReceived;
    int index = calculateIndex(resp);
    int tag = calculateTag(resp);
    // there should never be a request (and thus a response) for a line already in the cache
    assert(!hasLine(index, tag));

    int way = oldestWay(index);
    DPRINTF(TDTSimpleCache, "Miss: Replaced way: %d\n", way);
    // TODO: Direct-Mapped: Record new cache line in entries
    entries.at(index).at(way)->tag = tag;
    // TODO: Associative: Record LRU info for new line in entries
    for(int i = 0; i < associativity; i++) //
      entries.at(index).at(i)->lastUsed++; //
    entries.at(index).at(way)->lastUsed = 0; //
    sendResp(resp);
}

int
SimpleCache::calculateTag(Addr req)
{
    // TODO: Direct-Mapped: Calculate tag
    // hint: req >> ((int)std::log2(...
    req >>= ((int)std::log2(numSets*blockSize)); //
    return req;
}

int
SimpleCache::calculateIndex(Addr req)
{
    // TODO: Direct-Mapped: Calculate index
    req >>= ((int)std::log2(blockSize)); //
    req &= ((1 << ((int)std::log2(numSets))) - 1); // 
    return req;
}

bool
SimpleCache::hasLine(int index, int tag)
{
    // TODO: Direct-Mapped: Check if line is already in cache
    // TODO: Associative: Check all possible ways
    for(int i = 0; i < associativity; i++){ //
      if(entries.at(index).at(i)->tag == tag) //
        return true; //
    }
    return false; //
}

int
SimpleCache::lineWay(int index, int tag)
{
    // TODO: Associative: Find in which way a cache line is stored
    for(int i = 0; i < associativity; i++){ //
      if(entries.at(index).at(i)->tag == tag) //
        return i; //
    }
    return -1; //
}

int
SimpleCache::oldestWay(int index)
{
    // TODO: Associative: Determine the oldest way
    int oldest = 0;
    int oldestWay = 0;
    for(int i = 0; i < associativity; i++){ //
      if(entries.at(index).at(i)->lastUsed >= oldest){ //
        oldest = entries.at(index).at(i)->lastUsed;
        oldestWay = i;
      } 
    }
    return oldestWay;
}

void
SimpleCache::sendReq(Addr req, int size)
{
    memSide->recvReq(req, size);
}

void
SimpleCache::sendResp(Addr resp)
{
    cpuSide->recvResp(resp);
}

}
