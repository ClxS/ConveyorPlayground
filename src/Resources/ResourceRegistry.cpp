#include "ResourceRegistry.h"
#include <tuple>

bool cpp_conv::resources::registry::RegistryId::operator<(const RegistryId& r) const
{
	return std::make_tuple(m_index, m_category) < std::make_tuple(r.m_index, r.m_category);
}
