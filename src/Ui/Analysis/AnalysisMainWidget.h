#pragma once

#include "../Utils/OptionalWidget.h"
#include "../Utils/RenderWidget.h"
#include "../../PipelineGroups/Types.h"

#include <QMainWindow>

#include <memory>

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
    ~AnalysisMainWidget() override;

    auto
    UpdateData() -> void;

protected:
    OptionalWidget<ChartWidget>* ChartWidget_;

private:
    PipelineGroupList const& GroupList;
    std::unique_ptr<PipelineBatchListData> BatchData;

    ParameterSpaceStateRenderWidget* RenderWidget;
    OptionalWidget<AnalysisDataWidget>* DataWidget;
};

class PcaMainWidget : public AnalysisMainWidget {
    Q_OBJECT

public:
    PcaMainWidget(PipelineGroupList const& pipelineGroups, CtDataSource& dataSource);
    ~PcaMainWidget() override;

public Q_SLOTS:
    void SelectPcaPoints(QString const& pointSetName, QList<QPointF> const& points);
};

class TsneMainWidget : public AnalysisMainWidget {
    Q_OBJECT

public:
    TsneMainWidget(PipelineGroupList const& pipelineGroups, CtDataSource& dataSource);
    ~TsneMainWidget() override;

Q_SIGNALS:
    void PcaPointsSelected(QString const& pointSetName, QList<QPointF> const& points);
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
    vtkSmartPointer<vtkImageData> CurrentImage;
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
