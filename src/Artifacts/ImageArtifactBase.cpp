#include "ImageArtifactBase.h"

#include "BasicImageArtifact.h"
#include "CompositeImageArtifact.h"
#include "../NameLineEdit.h"

#include <QComboBox>
#include <QFormLayout>

template struct ImageArtifactBaseDetails::ImageArtifactBaseData<BasicImageArtifactDetails::BasicImageArtifactDataVariant>;
template struct ImageArtifactBaseDetails::ImageArtifactBaseData<CompositeImageArtifactDetails::CompositeImageArtifactData>;

template<ImageArtifactBaseDetails::TArtifactData ArtifactData>
auto ImageArtifactBaseDetails::ImageArtifactBaseData<ArtifactData>::PopulateFromArtifact(const Artifact& artifact) noexcept -> void {
    Name = QString::fromStdString(artifact.GetName());
    ViewName = QString::fromStdString(artifact.GetViewName());
    Data.PopulateFromArtifact(artifact);
}

template<ImageArtifactBaseDetails::TArtifactData ArtifactData>
auto ImageArtifactBaseDetails::ImageArtifactBaseData<ArtifactData>::PopulateArtifact(Artifact& artifact) const noexcept -> void {
    artifact.Name = Name.toStdString();
    Data.PopulateArtifact(artifact);
}



template struct ImageArtifactBaseDetails::ImageArtifactBaseWidget<BasicImageArtifactDetails::BasicImageArtifactWidgetImpl,
                                                                  BasicImageArtifactData>;
template struct ImageArtifactBaseDetails::ImageArtifactBaseWidget<CompositeImageArtifactDetails::CompositeImageArtifactWidgetImpl,
                                                                  CompositeImageArtifactData>;

template<ImageArtifactBaseDetails::TArtifactWidget ArtifactWidget, typename ArtifactData>
ImageArtifactBaseDetails::ImageArtifactBaseWidget<ArtifactWidget, ArtifactData>::ImageArtifactBaseWidget() :
        Layout(new QFormLayout(this)),
        NameEdit(new NameLineEdit()),
        ArtifactEditWidget(new ArtifactWidget()) {

    Layout->setContentsMargins(0, 5, 0, 5);
    Layout->setFieldGrowthPolicy(QFormLayout::FieldGrowthPolicy::FieldsStayAtSizeHint);
    Layout->setHorizontalSpacing(15);

    Layout->addRow("Name", NameEdit);

    Layout->addRow(ArtifactEditWidget);
}

template<ImageArtifactBaseDetails::TArtifactWidget ArtifactWidget, typename ArtifactData>
auto ImageArtifactBaseDetails::ImageArtifactBaseWidget<ArtifactWidget, ArtifactData>::GetData() const noexcept -> ArtifactData {
    ArtifactData data {};

    data.Name = NameEdit->GetData();
    data.Data = ArtifactEditWidget->GetData();

    return data;
}

template<ImageArtifactBaseDetails::TArtifactWidget ArtifactWidget, typename ArtifactData>
auto ImageArtifactBaseDetails::ImageArtifactBaseWidget<ArtifactWidget, ArtifactData>::Populate(const ArtifactData& data) noexcept -> void {
    NameEdit->SetData(data.Name);
    ArtifactEditWidget->Populate(data.Data);
}
