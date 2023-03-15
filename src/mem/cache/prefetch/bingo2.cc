#include "mem/cache/prefetch/bingo2.hh"

#include "mem/cache/prefetch/associative_set_impl.hh"
#include "mem/cache/replacement_policies/base.hh"
#include "params/Bingo2.hh"

namespace gem5
{

GEM5_DEPRECATED_NAMESPACE(Prefetcher, prefetch);
namespace prefetch
{

Bingo2::Bingo2Entry::Bingo2Entry()
    : TaggedEntry()
{
    invalidate();
}

void
Bingo2::Bingo2Entry::invalidate()
{
    accesses.clear();
    TaggedEntry::invalidate();
}

Bingo2::Bingo2(const Bingo2Params &params)
    : Queued(params), lastAccessedPage(0), toBeVictim(nullptr),
      pcTableInfo(params.table_assoc, params.table_entries,
                  params.table_indexing_policy,
                  params.table_replacement_policy)
    {}

Bingo2::PCTable*
Bingo2::findTable(int context)
{
    auto it = pcTables.find(context);
    if (it != pcTables.end())
        return &it->second;

    return allocateNewContext(context);
}

Bingo2::PCTable*
Bingo2::allocateNewContext(int context)
{
    auto insertion_result = pcTables.insert(
    std::make_pair(context,
        PCTable(pcTableInfo.assoc, pcTableInfo.numEntries,
        pcTableInfo.indexingPolicy, pcTableInfo.replacementPolicy,
        Bingo2Entry())));

    return &(insertion_result.first->second);
}

void
Bingo2::calculatePrefetch(const PrefetchInfo &pfi,
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
        lastAccessedPage = access_addr;

        if (toBeVictim != nullptr)
            pcTable->insertEntry(victimKey, is_secure, toBeVictim);

        Addr key = ((access_addr & nonBlockMask) << 32) | access_pc;

        // Get matching entry from PC
        Bingo2Entry *entry = pcTable->findEntry(key, false);

        // Check if you have entry
        if (entry != nullptr) {
            for (Addr addrPf : entry->accesses){
                addresses.push_back(AddrPriority((addrPf & justOffsetMask) | (lastAccessedPage & nonOffsetMask), 0));
                //fatal_if(true, "Caught prefetching");
            }
            pcTable->accessEntry(entry);
            toBeVictim = nullptr;
        } else {
            toBeVictim = pcTable->findVictim(key);
            victimKey = key;
        }
    } else {

        if (toBeVictim != nullptr){
            std::vector<Addr> &accesses = toBeVictim->accesses;
            Addr item = access_addr & nonBlockMask;
            if (std::find(accesses.begin(), accesses.end(), item) == accesses.end())
                accesses.push_back(item);
        }
    }
}

uint32_t
Bingo2HashedSetAssociative::extractSet(const Addr pc) const
{
    Addr hash1 = (pc >> 32) & 0xFFFFFFFF;
    Addr hash2 = pc & 0xFFFFFFFF;
    uint32_t key = 0;
    while (hash1 > 0 || hash2 > 0){
      key ^= hash1 ^ hash2;
      hash1 >>= floorLog2(numSets);
      hash2 >>= floorLog2(numSets);
    }
    return key & setMask;
}

Addr
Bingo2HashedSetAssociative::extractTag(const Addr addr) const
{
    return addr;
}

}
}
