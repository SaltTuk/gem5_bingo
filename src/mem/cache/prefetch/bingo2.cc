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
    : Queued(params), lastAccessedPage(0), victimKey(0), prefetchFail(0),
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

        if (prefetchFail){
            Bingo2Entry* victimEntry = pcTable->findVictim(victimKey);
            for (Addr addrPf : newAccesses)
                victimEntry->accesses.push_back(addrPf);
            pcTable->insertEntry(victimKey, is_secure, victimEntry);
        }

        Addr key = (access_pc << 32) | access_addr;

        // Get matching entry from PC
        Bingo2Entry *entry = pcTable->findEntry(key, false);

        if (entry == nullptr)
            entry = pcTable->findEntry2(key, false);

        // Check if you have entry
        if (entry != nullptr) {
            oldAccesses.clear();
            for (Addr addrPf : entry->accesses){
                oldAccesses.push_back(((addrPf & justOffsetMask) | (lastAccessedPage & nonOffsetMask)) & nonBlockMask);
                addresses.push_back(AddrPriority((addrPf & justOffsetMask) | (lastAccessedPage & nonOffsetMask), 0));
                //fatal_if(true, "Caught prefetching");
            }
            pcTable->accessEntry(entry);
            prefetchFail = 0;

        } else {
            prefetchFail = 1;
        }
        victimKey = key;
        newAccesses.clear();

    } else {
        Addr item = access_addr & nonBlockMask;
        if (std::find(newAccesses.begin(), newAccesses.end(), item) == newAccesses.end())
            newAccesses.push_back(item);
        if (!prefetchFail){
            if (std::find(oldAccesses.begin(), oldAccesses.end(), item) == oldAccesses.end())
                prefetchFail = 1;
        }
    }
}

uint32_t
Bingo2HashedSetAssociative::extractSet(const Addr pc) const
{
    uint32_t hash1 = (pc >> 26) & 0xFFFFFFC0;
    uint32_t hash2 = (pc >> 6) & 0x3F;
    uint32_t hash = hash1 | hash2;
    uint32_t key = 0;
    while (hash > 0){
      key ^= hash;
      hash >>= floorLog2(numSets);
    }
    return key & setMask;
}

Addr
Bingo2HashedSetAssociative::extractTag(const Addr addr) const
{
    return addr & ~((Addr)0x3F);
}

}
}
