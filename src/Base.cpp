#include "Base.h"

#include "Modeling/BasicStructure.h"
#include "Modeling/CombinedStructure.h"

template struct BaseData<BasicStructure, BasicStructureData>;
template struct BaseData<CombinedStructure, CombinedStructureData>;

template<typename T, typename TData>
TData BaseData<T, TData>::GetData(const T& t) {
    TData data {};
    TData::AddBaseData(t, data);
    return data;
}

template<typename T, typename TData>
QVariant BaseData<T, TData>::GetQVariant(const T& t) {
    return QVariant::fromValue(std::move(GetData(t)));
}

template<typename T, typename TData>
void BaseData<T, TData>::SetData(T& t, const TData& data) {
    TData::SetBaseData(t, data);
}


template<typename T, typename TData>
void BaseData<T, TData>::SetData(T& t, const QVariant& variant) {
    if (!variant.canConvert<TData>()) {
        qWarning("Cannot convert variant to data");
        return;
    }

    TData data = variant.value<TData>();
    SetData(t, data);
}



template class BaseUi<BasicStructureUi, BasicStructureData>;
template class BaseUi<CombinedStructureUi, CombinedStructureData>;

template<typename Ui, typename Data>
QWidget* BaseUi<Ui, Data>::GetWidget() {
    auto* widget = new QWidget();
    Ui::AddBaseWidgets(widget);
    return widget;
}

template<typename Ui, typename Data>
Data BaseUi<Ui, Data>::GetWidgetData(QWidget* widget) {
    if (!widget) {
        qWarning("Given widget was nullptr");
        return {};
    }

    Data data {};
    Ui::AddBaseWidgetsData(widget, data);
    return data;
}

template<typename Ui, typename Data>
void BaseUi<Ui, Data>::SetWidgetData(QWidget* widget, const Data& data) {
    if (!widget) {
        qWarning("Given widget was nullptr");
        return;
    }

    Ui::SetBaseWidgetsData(widget, data);
}

template<typename Ui, typename Data>
const QStringList BaseUi<Ui, Data>::AxisNames = { "x", "y", "z" };