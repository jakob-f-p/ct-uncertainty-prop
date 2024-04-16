#pragma once

#include <QVTKOpenGLNativeWidget.h>

class QVTKInteractor;
class vtkCamera;
class vtkOrientationMarkerWidget;
class vtkOpenGLRenderer;

class CtDataSource;
class CtStructureTree;

class ModelingRenderWidget : public QVTKOpenGLNativeWidget {
public:
    explicit ModelingRenderWidget(CtStructureTree& dataTree, QWidget* parent = nullptr);

    void ResetCamera() const;

    vtkNew<CtDataSource> DataSource;
    CtStructureTree& DataTree;

    vtkNew<vtkOrientationMarkerWidget> OrientationMarkerWidget;
    vtkNew<vtkOpenGLRenderer> Renderer;
    vtkNew<QVTKInteractor> RenderWindowInteractor;
    vtkNew<vtkCamera> InitialCamera;
};