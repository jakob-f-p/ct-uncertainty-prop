#pragma once

#include <QMetaEnum>
#include <QString>

template<class EnumType>
struct EnumNamePair {
    QString Name;
    EnumType EnumValue = {};
};

#define ENUM_GET_VALUES(EnumType)                                           \
[[nodiscard]] static auto                                                   \
Get##EnumType##Values() -> std::vector<EnumNamePair<EnumType>> {            \
    auto metaEnum = QMetaEnum::fromType<EnumType>();                        \
    size_t const numberOfEnums = metaEnum.keyCount();                       \
    std::vector<EnumNamePair<EnumType>> values(numberOfEnums);              \
    for (int i = 0; i < numberOfEnums; i++) {                               \
        auto enumValue = static_cast<EnumType>(metaEnum.value(i));          \
        values[i] = { QString::fromStdString(EnumType##ToString(enumValue)),\
                      enumValue };                                          \
    }                                                                       \
    return values;                                                          \
}                                                                           \
