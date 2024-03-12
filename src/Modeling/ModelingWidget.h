#pragma once

#include <QMainWindow>

class ModelingWidget : public QMainWindow {
Q_OBJECT

public:
    ModelingWidget();

private:
    void SetUpRenderingWidgetForShowingImplicitData();

    void SetUpDockWidgetForImplicitCsgTreeModeling();
};