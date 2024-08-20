#pragma once

#include "../Utils/OptionalWidget.h"
#include "../../PipelineGroups/Types.h"

#include <QWidget>

class AnalysisSampleDataWidget;
class NameLineEdit;
class PipelineParameterSpaceStateView;

struct ParameterSpaceStateData;
struct PipelineBatchListData;

class QDoubleSpinBox;
class QSpinBox;
class QVBoxLayout;



class AnalysisSampleDataWidget : public QWidget {
    Q_OBJECT

public:
    explicit AnalysisSampleDataWidget(QString const& analysisName);

    auto
    UpdateData(PipelineBatchListData const* batchData) -> void;

public Q_SLOTS:
    auto
    UpdateSample(std::optional<SampleId> sampleId) -> void;

protected:
    struct XYCoords { double X, Y; };

    [[nodiscard]] virtual auto
    GetXYData(ParameterSpaceStateData const& spaceStateData) const noexcept -> XYCoords = 0;

private:
    QString AnalysisName;
    PipelineBatchListData const* BatchListData;

    NameLineEdit* PipelineGroupNameEdit;
    NameLineEdit* BasePipelineNameEdit;
    QSpinBox* NumberOfGroupPipelinesSpinBox;
    QDoubleSpinBox* PointXSpinBox;
    QDoubleSpinBox* PointYSpinBox;
    OptionalWidget<PipelineParameterSpaceStateView>* ParameterSpaceStateView;
};

class PcaSampleDataWidget : public AnalysisSampleDataWidget {
public:
    PcaSampleDataWidget();

protected:
    [[nodiscard]] auto
    GetXYData(ParameterSpaceStateData const& spaceStateData) const noexcept -> XYCoords override;
};

class TsneSampleDataWidget : public AnalysisSampleDataWidget {
public:
    TsneSampleDataWidget();

protected:
    [[nodiscard]] auto
    GetXYData(ParameterSpaceStateData const& spaceStateData) const noexcept -> XYCoords override;
};
