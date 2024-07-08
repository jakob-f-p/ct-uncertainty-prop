#include "App.h"

#include "Artifacts/PipelineList.h"
#include "DataInitializer.h"
#include "Modeling/CtDataSource.h"
#include "Modeling/CtStructureTree.h"
#include "PipelineGroups/PipelineGroupList.h"
#include "PipelineGroups/PipelineParameterSpan.h"
#include "Segmentation/ThresholdFilter.h"
#include "Ui/MainWindow.h"
#include "Utils/PythonInterpreter.h"

#include <QApplication>
#include <QSurfaceFormat>

#include <QVTKOpenGLNativeWidget.h>

#include <SMP/Common/vtkSMPToolsAPI.h>


App::App(int argc, char* argv[]) :
        Argc(argc),
        Argv(argv),
        QApp(new QApplication(Argc, Argv)),
        MainWin(nullptr),
        CtDataTree(new CtStructureTree()),
        DataSource([&]() {
            vtkNew<CtDataSource> dataSource;
            dataSource->SetDataTree(CtDataTree.get());
            return dataSource;
        }()),
        Pipelines(new PipelineList(*CtDataTree, *DataSource)),
        PipelineGroups([&pipelines = *Pipelines]() {
            auto* const pipelineGroupList = new PipelineGroupList(pipelines);

            auto const& removeDependentPipelineGroups = [pipelineGroupList](PipelineEvent const& event) {
                if (event.Type != PipelineEventType::PRE_REMOVE)
                    return;

                if (event.PipelinePointer == nullptr)
                    throw std::runtime_error("pipeline pointer may not be nullptr");

                auto pipelineGroups = pipelineGroupList->FindPipelineGroupsByBasePipeline(*event.PipelinePointer);
                for (auto const* group : pipelineGroups)
                    pipelineGroupList->RemovePipelineGroup(*group);
            };
            pipelines.AddPipelineEventCallback(removeDependentPipelineGroups);

            return pipelineGroupList;
        }()),
        PyInterpreter(new PythonInterpreter()) {

    QSurfaceFormat::setDefaultFormat(QVTKOpenGLNativeWidget::defaultFormat());

    auto& smpToolsApi =  vtk::detail::smp::vtkSMPToolsAPI::GetInstance();
    //    smpToolsApi.SetBackend("STDTHREAD");
}

App::~App() {
    QApplication::quit();
}

App* App::Self = nullptr;

auto App::CreateInstance(int argc, char* argv[]) -> App* {
    if (Self)
        throw std::runtime_error("App already exists. Cannot create new instance.");

    Self = new App(argc, argv);
    return Self;
}

auto App::GetInstance() -> App* {
    if (!Self)
        throw std::runtime_error("No instance exists. Instance needs to be created first.");

    return Self;
}

auto App::Run() -> int {
    auto& smpToolsApi =  vtk::detail::smp::vtkSMPToolsAPI::GetInstance();
    qWarning(("Backend: " + std::string(smpToolsApi.GetBackend())).c_str());

    InitializeWithTestData();

    MainWin = std::make_unique<MainWindow>(*CtDataTree, *DataSource, *ThresholdFilterAlgorithm,
                                           *Pipelines, *PipelineGroups);
    MainWin->show();

    return QApplication::exec();
}

auto App::Quit() -> int {
    if (Self) {
        QApplication::quit();
        delete Self;
    }

    return 0;
}

auto App::GetCtDataTree() const -> CtStructureTree& {
    return *CtDataTree;
}

auto App::GetPipelines() const -> PipelineList& {
    return *Pipelines;
}

auto App::GetThresholdFilter() const -> vtkImageAlgorithm& {
    return *ThresholdFilterAlgorithm;
}

auto App::GetPipelineGroups() const -> PipelineGroupList& {
    return *PipelineGroups;
}

auto App::GetPythonInterpreter() const -> PythonInterpreter& {
    return *PyInterpreter;
}

void App::InitializeWithTestData() {
    static DataInitializer::Config const config = DataInitializer::Config::SIMPLE_SCENE;

    DataInitializer initializer { *this };
    initializer(config);
}
