#pragma once

#include <any>
#include <map>
#include <typeinfo>
#include <typeindex>
#include <variant>
#include "Exception.h"
#include "TypeName.h"

#include <iostream>

namespace Fmi
{

/**
 *   @brief A template class to associate object with specified types
 *
 *   Intended mostly for use with std::any and boost::variant which can
 *   contain data of different types to avoid long if then else chains
 *
 *   An example for use with std::function<std::string(const std::any&) kanssa:
 *   @code
 *   TypeMap<std::function<std::string(const std::any&)> > W;
 *   W.add<int>(
 *        [](const std::any& x) -> std::string
 *        {
 *            return "INT: " + std::to_string(std::any_cast<int>(x));
 *        });
 *    W.add<std::string>(
 *        [](const std::any& x) -> std::string
 *        {
 *            return "STRING: '" + std::any_cast<std::string>(x) + "'";
 *        });
 *    std::any foo = int(1), bar(std::string("bar"));
 *    std::cout << W[foo](foo) << std::endl;
 *    std::cout << W[bar](bar) << std::endl;
 *
 *   @endcode
 *
 *   Similar example for std::variant<>:
 *   @code
 *    TypeMap<std::function<std::string(const std::variant<int, std::string>&)> > W;
 *    W.add<int>([](
 *            const std::variant<int, std::string> x) -> std::string
 *            {
 *                return "INT: " + std::to_string(std::get<int>(x));
 *            });
 *    W.add<std::string>([](
 *            const std::variant<int, std::string> x) -> std::string
 *            {
 *                return "STRING: '" + std::get<std::string>(x) + "'";
 *            });
 *    std::variant<int, std::string> foo = int(42), bar = "24";
 *    std::cout << W[foo](foo) << std::endl;
 *    std::cout << W[bar](bar) << std::endl;
 *   @edcode
 */
template <typename ValueType>
class TypeMap
{
    struct GetTypeVisitor
    {
        template <typename Type>
        const std::type_info& operator () (const Type& x) const
        {
            return typeid(Type);
        }
    };
public:
    TypeMap() = default;
    virtual ~TypeMap() = default;

    template <typename Type>
    TypeMap& add(const ValueType& value)
    {
        content[std::type_index(typeid(Type))] = value;
        return *this;
    }

    const ValueType& operator [] (const std::type_info& ti) const
    {
        auto it = content.find(std::type_index(ti));
        if (it != content.end()) {
            return it->second;
        } else {
            throw Exception(BCP, "Fmi::TypeMap: No value provided for type "
                + demangle_cpp_type_name(ti.name()));
        }
    };

    const ValueType& operator [] (const std::any& x) const
    {
        return operator [] (x.type());
    };

    template <class... VariantTypes>
    const ValueType& operator [] (const std::variant<VariantTypes...>& x) const
    {
        return operator [] (std::visit(GetTypeVisitor(), x));
    };
private:
    std::map<std::type_index, ValueType> content;
};

} // namespace Fmi
