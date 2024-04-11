#pragma once

#include <QVTKInteractor.h>
#include <QVTKOpenGLNativeWidget.h>
#include <vtkOrientationMarkerWidget.h>
#include <vtkOpenGLRenderer.h>

class CtDataSource;
class CtStructureTree;

class ModelingRenderWidget : public QVTKOpenGLNativeWidget {
public:
    explicit ModelingRenderWidget(QWidget* parent = nullptr);

    void ResetCamera() const;

    vtkNew<CtDataSource> DataSource;
    CtStructureTree& DataTree;

    vtkNew<vtkOrientationMarkerWidget> OrientationMarkerWidget;
    vtkNew<vtkOpenGLRenderer> Renderer;
    vtkNew<QVTKInteractor> RenderWindowInteractor;
    vtkNew<vtkCamera> InitialCamera;
};