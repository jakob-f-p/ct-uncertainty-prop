#pragma once

#include <QDialog>
#include <QStyledItemDelegate>
#include <QWidget>

#include <concepts>

template<typename T>
concept TWidget = std::derived_from<T, QWidget> && std::same_as<T, std::remove_pointer_t<std::decay_t<T>>>;

namespace WidgetUtilsDetails {
    template<TWidget Widget>
    [[nodiscard]] auto static
    FindWidget(QWidget* widget) -> Widget& {
        if (!widget)
            throw std::runtime_error("Given widget must not be nullptr");

        auto* basicStructureWidget = widget->findChild<Widget*>();

        if (!basicStructureWidget)
            throw std::runtime_error("No basic structure widget contained in given widget");

        return *basicStructureWidget;
    }
}

template<TWidget Widget>
[[nodiscard]] auto static
GetWidgetData(QWidget* widget) { return WidgetUtilsDetails::FindWidget<Widget>(widget).GetData(); }

template<TWidget Widget>
auto static
SetWidgetData(QWidget* widget, const auto& data) -> void { WidgetUtilsDetails::FindWidget<Widget>(widget).Populate(data); }


class DialogDelegate : public QStyledItemDelegate {
public:
    explicit DialogDelegate(QObject* parent = nullptr) : QStyledItemDelegate(parent) {}

    auto
    createEditor(QWidget* parent,
                 const QStyleOptionViewItem& option,
                 const QModelIndex& index) const -> QWidget* override {
        if (!index.isValid())
            return nullptr;

        auto* dialog = getDialog(index, parent);

        connect(dialog, &QDialog::accepted, this, &DialogDelegate::commitEdit);
        connect(dialog, &QDialog::rejected, this, &DialogDelegate::discardChanges);

        return dialog;
    }

    void updateEditorGeometry(QWidget* editor,
                              QStyleOptionViewItem const& option,
                              QModelIndex const& index) const override {}

protected slots:
    void commitEdit() {
        auto* ctStructureEditDialog = qobject_cast<QDialog*>(sender());

        emit commitData(ctStructureEditDialog);
        emit closeEditor(ctStructureEditDialog);
    }

    void discardChanges() {
        auto* ctStructureEditDialog = qobject_cast<QDialog*>(sender());

        emit closeEditor(ctStructureEditDialog);
    }

protected:
    auto
    eventFilter(QObject* object, QEvent* event) -> bool override { return false; }

    [[nodiscard]] auto virtual
    getDialog(QModelIndex const& modelIndex, QWidget* parent) const noexcept -> QDialog* = 0;
};

[[nodiscard]] auto static
GenerateIcon(const std::string &filePrefix) noexcept -> QIcon {
    QIcon icon;
    QString const qFilePrefix = QString::fromStdString(filePrefix);
    icon.addPixmap(QPixmap(":/" + qFilePrefix + "Normal.png"), QIcon::Normal);
    icon.addPixmap(QPixmap(":/" + qFilePrefix + "Disabled.png"), QIcon::Disabled);
    return icon;
}

[[nodiscard]] auto static
GetHeaderStyleSheet() noexcept -> QString {
    return "font-size: 14px; font-weight: bold";
}

