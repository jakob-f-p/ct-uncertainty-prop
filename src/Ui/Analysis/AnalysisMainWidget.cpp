#include "AnalysisMainWidget.h"

#include "AnalysisDataWidget.h"
#include "ChartWidget.h"
#include "MainChartWidget.h"
#include "../../Modeling/CtDataSource.h"
#include "../../PipelineGroups/PipelineGroupList.h"
#include "../../App.h"

#include <QDockWidget>
#include <QSplitter>

#include <vtkFloatArray.h>
#include <vtkImageData.h>
#include <vtkPointData.h>
#include <vtkTypeInt16Array.h>

#include <span>


AnalysisMainWidget::AnalysisMainWidget(PipelineGroupList const& pipelineGroups,
                                       ChartWidget* chartWidget,
                                       AnalysisSampleDataWidget* dataWidget) :
        GroupList(pipelineGroups),
        BatchData([&pipelineGroups] {
            auto const batchData = pipelineGroups.GetBatchData();
            return batchData
                    ? std::make_unique<PipelineBatchListData>(std::move(*batchData))
                    : nullptr;
        }()),
        ChartWidget_(new OptionalWidget("Please generate the data first", chartWidget)),
        RenderWidget(new ParameterSpaceStateRenderWidget()),
        DataWidget(new OptionalWidget("Please select a sample point", dataWidget)) {

    auto* dockWidget = new QDockWidget();
    dockWidget->setFeatures(
            QDockWidget::DockWidgetFeature::DockWidgetMovable | QDockWidget::DockWidgetFeature::DockWidgetFloatable);
    dockWidget->setAllowedAreas(Qt::DockWidgetArea::LeftDockWidgetArea | Qt::DockWidgetArea::RightDockWidgetArea);
    dockWidget->setMinimumWidth(250);
    dockWidget->setContentsMargins({});
    DataWidget->setContentsMargins({ 10, 0, 10, 10 });
    dockWidget->setWidget(DataWidget);
    addDockWidget(Qt::DockWidgetArea::RightDockWidgetArea, dockWidget);

    auto* splitter = new QSplitter();
    splitter->setContentsMargins({});
    splitter->addWidget(ChartWidget_);
    splitter->addWidget(RenderWidget);
    splitter->setSizes({ std::numeric_limits<int>::max(), std::numeric_limits<int>::max() });
    setCentralWidget(splitter);

    ChartWidget_->setContentsMargins({ 15, 15, 15, 15 });

    connect(&ChartWidget_->Widget(), &ChartWidget::SamplePointChanged,
            RenderWidget, &ParameterSpaceStateRenderWidget::UpdateSample);

    connect(&ChartWidget_->Widget(), &ChartWidget::SamplePointChanged,
            this, [this](std::optional<SampleId> sampleId) {
        DataWidget->Widget().UpdateSample(sampleId);

        if (sampleId)
            DataWidget->ShowWidget();
        else
            DataWidget->HideWidget();
    });
}

AnalysisMainWidget::~AnalysisMainWidget() = default;

auto AnalysisMainWidget::UpdateData() -> void {
    auto const optionalBatchData = GroupList.GetBatchData();
    BatchData = optionalBatchData
            ? std::make_unique<PipelineBatchListData>(std::move(*optionalBatchData))
            : nullptr;

    PipelineBatchListData const* data = BatchData.get();
    RenderWidget->UpdateData(data);
    ChartWidget_->Widget().UpdateData(data);
    DataWidget->Widget().UpdateData(data);

    if (data)
        ChartWidget_->ShowWidget();
    else
        ChartWidget_->HideWidget();

    Q_EMIT ChartWidget_->Widget().SamplePointChanged(std::nullopt);
}

PcaMainWidget::PcaMainWidget(PipelineGroupList const& pipelineGroups) :
        AnalysisMainWidget(pipelineGroups, new PcaChartWidget(), new PcaSampleDataWidget()) {
    auto const* pcaChartWidget = findChild<PcaMainChartWidget*>();
    auto const* pcaDataWidget = findChild<PcaSampleDataWidget*>();

    connect(pcaChartWidget, &PcaMainChartWidget::PcaDataChanged, pcaDataWidget, &PcaSampleDataWidget::UpdateData);
}

PcaMainWidget::~PcaMainWidget() = default;

void PcaMainWidget::SelectPcaPoints(QString const& pointSetName, QList<QPointF> const& points) const {
    dynamic_cast<PcaChartWidget&>(ChartWidget_->Widget()).SelectPcaPoints(pointSetName, points);
}

TsneMainWidget::TsneMainWidget(PipelineGroupList const& pipelineGroups) :
        AnalysisMainWidget(pipelineGroups, new TsneChartWidget(), new TsneSampleDataWidget()) {

    connect(&dynamic_cast<TsneChartWidget&>(ChartWidget_->Widget()), &TsneChartWidget::PcaPointsSelected,
            this, &TsneMainWidget::PcaPointsSelected);
}

TsneMainWidget::~TsneMainWidget() = default;


ParameterSpaceStateRenderWidget::ParameterSpaceStateRenderWidget() :
        RenderWidget(nullptr, { true, true, WindowWidthSliderMode::ABOVE, true }),
        BatchListData(nullptr),
        DataSource(nullptr) {}

auto ParameterSpaceStateRenderWidget::UpdateData(PipelineBatchListData const* batchData) -> void {
    BatchListData = batchData;

    DataSource = &App::GetInstance().GetCtDataSource();

    UpdateSample(std::nullopt);
}

auto ParameterSpaceStateRenderWidget::UpdateSample(std::optional<SampleId> sampleId) -> void {
    if (!sampleId)
        UpdateImageAlgorithm(*DataSource);
    else {
        auto const& parameterSpaceStateData = BatchListData->GetSpaceStateData(*sampleId);

        CurrentImage = parameterSpaceStateData.ImageHandle.Read({ "Radiodensities", "Segmentation Mask" });

        auto const numberOfPoints = CurrentImage->GetNumberOfPoints();

        auto* radiodensityArray = vtkFloatArray::SafeDownCast(CurrentImage->GetPointData()->GetScalars());
        auto* maskArray = vtkTypeInt16Array::SafeDownCast(CurrentImage->GetPointData()->GetArray("Segmentation Mask"));

        if (!radiodensityArray || !maskArray)
            throw std::runtime_error("arrays may not be null");

        auto* radiodensities = radiodensityArray->WritePointer(0, numberOfPoints);
        auto* mask = maskArray->WritePointer(0, numberOfPoints);

        std::span const radiodensitySpan { radiodensities, std::next(radiodensities, numberOfPoints) };
        std::span const maskSpan { mask, std::next(mask, numberOfPoints) };

        std::transform(radiodensitySpan.begin(), radiodensitySpan.end(),
                       maskSpan.begin(),
                       radiodensitySpan.begin(),
                       [](float const& radiodensity, vtkTypeInt16 const& maskValue) {
            return maskValue == 0 ? -1000.0 : radiodensity;
        });

        UpdateImageAlgorithm(*CurrentImage);
    }

    Render();
}
