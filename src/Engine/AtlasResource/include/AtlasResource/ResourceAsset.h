#pragma once

namespace atlas::resource
{
    struct ResourceAsset
    {
        ResourceAsset() = default;
        virtual ~ResourceAsset() = default;
        ResourceAsset(const ResourceAsset&) = delete;
        ResourceAsset(const ResourceAsset&&) = delete;
        ResourceAsset operator=(const ResourceAsset&) = delete;
        ResourceAsset operator=(const ResourceAsset&&) = delete;
    };
}
