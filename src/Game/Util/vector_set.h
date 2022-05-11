#pragma once

namespace cpp_conveyor
{
    template <typename T>
    struct vector_set
    {
        using vec_type = std::vector<T>;
        using const_iterator = typename vec_type::const_iterator;
        using iterator = typename vec_type::iterator;

        vector_set(size_t max_size)
            : _max_size{max_size}
        {
            _v.reserve(_max_size);
        }

        /// @returns: pair of iterator, bool
        /// If the value has been inserted, the bool will be true
        /// the iterator will point to the value, or end if it wasn't
        /// inserted due to space exhaustion
        bool insert(const T& elem)
        {
            if (_v.size() < _max_size)
            {
                auto it = std::lower_bound(_v.begin(), _v.end(), elem);
                if (_v.end() == it || *it != elem)
                {
                    _v.insert(it, elem);
                    return true;
                }

                return false;
            }
            return false;
        }

        [[nodiscard]] const_iterator find(const T& elem) const
        {
            auto vend = _v.end();
            auto it = std::lower_bound(_v.begin(), vend, elem);
            if (it != vend && *it != elem)
            {
                it = vend;
            }
            return it;
        }

        [[nodiscard]] bool contains(const T& elem) const
        {
            return find(elem) != _v.end();
        }

        [[nodiscard]] const_iterator begin() const
        {
            return _v.begin();
        }

        [[nodiscard]] const_iterator end() const
        {
            return _v.end();
        }

    private:
        vec_type _v;
        size_t _max_size;
    };
}
