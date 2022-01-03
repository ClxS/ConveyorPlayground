#pragma once

#include <string>

namespace cpp_conv
{
    class Item
    {
    public:
        Item(std::string strName)
            : m_strName(std::move(strName))
        {
        }

        const std::string& GetName() const { return m_strName; }

    private:
        std::string m_strName;
    };

    class Aluminium : public Item { public: Aluminium() : Item("Aluminium") { } };
    class Metal : public Item { public: Metal() : Item("Metal") { } };
    class Tin : public Item { public: Tin() : Item("Tin") { } };
    class Copper : public Item { public: Copper() : Item("Copper") { } };
}