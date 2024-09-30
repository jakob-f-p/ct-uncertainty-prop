#include "ThresholdFilter.h"

#include "../Utils/vtkImageDataArrayIterator.h"

#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFormLayout>

#include <vtkFloatArray.h>
#include <vtkImageData.h>
#include <vtkImageIterator.h>
#include <vtkImageProgressIterator.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkTypeInt16Array.h>

vtkStandardNewMacro(ThresholdFilter);

ThresholdFilter::ThresholdFilter() {
    ThresholdByUpper(500.0);
    ReplaceOut = 1;
    OutValue = -1001.0;
}

template<typename IT, typename OT>
void ThresholdFilterExecute(ThresholdFilter* self, vtkImageData* inData, vtkImageData* outData,
                            int outExt[6], int id, IT*, OT*) {

    vtkImageDataArrayIterator<IT> inIt(inData, outExt, "Radiodensities");
    vtkImageDataArrayIterator<vtkTypeInt16> maskIt(inData, outExt, "Segmentation Mask");
    vtkImageProgressIterator<OT> outIt(outData, outExt, self, id);

    int const replaceIn = self->GetReplaceIn();
    int const replaceOut = self->GetReplaceOut();

    IT const lowerThreshold = std::clamp(self->GetLowerThreshold(),
                                         inData->GetScalarTypeMin(), inData->GetScalarTypeMax());
    IT const upperThreshold = std::clamp(self->GetUpperThreshold(),
                                         inData->GetScalarTypeMin(), inData->GetScalarTypeMax());

    OT const inValue  = std::clamp(self->GetInValue(),  inData->GetScalarTypeMin(), inData->GetScalarTypeMax());
    OT const outValue = std::clamp(self->GetOutValue(), inData->GetScalarTypeMin(), inData->GetScalarTypeMax());

    for (; !outIt.IsAtEnd(); inIt.NextSpan(), maskIt.NextSpan(), outIt.NextSpan()) {
        IT* inSI = inIt.BeginSpan();
        OT* outSI = outIt.BeginSpan();
        vtkTypeInt16* maskSI = maskIt.BeginSpan();
        OT* outSIEnd = outIt.EndSpan();

        for (; outSI != outSIEnd; ++inSI, ++maskSI, ++outSI) {
            IT const value = (*inSI);
            bool const valueIsWithinBounds = lowerThreshold <= value && value <= upperThreshold;

            *maskSI = valueIsWithinBounds
                    ? 1
                    : 0;

            *outSI = valueIsWithinBounds
                    ? (replaceIn  ? inValue  : static_cast<OT>(value))
                    : (replaceOut ? outValue : static_cast<OT>(value));
        }
    }
}

template<typename T>
void ThresholdFilterExecute1(ThresholdFilter* self, vtkImageData* inData, vtkImageData* outData, int outExt[6],
                             int id, T*) {
    switch (outData->GetScalarType()) {
        vtkTemplateMacro(ThresholdFilterExecute(self, inData, outData, outExt, id,
                                                static_cast<T*>(nullptr), static_cast<VTK_TT*>(nullptr)));
        default:
            vtkGenericWarningMacro("Execute: Unknown input ScalarType");
    }
}

void ThresholdFilter::ThreadedRequestData(vtkInformation* vtkNotUsed(request),
                                          vtkInformationVector** vtkNotUsed(inputVector),
                                          vtkInformationVector* vtkNotUsed(outputVector),
                                          vtkImageData*** inData, vtkImageData** outData, int outExt[6], int id) {
    switch (inData[0][0]->GetScalarType()) {
        vtkTemplateMacro(ThresholdFilterExecute1(this, inData[0][0], outData[0], outExt, id,
                                                 static_cast<VTK_TT*>(nullptr)));
        default:
            vtkErrorMacro(<< "Execute: Unknown input ScalarType");
    }
}

void ThresholdFilter::PrepareImageData(vtkInformationVector** inputVector, vtkInformationVector* outputVector,
                                       vtkImageData*** inDataObjects, vtkImageData** outDataObjects) {

    vtkInformation* info = inputVector[0]->GetInformationObject(0);
    vtkImageData* inData = vtkImageData::SafeDownCast(info->Get(vtkDataObject::DATA_OBJECT()));

    vtkNew<vtkTypeInt16Array> segmentationMaskArray;
    segmentationMaskArray->SetNumberOfComponents(1);
    segmentationMaskArray->SetName("Segmentation Mask");
    segmentationMaskArray->SetNumberOfTuples(inData->GetNumberOfPoints());
    segmentationMaskArray->FillValue(0);
    inData->GetPointData()->AddArray(segmentationMaskArray);

    vtkNew<vtkFloatArray> segmentedRadiodensitiesArray;
    segmentedRadiodensitiesArray->SetNumberOfComponents(1);
    segmentedRadiodensitiesArray->SetName("Segmented Radiodensities");
    segmentedRadiodensitiesArray->SetNumberOfTuples(inData->GetNumberOfPoints());
    segmentedRadiodensitiesArray->FillValue(0.0F);
    inData->GetPointData()->AddArray(segmentedRadiodensitiesArray);
    inData->GetPointData()->SetActiveScalars("Segmented Radiodensities");

    vtkThreadedImageAlgorithm::PrepareImageData(inputVector, outputVector, inDataObjects, outDataObjects);

    inData->GetPointData()->SetActiveScalars("Radiodensities");
}

auto ThresholdFilterWidget::FilterModeToString(ThresholdFilterWidget::FilterMode mode) -> std::string {
    return [mode]() -> std::string {
        switch (mode) {
            case FilterMode::ABOVE:   return "Above";
            case FilterMode::BELOW:   return "Below";
            case FilterMode::BETWEEN: return "Between";
            default: throw std::runtime_error("invalid mode");
        }
    }();
}

ThresholdFilterWidget::ThresholdFilterWidget() :
        FLayout(new QFormLayout(this)),
        ModeComboBox(new QComboBox()),
        LowerThresholdSpinBox(new QDoubleSpinBox()),
        UpperThresholdSpinBox(new QDoubleSpinBox()) {

    FLayout->setHorizontalSpacing(20);
    FLayout->setContentsMargins({});

    for (uint8_t i = 0; i < NumberOfFilterModes(); i++) {
        auto const mode = static_cast<FilterMode>(i);
        ModeComboBox->addItem(QString::fromStdString(FilterModeToString(mode)),
                              QVariant::fromValue(mode));
    }

    LowerThresholdSpinBox->setRange(-1000.0, 3000.0);
    LowerThresholdSpinBox->setSingleStep(10.0);
    LowerThresholdSpinBox->setValue(-1000.0);

    UpperThresholdSpinBox->setRange(-1000.0, 3000.0);
    UpperThresholdSpinBox->setSingleStep(10.0);
    UpperThresholdSpinBox->setValue(3000.0);

    FLayout->addRow("Threshold Mode", ModeComboBox);
    FLayout->addRow("Lower Threshold", LowerThresholdSpinBox);
    FLayout->addRow("Upper Threshold", UpperThresholdSpinBox);

    connect(ModeComboBox, &QComboBox::currentIndexChanged, this, &ThresholdFilterWidget::UpdateSpinBoxVisibility);

    connect(LowerThresholdSpinBox, &QDoubleSpinBox::valueChanged, this, &ThresholdFilterWidget::DataChanged);
    connect(UpperThresholdSpinBox, &QDoubleSpinBox::valueChanged, this, &ThresholdFilterWidget::DataChanged);
    connect(ModeComboBox, &QComboBox::currentIndexChanged, this, &ThresholdFilterWidget::DataChanged);

    UpdateSpinBoxVisibility(0);
}

void ThresholdFilterWidget::UpdateSpinBoxVisibility(int /*idx*/) {
    auto const mode = ModeComboBox->currentData().value<FilterMode>();

    bool const lowerVisible = mode != FilterMode::BELOW;
    bool const upperVisible = mode != FilterMode::ABOVE;

    bool const visibilityChanged = FLayout->isRowVisible(LowerThresholdSpinBox) != lowerVisible
                                           || FLayout->isRowVisible(UpperThresholdSpinBox) != upperVisible;

    FLayout->setRowVisible(LowerThresholdSpinBox, lowerVisible);
    if (!lowerVisible)
        LowerThresholdSpinBox->setValue(LowerThresholdSpinBox->minimum());

    FLayout->setRowVisible(UpperThresholdSpinBox, upperVisible);
    if (!upperVisible)
        UpperThresholdSpinBox->setValue(UpperThresholdSpinBox->maximum());

    if (visibilityChanged)
        Q_EMIT DataChanged();
}

auto ThresholdFilterWidget::Populate(ThresholdFilter& thresholdFilter) -> void {
    auto const lower = static_cast<float>(thresholdFilter.GetLowerThreshold());
    auto const upper = static_cast<float>(thresholdFilter.GetUpperThreshold());

    bool const hasLower = lower > VTK_FLOAT_MIN;
    bool const hasUpper = upper < VTK_FLOAT_MAX;

    if (!(hasUpper || hasLower))
        throw std::runtime_error("must have at least one bound");

    FilterMode const mode = hasLower
                                ? (hasUpper ? FilterMode::BETWEEN : FilterMode::ABOVE)
                                : FilterMode::BELOW;
    int const modeIdx = ModeComboBox->findData(QVariant::fromValue(mode));
    if (modeIdx == -1)
        throw std::runtime_error("mode must be present");

    ModeComboBox->setCurrentIndex(modeIdx);

    if (hasLower)
        LowerThresholdSpinBox->setValue(lower);

    if (hasUpper)
        UpperThresholdSpinBox->setValue(upper);
}

auto ThresholdFilterWidget::SetFilterData(ThresholdFilter& thresholdFilter) -> void {
    auto const lower = static_cast<float>(LowerThresholdSpinBox->value());
    auto const upper = static_cast<float>(UpperThresholdSpinBox->value());

    auto const mode = ModeComboBox->currentData().value<FilterMode>();

    switch (mode) {
        case FilterMode::ABOVE:   thresholdFilter.ThresholdByUpper(lower); break;
        case FilterMode::BELOW:   thresholdFilter.ThresholdByLower(upper); break;
        case FilterMode::BETWEEN: thresholdFilter.ThresholdBetween(lower, upper); break;
        default: break;
    }
}
