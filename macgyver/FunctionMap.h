#pragma once

#include <functional>
#include <map>
#include <vector>
#include <initializer_list>
#include <boost/regex.hpp>
#include "Exception.h"

namespace Fmi
{
    /**
     *  @brief A template class to associate functions with specified names
     *
     *  There are two ways to add functions to the map:
     *    - providing exact names (recommended if possible for better performance)
     *    - providing regex patterns for matching names (in this case mached regex groups
     *      ar passed to the function as argument (type std::vector<std::string>))
     */
    template <typename ValueType, class... ArgumentTypes>
    class FunctionMap
    {
        struct Entry
        {
            std::function<ValueType(ArgumentTypes...)> function;
            std::string description;
            Entry() = default;
            Entry(
                std::function<ValueType(ArgumentTypes...)> function,
                const std::string& description)
                : function(function), description(description)
            {
            }
        };

        struct RegexEntry
        {
            const std::string name;
            boost::regex regex;
            std::function<ValueType(const std::vector<std::string>&, ArgumentTypes...)> function;
            std::string description;
            RegexEntry() = default;
            RegexEntry(const std::string& name,
                       const boost::regex& regex,
                       std::function<ValueType(std::vector<std::string>, ArgumentTypes...)> function,
                       const std::string& description)
                : name(name), regex(regex), function(function), description(description)
            {
            }
        };

        std::map<std::string, Entry> m_map;
        std::vector<RegexEntry> m_regex_entries;

    public:
        FunctionMap() = default;

        FunctionMap(const FunctionMap&) = default;

        FunctionMap& operator=(const FunctionMap&) = default;

        /**
         * @brief Add a function with a single name
         *
         * One may of course call it several times to add multiple identical functions.
         */
        FunctionMap& add(
            const std::string& name,
            std::function<ValueType(ArgumentTypes...)> function,
            const std::string& description = "")
        {
            if (!function)
            {
                report_empty_function( { name } );
            }
            m_map.emplace(name, Entry(function, description));
            return *this;
        }

        /**
         * @brief Add a function with multiple names
         */
        FunctionMap& add(
            const std::vector<std::string>& names,
            std::function<ValueType(ArgumentTypes...)> function,
            const std::string& description = "")
        {
            if (!function)
            {
                report_empty_function(names);
            }

            for (const auto& name : names)
            {
                m_map.emplace(name, Entry(function, description));
            }
            return *this;
        }

        /**
         * @brief Add a function with a regex pattern for matching names
         *
         * @param name The name of the function (used for descriptions only)
         * @param regex The regex pattern to match against full function name
         * @param function The function to be called when the regex matches (the first argument
         *               is a vector of matched regex groups, arguments provided as template
         *               parameters follow after that)
         * @param description A description of the function (optional)
         */
        FunctionMap& add(
            const std::string& name,
            const boost::regex& regex,
            std::function<ValueType(const std::vector<std::string>&, ArgumentTypes...)> function,
            const std::string& description = "")
        {
            if (!function)
            {
                report_empty_function( { name } );
            }
            //RegexEntry entry(name, regex, function, description);
            m_regex_entries.emplace_back(name, regex, function, description);
            return *this;
        }

        std::vector<std::string> get_names() const
        {
            std::vector<std::string> names;
            for (const auto& [name, _] : m_map)
            {
                names.push_back(name);
            }
            return names;
        }

        std::vector<std::pair<std::string, std::string>> get_descriptions() const
        {
            std::vector<std::pair<std::string, std::string>> descriptions;
            for (const auto& [name, entry] : m_map)
            {
                descriptions.push_back(std::make_pair(name, entry.description));
            }
            for (const auto& entry : m_regex_entries)
            {
                descriptions.push_back(std::make_pair(entry.name, entry.description));
            }
            return descriptions;
        }

        bool contains(const std::string& name) const
        {
            if (m_map.find(name) != m_map.end())
                return true;

            for (const auto& entry : m_regex_entries)
            {
                if (boost::regex_match(name, entry.regex))
                    return true;
            }

            return false;
        }

        ValueType operator()(const std::string& name, ArgumentTypes... args) const
        try
        {
            // Try exact names first
            auto it = m_map.find(name);
            if (it != m_map.end())
                return it->second.function(args...);

            // Try provided regex (exact regex match, not substring match)
            for (const auto& entry : m_regex_entries)
            {
                boost::smatch pieces_match;
                if (boost::regex_match(name, pieces_match, entry.regex))
                {
                    std::vector<std::string> r_args;
                    for (size_t i = 1; i < pieces_match.size(); ++i)
                    {
                        r_args.push_back(pieces_match[i].str());
                    }
                    return entry.function(r_args, args...);
                }
            }

            // Nothing found: report error
            throw Fmi::Exception(BCP, "'" + name + "'" + " not found in FunctionMap");
        }
        catch (...)
        {
            auto error = Fmi::Exception::Trace(BCP, "Exception while evaluating value");
            error.addParameter("name", name);
            throw error;
        }

        virtual ~FunctionMap() = default;

    private:
        void report_empty_function(const std::vector<std::string>& names) const
        {
            Fmi::Exception error(BCP, "FunctionMap: Function is null");
            std::string names_str;
            for (const auto& name : names)
            {
                names_str += (names_str == "" ? "" : ", ") + name;
            }
            error.addParameter("names", names_str);
            throw error;
        }
    };

}  // namespace Fmi
