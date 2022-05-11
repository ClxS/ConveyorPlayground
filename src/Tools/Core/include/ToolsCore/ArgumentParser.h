#pragma once
#include <functional>
#include <string>
#include <vector>

namespace cppconv::tools::arg_parser
{
    template<typename TReadType>
    struct TypedArgReader;

#define DEFINE_ARG_TYPE_HANDLER(TYPE) \
    template<>\
    struct TypedArgReader<TYPE>\
    {\
        static bool Read(const std::string& value, void* pTargetVariable);\
    };

    DEFINE_ARG_TYPE_HANDLER(bool);
    DEFINE_ARG_TYPE_HANDLER(int32_t);
    DEFINE_ARG_TYPE_HANDLER(float);
    DEFINE_ARG_TYPE_HANDLER(std::string);
    DEFINE_ARG_TYPE_HANDLER(std::vector<std::string>);

    struct ArgumentParameterBase
    {
        ArgumentParameterBase(std::string name, bool isRequired, const char argSwitch, std::string longSwitch, bool(*readFunc)(const std::string&, void*))
            : m_Name(std::move(name))
            , m_bIsRequired(isRequired)
            , m_Switch(argSwitch)
            , m_SwitchLong(std::move(longSwitch))
            , m_ReadFunc(readFunc)
        {
        }
        virtual ~ArgumentParameterBase() = default;

        virtual bool Read(const std::string& value) = 0;

        std::string m_Name {};
        bool m_bIsRequired;
        char m_Switch {};
        std::string m_SwitchLong {};
        bool m_bIsSet {};
        bool m_bIsFlag {};

        bool(*m_ReadFunc)(const std::string&, void*);
    };

    template<typename T>
    struct ArgumentParameter final : public ArgumentParameterBase
    {
        ArgumentParameter(std::string name, const bool isRequired, const char argSwitch, std::string longSwitch)
            : ArgumentParameterBase(std::move(name), isRequired, argSwitch, std::move(longSwitch), &TypedArgReader<T>::Read)
        {}

        bool Read(const std::string& value) override
        {
            if (!m_ReadFunc)
            {
                return false;
            }

            return m_ReadFunc(value, &m_Value);
        }

        T m_Value {};
    };

    template<>
    struct ArgumentParameter<bool> final : public ArgumentParameterBase
    {
        ArgumentParameter(std::string name, const bool isRequired, const char argSwitch, std::string longSwitch)
            : ArgumentParameterBase(std::move(name), isRequired, argSwitch, std::move(longSwitch), &TypedArgReader<bool>::Read)
        {
            m_bIsFlag = true;
        }

        bool Read(const std::string& value) override
        {
            if (!m_ReadFunc)
            {
                return false;
            }

            return m_ReadFunc(value, &m_Value);
        }

        bool m_Value {};
    };

    struct VerbParameter
    {
        std::string m_Name;
        std::vector<std::string> m_ValidValues;

        std::string m_Value {};
    };

    struct ArgumentsBase
    {
        explicit ArgumentsBase(VerbParameter* verb, std::vector<ArgumentParameterBase*> argumentFields)
            : m_pVerb{verb}
            , m_ArgumentFields(std::move(argumentFields))
        {
        }

        explicit ArgumentsBase(std::vector<ArgumentParameterBase*> argumentFields)
            : m_pVerb{nullptr}
            , m_ArgumentFields(std::move(argumentFields))
        {
        }

        bool TryRead(int argc, char **argv) const;

        VerbParameter* m_pVerb;
        std::vector<ArgumentParameterBase*> m_ArgumentFields;
    };
}

#define ARG_CTOR_0(Type) Type() {}
#define ARG_CTOR_0_V(Type, Verb) Type() : cppconv::tools::arg_parser::ArgumentsBase(&(m_##Verb), {}) {}
#define ARG_CTOR_1(Type, A) Type() : cppconv::tools::arg_parser::ArgumentsBase({&(m_##A)}) {}
#define ARG_CTOR_1_V(Type, Verb, A) Type() : cppconv::tools::arg_parser::ArgumentsBase(&(m_##Verb), {&(m_##A)}) {}
#define ARG_CTOR_2(Type, A, B) Type() : cppconv::tools::arg_parser::ArgumentsBase({&(m_##A), &(m_##B)}) {}
#define ARG_CTOR_2_V(Type, Verb, A, B) Type() : cppconv::tools::arg_parser::ArgumentsBase(&(m_##Verb), {&(m_##A), &(m_##B)}) {}
#define ARG_CTOR_3(Type, A, B, C) Type() : cppconv::tools::arg_parser::ArgumentsBase({&(m_##A), &(m_##B), &(m_##C)}) {}
#define ARG_CTOR_3_V(Type, Verb, A, B, C) Type() : cppconv::tools::arg_parser::ArgumentsBase(&(m_##Verb), {&(m_##A), &(m_##B), &(m_##C)}) {}
#define ARG_CTOR_4(Type, A, B, C, D) Type() : cppconv::tools::arg_parser::ArgumentsBase({&(m_##A), &(m_##B), &(m_##C), &(m_##D)}) {}
#define ARG_CTOR_4_V(Type, Verb, A, B, C, D) Type() : cppconv::tools::arg_parser::ArgumentsBase(&(m_##Verb), {&(m_##A), &(m_##B), &(m_##C), &(m_##D)}) {}
#define ARG_CTOR_5(Type, A, B, C, D, E) Type() : cppconv::tools::arg_parser::ArgumentsBase({&(m_##A), &(m_##B), &(m_##C), &(m_##D), &(m_##E)}) {}
#define ARG_CTOR_5_V(Type, Verb, A, B, C, D, E) Type() : cppconv::tools::arg_parser::ArgumentsBase(&(m_##Verb), {&(m_##A), &(m_##B), &(m_##C), &(m_##D), &(m_##E)}) {}
#define ARG_CTOR_6_V(Type, Verb, A, B, C, D, E, F) Type() : cppconv::tools::arg_parser::ArgumentsBase(&(m_##Verb), {&(m_##A), &(m_##B), &(m_##C), &(m_##D), &(m_##E), &(m_##F)}) {}
#define ARG_CTOR_7_V(Type, Verb, A, B, C, D, E, F, G) Type() : cppconv::tools::arg_parser::ArgumentsBase(&(m_##Verb), {&(m_##A), &(m_##B), &(m_##C), &(m_##D), &(m_##E), &(m_##F), &(m_##G)}) {}
#define ARG_CTOR_8_V(Type, Verb, A, B, C, D, E, F, G, H) Type() : cppconv::tools::arg_parser::ArgumentsBase(&(m_##Verb), {&(m_##A), &(m_##B), &(m_##C), &(m_##D), &(m_##E), &(m_##F), &(m_##G), &(m_##H)}) {}

#define VERB_1(NAME, VALUE1) cppconv::tools::arg_parser::VerbParameter m_##NAME = { #NAME, { VALUE1 } }
#define VERB_2(NAME, VALUE1, VALUE2) cppconv::tools::arg_parser::VerbParameter m_##NAME = { #NAME, { VALUE1, VALUE2 } }
#define VERB_3(NAME, VALUE1, VALUE2, VALUE3) cppconv::tools::arg_parser::VerbParameter m_##NAME = { #NAME, { VALUE1, VALUE2, VALUE3 } }

#define ARG(NAME, TYPE, SWITCH_SHORT, SWITCH_LONG, DESCRIPTION)\
    cppconv::tools::arg_parser::ArgumentParameter<TYPE> m_##NAME = { #NAME, false, (char)(SWITCH_SHORT), SWITCH_LONG }

#define REQUIRED_ARG(NAME, TYPE, SWITCH_SHORT, SWITCH_LONG, DESCRIPTION)\
cppconv::tools::arg_parser::ArgumentParameter<TYPE> m_##NAME = { #NAME, true, (char)(SWITCH_SHORT), SWITCH_LONG }
