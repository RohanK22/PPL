#ifndef EOS_VALUE_HPP
#define EOS_VALUE_HPP

#include <string>

// Trait class to define EOS value for different queue types
template<typename T>
struct EOSValue {
    static T value() { return T(); }
};

// Specialization for string queue (EOS is "EOS")
template<>
struct EOSValue<std::string> {
    static std::string value() { return "EOS"; }
};

template<>
struct EOSValue<void*> {
    static void* value() { return nullptr; }
};

#endif // EOS_VALUE_HPP
