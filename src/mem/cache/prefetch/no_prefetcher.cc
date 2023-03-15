#include "mem/cache/prefetch/no_prefetcher.hh"

namespace gem5
{

GEM5_DEPRECATED_NAMESPACE(Prefetcher, prefetch);
namespace prefetch
{

NoPrefetcher::NoPrefetcher(const TDTPrefetcherParams &params)
  : TDTPrefetcher(params)
  {}

void
NoPrefetcher::calculatePrefetch(const PrefetchInfo &pfi,
                                 std::vector<AddrPriority> &addresses)
{
    // do nothing
}

}
}
