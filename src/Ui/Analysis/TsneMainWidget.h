#pragma once

#include "../Utils/OptionalWidget.h"
#include "../Utils/RenderWidget.h"
#include "../../Utils/Types.h"

#include <QMainWindow>

#include <memory>
#include <optional>

class QDoubleSpinBox;
class QPushButton;
class QSpinBox;

class CtDataSource;
class NameLineEdit;
class ParameterSpaceStateRenderWidget;
class PipelineGroupList;
class PipelineParameterSpaceStateView;
class TsneChartWidget;
class TsneDataWidget;

struct PipelineBatchListData;


class TsneMainWidget : public QMainWindow {
public:
    explicit TsneMainWidget(PipelineGroupList const& pipelineGroups, CtDataSource& dataSource);

    auto
    UpdateData() -> void;

private:
    PipelineGroupList const& GroupList;
    std::unique_ptr<std::optional<PipelineBatchListData>> BatchData;

    OptionalWidget<TsneChartWidget>* ChartWidget;
    ParameterSpaceStateRenderWidget* RenderWidget;
    OptionalWidget<TsneDataWidget>* DataWidget;
};


class ParameterSpaceStateRenderWidget : public RenderWidget {
    Q_OBJECT

public:
    explicit ParameterSpaceStateRenderWidget(CtDataSource& dataSource);

    auto
    UpdateData(PipelineBatchListData const* batchData) -> void;

public Q_SLOTS:
    auto
    UpdateSample(std::optional<SampleId> sampleId) -> void;

private:
    PipelineBatchListData const* BatchListData;
    CtDataSource& DataSource;
};


class TsneDataWidget : public QWidget {
    Q_OBJECT

public:
    explicit TsneDataWidget();

    auto
    UpdateData(PipelineBatchListData const* batchData) -> void;

Q_SIGNALS:
    void RequestResetCamera();

public Q_SLOTS:
    auto
    UpdateSample(std::optional<SampleId> sampleId) -> void;

private:
    PipelineBatchListData const* BatchListData;

    QPushButton* ResetCameraButton;

    NameLineEdit* PipelineGroupNameEdit;
    NameLineEdit* BasePipelineNameEdit;
    QSpinBox* NumberOfGroupPipelinesSpinBox;

    QDoubleSpinBox* PointXSpinBox;
    QDoubleSpinBox* PointYSpinBox;

    OptionalWidget<PipelineParameterSpaceStateView>* ParameterSpaceStateView;
};
