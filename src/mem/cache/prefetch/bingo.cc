#include "mem/cache/prefetch/bingo.hh"

#include "mem/cache/prefetch/associative_set_impl.hh"
#include "mem/cache/replacement_policies/base.hh"
#include "params/TDTPrefetcher.hh"

namespace gem5
{

GEM5_DEPRECATED_NAMESPACE(Prefetcher, prefetch);
namespace prefetch
{

BingoPrefetcher::BingoPrefetcher(const TDTPrefetcherParams &params)
  : TDTPrefetcher(params), lastAccessedPage(0), toBeVictim(nullptr)
  {}

void
BingoPrefetcher::calculatePrefetch(const PrefetchInfo &pfi,
                                 std::vector<AddrPriority> &addresses)
{
    Addr nonOffsetMask = ~((Addr)0xFFF);
    Addr justOffsetMask = ((Addr)0xFFF);
    Addr nonBlockMask = ~((Addr)0x3F);

    Addr access_addr = pfi.getAddr();
    Addr access_pc = pfi.getPC();
    bool is_secure = pfi.isSecure();

    int context = 0;

    // Get matching storage of entries
    // Context is 0 due to single-threaded application
    PCTable* pcTable = findTable(context);

    if ((access_addr & nonOffsetMask) != (lastAccessedPage & nonOffsetMask)){

        if (toBeVictim != nullptr)
                    pcTable->insertEntry(victimKey, is_secure, toBeVictim);

        Addr key = ((access_addr & nonBlockMask) << 32) | access_pc;

        // Get matching entry from PC
        TDTEntry *entry = pcTable->findEntry(key, false);

        // Check if you have entry
        if (entry != nullptr) {
            for (Addr addrPf : entry->accesses)
                addresses.push_back(AddrPriority((addrPf & justOffsetMask) | (lastAccessedPage & nonOffsetMask), 0));
            pcTable->accessEntry(entry);
            toBeVictim = nullptr;
        } else {
            toBeVictim = pcTable->findVictim(key);
            victimKey = key;
        }
    } else {

        if (toBeVictim != nullptr){
            std::vector<Addr> accesses = toBeVictim->accesses;
            Addr item = access_addr & nonBlockMask;
            if (std::find(accesses.begin(), accesses.end(), item) == accesses.end())
                accesses.push_back(item);
        }
    }
}

}
}
