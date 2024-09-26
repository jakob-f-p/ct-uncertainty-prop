#include "App.h"

#include "Artifacts/PipelineList.h"
#include "DataInitializer.h"
#include "Modeling/ImplicitCtDataSource.h"
#include "Modeling/CtStructureTree.h"
#include "Modeling/NrrdCtDataSource.h"
#include "PipelineGroups/PipelineGroupList.h"
#include "PipelineGroups/PipelineParameterSpan.h"
#include "Segmentation/ThresholdFilter.h"
#include "Ui/MainWindow.h"
#include "Utils/PythonInterpreter.h"

#include <QApplication>
#include <QSurfaceFormat>

#include <QVTKOpenGLNativeWidget.h>

#include <SMP/Common/vtkSMPToolsAPI.h>

#include <spdlog/spdlog.h>


App::App(int argc, char* argv[]) :
        Argc(argc),
        Argv(argv),
        QApp(std::make_unique<QApplication>(Argc, Argv)),
        CtDataTree(new CtStructureTree()),
        DataSource([this](){
            vtkNew<ImplicitCtDataSource> dataSource;
            dataSource->SetDataTree(CtDataTree.get());
            return dataSource;
        }()),
        Pipelines(new PipelineList(*CtDataTree)),
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
        PyInterpreter(std::make_unique<PythonInterpreter>()) {

    QSurfaceFormat::setDefaultFormat(QVTKOpenGLNativeWidget::defaultFormat());

#ifndef BUILD_TYPE_DEBUG
    auto& smpToolsApi =  vtk::detail::smp::vtkSMPToolsAPI::GetInstance();
    smpToolsApi.SetBackend("STDTHREAD");  // leaks a small, constant amount of memory
#endif
}

App::~App() {
    Self = nullptr;
};

App* App::Self = nullptr;

auto App::CreateInstance(int argc, char* argv[]) -> App* {
    if (Self)
        throw std::runtime_error("App already exists. Cannot create new instance.");

    Self = new App(argc, argv);
    return Self;
}

auto App::GetInstance() -> App& {
    if (!Self)
        throw std::runtime_error("No instance exists. Instance needs to be created first.");

    return *Self;
}

auto App::Run() -> int {
    auto& smpToolsApi =  vtk::detail::smp::vtkSMPToolsAPI::GetInstance();
    spdlog::debug("Running backend '{}'", smpToolsApi.GetBackend());

    spdlog::debug("Creating Ui...");

    auto const mode = MainWindow::Mode::PRESENTATION;
    MainWindow_ = std::make_unique<MainWindow>(*CtDataTree, *ThresholdFilterAlgorithm,
                                               *Pipelines, *PipelineGroups, mode);
//    MainWindow mainWindow(*CtDataTree, *ThresholdFilterAlgorithm, *Pipelines, *PipelineGroups, mode);

    spdlog::debug("Initializing with test data...");

    InitializeWithTestData();

    spdlog::info("Application running...");

    MainWindow_->show();

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

auto App::GetCtDataSource() const -> CtDataSource& {
    return *DataSource;
}

auto App::SetCtDataSource(CtDataSource& ctDataSource) -> void {
    ctDataSource.Modified();
    DataSource = &ctDataSource;
    MainWindow_->UpdateDataSource(ctDataSource);
}

auto App::GetCtDataSourceType() const -> App::CtDataSourceType {
    if (auto* implicitSource = dynamic_cast<ImplicitCtDataSource*>(DataSource.Get()))
        return CtDataSourceType::IMPLICIT;

    if (auto* importedSource = dynamic_cast<NrrdCtDataSource*>(DataSource.Get()))
        return CtDataSourceType::IMPORTED;

    throw std::runtime_error("Invalid data source type");
}

auto App::GetImageDimensions() const -> std::array<uint32_t, 3> {
    if (!DataSource)
        throw std::runtime_error("data source does not exist");

    auto const intDims = DataSource->GetVolumeNumberOfVoxels();

    std::array<uint32_t, 3> uintDims {};
    std::transform(intDims.cbegin(), intDims.cend(),
                   uintDims.begin(),
                   [](int n) { return static_cast<uint32_t>(n); });

    return uintDims;
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

auto App::GetPythonInterpreter() -> PythonInterpreter& {
    return *PyInterpreter;
}

void App::InitializeWithTestData() {
    static constexpr DataInitializer::Config config = DataInitializer::Config::SCENARIO_IMPORTED;

    DataInitializer initializer { *this };
    initializer(config);
}
