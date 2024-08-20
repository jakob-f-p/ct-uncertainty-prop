#pragma once

#include "../../PipelineGroups/Types.h"

#include <QSplitter>

struct PipelineBatchListData;

class PcaMainChartWidget;
class PcaSecondaryChartWidget;
class TsneMainChartWidget;


class ChartWidget : public QSplitter {
    Q_OBJECT

public:
    explicit ChartWidget(QWidget* parent = nullptr) noexcept;

    virtual auto
    UpdateData(PipelineBatchListData const* batchListData) -> void = 0;

Q_SIGNALS:
    void SamplePointChanged(std::optional<SampleId> sampleId);
};



class PcaChartWidget : public ChartWidget {
    Q_OBJECT

public:
    explicit PcaChartWidget(QWidget* parent = nullptr) noexcept;

    auto
    UpdateData(PipelineBatchListData const* batchListData) -> void override;

    auto
    SelectPcaPoints(QString const& name, QList<QPointF> const& points) -> void;

private:
    PcaMainChartWidget* PcaMainWidget;
    PcaSecondaryChartWidget* PcaSecondaryWidget;
};



class TsneChartWidget : public ChartWidget {
    Q_OBJECT

public:
    explicit TsneChartWidget(QWidget* parent = nullptr) noexcept;

    auto
    UpdateData(PipelineBatchListData const* batchListData) -> void override;

Q_SIGNALS:
    void PcaPointsSelected(QString const& name, QList<QPointF> const& points);

private:
    TsneMainChartWidget* TsneWidget;
};



