#pragma once

#include <QMetaEnum>
#include <QString>

template<class EnumType>
struct EnumString {
    QString Name;
    EnumType EnumValue = {};
};

#define ENUM_GET_VALUES(EnumType)                                           \
[[nodiscard]] static auto                                                   \
Get##EnumType##Values() -> std::vector<EnumString<EnumType>> {              \
    auto metaEnum = QMetaEnum::fromType<EnumType>();                        \
    size_t numberOfEnums = metaEnum.keyCount();                             \
    std::vector<EnumString<EnumType>> values(numberOfEnums);                \
    for (int i = 0; i < numberOfEnums; i++) {                               \
        auto enumValue = static_cast<EnumType>(metaEnum.value(i));          \
        values[i] = { QString::fromStdString(EnumType##ToString(enumValue)),\
                      enumValue };                                          \
    }                                                                       \
    return values;                                                          \
}                                                                           \

#define ENUM_GET_VALUES_CONSTEXPR(EnumType, LastEnumName)                   \
[[nodiscard]] static auto                                                   \
Get##EnumType##Values() -> std::vector<EnumString<EnumType>> {              \
    size_t numberOfEnums = EnumType::LastEnumName;                          \
    std::vector<EnumString<EnumType>> values(numberOfEnums);                \
    for (int i = 0; i < numberOfEnums; i++) {                               \
        auto enumValue = static_cast<EnumType>(i);          \
        values[i] = { QString::fromStdString(EnumType##ToString(enumValue)),\
                      enumValue };                                          \
    }                                                                       \
    return values;                                                          \
}                                                                           \

