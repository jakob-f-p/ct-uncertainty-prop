#pragma once

#include <QApplication>
#include <QDialog>
#include <QStyledItemDelegate>
#include <QWidget>

#include <concepts>
#include <format>

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
SetWidgetData(QWidget* widget, const auto& data) -> void {
    WidgetUtilsDetails::FindWidget<Widget>(widget).Populate(data);
}


class DialogDelegate : public QStyledItemDelegate {
public:
    explicit DialogDelegate(QObject* parent = nullptr) : QStyledItemDelegate(parent) {}

    auto
    createEditor(QWidget* parent,
                 QStyleOptionViewItem const& option,
                 QModelIndex const& index) const -> QWidget* override;

    void updateEditorGeometry(QWidget* editor,
                              QStyleOptionViewItem const& option,
                              QModelIndex const& index) const override;

protected Q_SLOTS:
    void commitEdit();

    void discardChanges();

protected:
    auto
    eventFilter(QObject* object, QEvent* event) -> bool override;

    [[nodiscard]] virtual auto
    getDialog(QModelIndex const& modelIndex, QWidget* parent) const noexcept -> QDialog* = 0;
};

[[nodiscard]] auto
GenerateIcon(const std::string &fileNamePrefix) noexcept -> QIcon;

[[nodiscard]] auto
GenerateSimpleIcon(const std::string &fileName) noexcept -> QIcon;

[[nodiscard]] auto
GetHeader1StyleSheet() noexcept -> QString;

[[nodiscard]] auto
GetHeader2StyleSheet() noexcept -> QString;

[[nodiscard]] auto
GetHeader3StyleSheet() noexcept -> QString;
