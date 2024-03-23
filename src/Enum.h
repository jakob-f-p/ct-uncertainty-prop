#pragma once

#include <QMetaEnum>
#include <QString>

template<class EnumType>
struct EnumString {
    QString Name;
    EnumType EnumValue = {};
};

#define GET_ENUM_VALUES(EnumType)                                           \
static std::vector<EnumString<EnumType>> Get##EnumType##Values() {          \
    auto metaEnum = QMetaEnum::fromType<EnumType>();                        \
    std::vector<EnumString<EnumType>> values(metaEnum.keyCount());          \
    EnumType enumValue;                                                     \
    for (int i = 0; i < values.size(); i++) {                               \
        enumValue = static_cast<EnumType>(metaEnum.value(i));               \
        values[i] = { EnumType##ToString(enumValue).c_str(), enumValue };   \
    }                                                                       \
    return values;                                                          \
}                                                                           \

