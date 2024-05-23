#pragma once

#include <QLabel>
#include <QStackedWidget>
#include <QVBoxLayout>

template<typename WidgetType>
class OptionalWidget : public QStackedWidget {
public:
    explicit OptionalWidget(QString placeholderText, QWidget* parent = nullptr) :
            QStackedWidget(parent),
            PlaceholderWidget([&placeholderText]() {
                auto* widget = new QWidget();
                auto* vLayout = new QVBoxLayout(widget);
                auto* textLabel = new QLabel(placeholderText);
                vLayout->addWidget(textLabel);
                return widget;
            }()),
            MainWidget(nullptr) {

        addWidget(PlaceholderWidget);
        setCurrentWidget(PlaceholderWidget);
    }

public slots:
    auto
    UpdateWidget(WidgetType* widget) noexcept -> void {
        if (MainWidget) {
            removeWidget(MainWidget);
            delete MainWidget;
        }

        MainWidget = widget;
        addWidget(MainWidget);
        ShowWidget();
    }

    auto
    ShowWidget() noexcept -> void {
        setCurrentWidget(MainWidget);
    }

    auto
    HideWidget() noexcept -> void {
        setCurrentWidget(PlaceholderWidget);
    }

    [[nodiscard]] auto
    Widget() -> WidgetType& {
        if (!MainWidget)
            throw std::runtime_error("Cannot get main widget because it is nullptr");

        return *MainWidget;
    }

private:
    QWidget* PlaceholderWidget;
    WidgetType* MainWidget;
};

