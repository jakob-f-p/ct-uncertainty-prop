#pragma once

#include <QVariant>
#include <QWidget>

template<typename T, typename TData>
struct BaseData {
    static TData GetData(const T& t);

    static QVariant GetQVariant(const T& t);

    static void SetData(T& t, const TData& data);

    static void SetData(T& t, const QVariant& variant);

private:
    BaseData() = default;
};

template<typename Ui, typename Data>
struct BaseUi {
    static QWidget* GetWidget();

    static Data GetWidgetData(QWidget* widget);

    static void SetWidgetData(QWidget* widget, const Data& data);

protected:
    static const QStringList AxisNames;

private:
    BaseUi() = default;
};
