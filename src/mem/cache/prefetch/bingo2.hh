#ifndef __MEM_CACHE_PREFETCH_BINGO2_HH__
#define __MEM_CACHE_PREFETCH_BINGO2_HH__

#include "base/sat_counter.hh"
#include "base/types.hh"
#include "mem/cache/prefetch/associative_set.hh"
#include "mem/cache/prefetch/queued.hh"
#include "mem/cache/tags/indexing_policies/set_associative.hh"
#include "mem/packet.hh"
#include "params/Bingo2HashedSetAssociative.hh"

namespace gem5
{

GEM5_DEPRECATED_NAMESPACE(ReplacementPolicy, replacement_policy);
namespace replacement_policy
{
    class Base;
}

struct Bingo2Params;

GEM5_DEPRECATED_NAMESPACE(Prefetcher, prefetch);
namespace prefetch
{

class Bingo2HashedSetAssociative : public SetAssociative
{
    protected:
        uint32_t extractSet(const Addr addr) const override;
        Addr extractTag(const Addr addr) const override;

    public:
        Bingo2HashedSetAssociative(
            const Bingo2HashedSetAssociativeParams &p)
        : SetAssociative(p)
        {}

        ~Bingo2HashedSetAssociative() = default;
};

class Bingo2 : public Queued
{

  protected:

    const struct PCTableInfo
    {
        const int assoc;
        const int numEntries;

        BaseIndexingPolicy* const indexingPolicy;
        replacement_policy::Base* const replacementPolicy;

        PCTableInfo(int assoc, int num_entries,
            BaseIndexingPolicy* indexing_policy,
            replacement_policy::Base* repl_policy)
          : assoc(assoc), numEntries(num_entries),
            indexingPolicy(indexing_policy), replacementPolicy(repl_policy)
        {

        }
    } pcTableInfo;

    struct Bingo2Entry : public TaggedEntry
    {
        Bingo2Entry();

        Addr lastAddr = 0;

        void invalidate() override;

        std::vector<Addr> accesses;

    };

    typedef AssociativeSet<Bingo2Entry> PCTable;
    std::unordered_map<int, PCTable> pcTables;

    PCTable* findTable(int context);

    PCTable* allocateNewContext(int context);

  public:
    Bingo2(const Bingo2Params &p);

    void calculatePrefetch(const PrefetchInfo &pf1,
                           std::vector<AddrPriority> &addresses) override;

    Addr lastAccessedPage;
    Addr victimKey;
    std::vector<Addr> newAccesses, oldAccesses;
    int prefetchFail;
};

} //namespace prefetch
} //namespace gem5

#endif //__MEM_CACHE_PREFETCH_BINGO2_HH__
