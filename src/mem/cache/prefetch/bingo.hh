#ifndef __MEM_CACHE_BINGO_HH__
#define __MEM_CACHE_BINGO_HH__

#include "mem/cache/prefetch/tdt_prefetcher.hh"

namespace gem5
{
GEM5_DEPRECATED_NAMESPACE(Prefetcher, prefetch);
namespace prefetch
{

class BingoPrefetcher : public TDTPrefetcher
{

  public:
    BingoPrefetcher(const TDTPrefetcherParams &p);

    void calculatePrefetch(const PrefetchInfo &pf1,
                           std::vector<AddrPriority> &addresses) override;

    Addr lastAccessedPage;
    TDTEntry* toBeVictim;
    Addr victimKey;
};

} //namespace prefetch
} //namespace gem5

#endif //__MEM_CACHE_BINGO_HH__
