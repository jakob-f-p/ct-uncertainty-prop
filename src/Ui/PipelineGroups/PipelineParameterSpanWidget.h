#pragma once

#include "../Utils/OptionalWidget.h"
#include "../../PipelineGroups/ArtifactVariantPointer.h"

#include <QSplitter>
#include <QWidget>

class CtStructureTree;
class CtStructureReadOnlyView;
class FloatParameterSpanWidget;
class FloatPointParameterSpanWidget;
class ImageArtifactsReadOnlyView;
class NameLineEdit;
class ObjectPropertyGroup;
class Pipeline;
class PipelineArtifactsView;
class PipelineParameterSpan;
class PipelineStructureArtifactsView;
class StructureArtifactsReadOnlyView;

class QComboBox;
class QDialogButtonBox;
class QFormLayout;
class QSpinBox;


class PipelineParameterSpanWidget : public QWidget {
protected:
    explicit PipelineParameterSpanWidget(Pipeline const& pipeline, QWidget* parent = nullptr);

    QFormLayout* FLayout;
    NameLineEdit* NameEdit;
    QSpinBox* NumberOfPipelinesSpinBox;
    PipelineArtifactsView* ArtifactsView;
    OptionalWidget<ObjectPropertyGroup>* PropertyGroup;
};


class PipelineParameterSpanCreateWidget : public PipelineParameterSpanWidget {
    Q_OBJECT

public:
    explicit PipelineParameterSpanCreateWidget(Pipeline const& pipeline, QWidget* parent = nullptr);

    struct Data {
        QString Name;
        ArtifactVariantPointer ArtifactPointer;
    };

    [[nodiscard]] auto
    GetData() const noexcept -> Data;

signals:
    void Accept();
    void Reject();

private:
    QDialogButtonBox* DialogButtonBox;
};

class PipelineParameterSpanReadOnlyWidget : public PipelineParameterSpanWidget {
    Q_OBJECT

public:
    explicit PipelineParameterSpanReadOnlyWidget(Pipeline const& pipeline,
                                                 PipelineParameterSpan& parameterSpan,
                                                 ArtifactVariantPointer artifactVariantPointer,
                                                 QWidget* parent = nullptr);

private:
    QDialogButtonBox* DialogButtonBox;
};


class PipelineArtifactsView : public QStackedWidget {
    Q_OBJECT

public:
    explicit PipelineArtifactsView(Pipeline const& pipeline);

    enum View : uint8_t {
        IMAGE_ARTIFACTS = 0,
        STRUCTURE_ARTIFACTS = 1,
    };

signals:
    void ArtifactChanged(ArtifactVariantPointer artifactVariantPointer);
    
public slots:
    void Show(View view);

private:
    ImageArtifactsReadOnlyView* ImageArtifactsView;
    PipelineStructureArtifactsView* StructureArtifactsView;
};


class PipelineStructureArtifactsView : public QSplitter {
    Q_OBJECT

public:
    explicit PipelineStructureArtifactsView(Pipeline const& pipeline, QWidget* parent = nullptr);

signals:
    void StructureArtifactChanged(StructureArtifact* structureArtifact);

private:
    Pipeline const& APipeline;
    CtStructureReadOnlyView* StructuresView;
    OptionalWidget<StructureArtifactsReadOnlyView>* ArtifactsView;
};
