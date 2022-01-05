#pragma once

#include <string>

namespace cpp_conv
{
    class Item
    {
    public:
        Item(std::string strName, wchar_t displayIcon)
            : m_strName(std::move(strName))
            , m_displayIcon(displayIcon)
        {
        }

        const std::string& GetName() const { return m_strName; }
        wchar_t GetDisplayIcon() const { return m_displayIcon; }

    private:
        std::string m_strName;
        wchar_t m_displayIcon;
    };

    class Aluminium : public Item { public: Aluminium() : Item("Aluminium", L'A') { } };
    class Metal : public Item { public: Metal() : Item("Metal", L'M') { } };
    class Tin : public Item { public: Tin() : Item("Tin", L'T') { } };
    class Copper : public Item { public: Copper() : Item("Copper", L'C') { } };
}