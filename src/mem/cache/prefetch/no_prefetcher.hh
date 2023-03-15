#ifndef __MEM_CACHE_NO_PREFETCH_HH__
#define __MEM_CACHE_NO_PREFETCH_HH__

#include "mem/cache/prefetch/tdt_prefetcher.hh"

namespace gem5
{
GEM5_DEPRECATED_NAMESPACE(Prefetcher, prefetch);
namespace prefetch
{

class NoPrefetcher : public TDTPrefetcher
{

  public:
    NoPrefetcher(const TDTPrefetcherParams &p);

    void calculatePrefetch(const PrefetchInfo &pf1,
                           std::vector<AddrPriority> &addresses) override;

};

} //namespace prefetch
} //namespace gem5

#endif //__MEM_CACHE_NO_PREFETCH_HH__
