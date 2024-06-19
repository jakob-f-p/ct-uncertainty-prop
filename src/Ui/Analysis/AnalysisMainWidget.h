#pragma once

#include "../Utils/OptionalWidget.h"
#include "../Utils/RenderWidget.h"
#include "../../PipelineGroups/Types.h"

#include <QMainWindow>

#include <memory>
#include <optional>

class QDoubleSpinBox;
class QPushButton;
class QSpinBox;

class AnalysisDataWidget;
class ChartWidget;
class CtDataSource;
class NameLineEdit;
class ParameterSpaceStateData;
class ParameterSpaceStateRenderWidget;
class PipelineGroupList;
class PipelineParameterSpaceStateView;

struct PipelineBatchListData;


class AnalysisMainWidget : public QMainWindow {
public:
    AnalysisMainWidget(PipelineGroupList const& pipelineGroups, CtDataSource& dataSource,
                       ChartWidget* chartWidget, AnalysisDataWidget* dataWidget);

    auto
    UpdateData() -> void;

private:
    PipelineGroupList const& GroupList;
    std::unique_ptr<std::optional<PipelineBatchListData>> BatchData;

    OptionalWidget<ChartWidget>* ChartWidget_;
    ParameterSpaceStateRenderWidget* RenderWidget;
    OptionalWidget<AnalysisDataWidget>* DataWidget;
};

class PcaMainWidget : public AnalysisMainWidget {
public:
    PcaMainWidget(PipelineGroupList const& pipelineGroups, CtDataSource& dataSource);
};

class TsneMainWidget : public AnalysisMainWidget {
public:
    TsneMainWidget(PipelineGroupList const& pipelineGroups, CtDataSource& dataSource);
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



class AnalysisDataWidget : public QWidget {
    Q_OBJECT

public:
    explicit AnalysisDataWidget(QString const& analysisName);

    auto
    UpdateData(PipelineBatchListData const* batchData) -> void;

Q_SIGNALS:
    void RequestResetCamera();

public Q_SLOTS:
    auto
    UpdateSample(std::optional<SampleId> sampleId) -> void;

protected:
    struct XYCoords {
        double X, Y;
    };

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

class PcaDataWidget : public AnalysisDataWidget {
public:
    PcaDataWidget();

protected:
    [[nodiscard]] auto
    GetXYData(ParameterSpaceStateData const& spaceStateData) const noexcept -> XYCoords override;
};

class TsneDataWidget : public AnalysisDataWidget {
public:
    TsneDataWidget();

protected:
    [[nodiscard]] auto
    GetXYData(ParameterSpaceStateData const& spaceStateData) const noexcept -> XYCoords override;
};
