#pragma once
#include <optional>

#include "AtlasCore/Hashing.h"
#include "AssetRegistryEntry.h"
#include "RegistryId.h"

namespace atlas::resource
{
    class RegistryBundle
    {
    public:
        virtual ~RegistryBundle() {}
        virtual std::optional<AssetRegistryEntry> GetAsset(RegistryId id) = 0;

        std::optional<RegistryId> LookupId(const std::string_view id)
        {
            const auto hash = core::hashing::fnv1(id);
            return TryLookupId(hash);
        }

    protected:
        virtual std::optional<RegistryId> TryLookupId(uint64_t relativeNameHash) = 0;
    };
}
