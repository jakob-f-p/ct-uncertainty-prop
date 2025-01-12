#pragma once

#include "../Utils/OptionalWidget.h"
#include "../../PipelineGroups/ArtifactVariantPointer.h"

#include <QGroupBox>
#include <QSplitter>
#include <QWidget>

class CtStructureReadOnlyView;
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

    ArtifactVariantPointer CurrentArtifactPointer;

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

    [[nodiscard]] auto
    GetPipelineParameterSpan() const -> PipelineParameterSpan;

Q_SIGNALS:
    void Accept();
    void Reject();

private:
    QDialogButtonBox* DialogButtonBox;
};


class PipelineParameterSpanReadOnlyWidget : public PipelineParameterSpanWidget {
    Q_OBJECT

public:
    explicit PipelineParameterSpanReadOnlyWidget(Pipeline const& pipeline,
                                                 PipelineParameterSpan const& parameterSpan,
                                                 QWidget* parent = nullptr);
};


class PipelineArtifactsView : public QGroupBox {
    Q_OBJECT

public:
    explicit PipelineArtifactsView(Pipeline const& pipeline);

    auto
    SelectArtifact(ArtifactVariantPointer artifactPointer) const -> void;

Q_SIGNALS:
    void ArtifactChanged(ArtifactVariantPointer artifactVariantPointer);

private:
    enum View : uint8_t {
        IMAGE_ARTIFACTS = 0,
        STRUCTURE_ARTIFACTS = 1,
    };

    QComboBox* SelectViewComboBox;
    ImageArtifactsReadOnlyView* ImageArtifactsView;
    PipelineStructureArtifactsView* StructureArtifactsView;
};


class PipelineStructureArtifactsView : public QSplitter {
    Q_OBJECT

public:
    explicit PipelineStructureArtifactsView(Pipeline const& pipeline, QWidget* parent = nullptr);

    auto
    Select(StructureArtifact const& structureArtifact) -> void;

Q_SIGNALS:
    void StructureArtifactChanged(StructureArtifact* structureArtifact);

private:
    Pipeline const& APipeline;
    CtStructureReadOnlyView* StructuresView;
    OptionalWidget<StructureArtifactsReadOnlyView>* ArtifactsView;
};
