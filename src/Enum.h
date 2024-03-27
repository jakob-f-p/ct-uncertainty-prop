#pragma once

#include <QMetaEnum>
#include <QString>

template<class EnumType>
struct EnumString {
    QString Name;
    EnumType EnumValue = {};
};

#define GET_ENUM_VALUES(EnumType, hasInvalid)                               \
static std::vector<EnumString<EnumType>> Get##EnumType##Values() {          \
    auto metaEnum = QMetaEnum::fromType<EnumType>();                        \
    size_t numberOfEnums = metaEnum.keyCount();                             \
    if (hasInvalid) numberOfEnums--;                                        \
    std::vector<EnumString<EnumType>> values(numberOfEnums);                \
    for (int i = 0; i < numberOfEnums; i++) {                               \
        auto enumValue = static_cast<EnumType>(metaEnum.value(i));          \
        values[i] = { QString::fromStdString(EnumType##ToString(enumValue)),\
                      enumValue };                                          \
    }                                                                       \
    return values;                                                          \
}                                                                           \

