#pragma once

#include "../Utils/OptionalWidget.h"
#include "../Utils/RenderWidget.h"
#include "../../Utils/Types.h"

#include <QDockWidget>
#include <QMainWindow>

#include <memory>
#include <optional>

class QChart;
class QChartView;
class QPushButton;
class QValueAxis;

class CtDataSource;
class ParameterSpaceStateRenderWidget;
class PipelineGroupList;
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

class TsneChartWidget : public QWidget {
    Q_OBJECT

public:
    explicit TsneChartWidget();

    auto
    UpdateData(PipelineBatchListData const* batchListData) -> void;

Q_SIGNALS:
    void SamplePointChanged(std::optional<SampleId> sampleId);

private:
    [[nodiscard]] static auto
    GetAxisRange(QValueAxis* axis) -> std::pair<int, int>;

    [[nodiscard]] static auto
    GetAxisTickInterval(QValueAxis* axis) -> double;

    PipelineBatchListData const* BatchListData;
    QChartView* ChartView;
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
    QPushButton* ResetCameraButton;

    PipelineBatchListData const* BatchListData;
};
