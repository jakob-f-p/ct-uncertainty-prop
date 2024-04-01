#include "App.h"

#include "MainWindow.h"
#include "Modeling/CtStructureTree.h"
#include "Modeling/BasicStructure.h"
#include "Artifacts/GaussianArtifact.h"
#include "Artifacts/ImageArtifactConcatenation.h"
#include "Artifacts/ImageArtifactComposition.h"
#include "Artifacts/PipelineList.h"
#include "Artifacts/Pipeline.h"

#include <QSurfaceFormat>
#include <QVTKOpenGLNativeWidget.h>

#include <SMP/Common/vtkSMPToolsAPI.h>

App* App::Self = nullptr;

App* App::CreateInstance(int argc, char* argv[]) {
    if (Self) {
        qWarning("App already exists. Cannot create new instance.");
        return nullptr;
    }

    Self = new App(argc, argv);
    return Self;
}

App* App::GetInstance() {
    if (!Self) {
        qWarning("No instance exists. Instance needs to be created first.");
        return nullptr;
    }
    return Self;
}

App::App(int argc, char* argv[]) :
        Argc(argc),
        Argv(argv),
        QApp(*new QApplication(Argc, Argv)),
        CtDataTree(*CtStructureTree::New()),
        Pipelines(*PipelineList::New()),
        MainWin(nullptr) {
}

App::~App() {
    CtDataTree.Delete();
    Pipelines.Delete();
    delete &MainWin;

    QApplication::quit();
    delete &QApp;
}

int App::Run() {
    QSurfaceFormat::setDefaultFormat(QVTKOpenGLNativeWidget::defaultFormat());

    auto& smpToolsApi =  vtk::detail::smp::vtkSMPToolsAPI::GetInstance();
//    smpToolsApi.SetBackend("STDTHREAD");
    qWarning(("Backend: " + std::string(smpToolsApi.GetBackend())).c_str());

    InitializeWithTestData();

    MainWin = new MainWindow();
    MainWin->show();

    return QApplication::exec();
}

int App::Quit() {
    if (Self) {
        QApplication::quit();
        delete Self;
    }

    return 0;
}

void App::InitializeWithTestData() {
    vtkNew<BasicStructure> basicStructure1;
    basicStructure1->SetImplicitFunction(BasicStructure::ImplicitFunctionType::SPHERE);
    basicStructure1->SetTissueType(BasicStructure::GetTissueOrMaterialTypeByName("Cancellous Bone"));
    basicStructure1->SetTransform({ 40.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f });

    vtkNew<BasicStructure> basicStructure2;
    basicStructure2->SetImplicitFunction(BasicStructure::ImplicitFunctionType::BOX);
    basicStructure2->SetTissueType(BasicStructure::GetTissueOrMaterialTypeByName("Cortical Bone"));

//    vtkNew<BasicStructure> basicStructure3;
//    basicStructure3->SetImplicitFunction(BasicStructure::ImplicitFunctionType::BOX);
//    basicStructure3->SetTissueType(BasicStructure::GetTissueOrMaterialTypeByName("Soft Tissue"));

    CtDataTree.AddBasicStructure(*basicStructure1);
    CtDataTree.CombineWithBasicStructure(*basicStructure2, CombinedStructure::OperatorType::INTERSECTION);
//    CtDataTree->CombineWithBasicStructure(*basicStructure3, CombinedStructure::OperatorType::UNION);
//    CtDataTree->RemoveBasicStructure(*basicStructure2);
//    CtDataTree->RefineWithBasicStructure({ "a", "", {}, {}, "Water", {}}, *basicStructure3);

    vtkNew<Pipeline> pipeline;
    pipeline->SetCtDataTree(&CtDataTree);
    Pipelines.AddPipeline(pipeline);
    ImageArtifactConcatenation& imageArtifactConcatenation = pipeline->GetImageArtifactConcatenation();

    vtkNew<GaussianArtifact> gaussianArtifact;
    gaussianArtifact->SetName("sequential gaussian");
    imageArtifactConcatenation.AddImageArtifact(*gaussianArtifact);

    vtkNew<ImageArtifactComposition> imageArtifactComposition;
    imageArtifactComposition->SetCompType(ImageArtifactComposition::CompositionType::PARALLEL);
    vtkNew<ImageArtifactComposition> imageArtifactComposition1;
    imageArtifactComposition1->SetCompType(ImageArtifactComposition::CompositionType::SEQUENTIAL);
    vtkNew<GaussianArtifact> gaussianArtifact1;
    imageArtifactComposition->AddImageArtifact(*imageArtifactComposition1);
    imageArtifactComposition->AddImageArtifact(*gaussianArtifact1);
    imageArtifactConcatenation.AddImageArtifact(*imageArtifactComposition);

    vtkNew<GaussianArtifact> gaussianArtifact2;
    vtkNew<GaussianArtifact> gaussianArtifact3;
    imageArtifactComposition1->AddImageArtifact(*gaussianArtifact2);
    imageArtifactComposition1->AddImageArtifact(*gaussianArtifact3);

    vtkNew<GaussianArtifact> gaussianArtifact4;
    imageArtifactConcatenation.AddImageArtifact(*gaussianArtifact4);
}

CtStructureTree& App::GetCtDataTree() const {
    return CtDataTree;
}

PipelineList& App::GetPipelines() const {
    return Pipelines;
}
