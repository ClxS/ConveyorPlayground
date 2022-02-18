#pragma once
#include "DataField.h"

namespace cpp_conv
{
    template<typename TSelf, typename Serializer, typename TBase>
    class Serializable : public TBase
    {
    public:
        std::string Serialize(std::string* outErrors = nullptr)
        {
            return Serializer::Serialize(m_Config, m_Fields, outErrors);
        }

        static std::unique_ptr<TSelf> Deserialize(const std::string& input, std::string* outErrors = nullptr)
        {
            std::unique_ptr<TSelf> self = std::make_unique<TSelf>();
            if (!Serializer::TryDeserialize(input, self->m_Config, self->m_Fields, outErrors))
            {
                if (outErrors)
                {
                    *outErrors = std::format("Error reading [{}:{}]\n{}", self->m_Config.m_RootTable, self->GetName(), *outErrors);
                }

                self.reset();
            }

            return self;
        }

    protected:
        explicit Serializable(const typename Serializer::Config& config, std::vector<DataFieldBase*> fields)
            : m_Config(config)
            , m_Fields(std::move(fields))
        {
        }

    private:
        const typename Serializer::Config& m_Config;
        std::vector<DataFieldBase*> m_Fields;
    };
}

