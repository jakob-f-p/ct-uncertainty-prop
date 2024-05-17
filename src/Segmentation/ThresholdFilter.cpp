#include "ThresholdFilter.h"

#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFormLayout>

#include <vtkFloatArray.h>
#include <vtkImageData.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>

vtkStandardNewMacro(ThresholdFilter);

ThresholdFilter::ThresholdFilter() {
    ThresholdByUpper(500.0);
    ReplaceOut = 1;
    OutValue = -1001.0;
}

void ThresholdFilter::ThreadedRequestData(vtkInformation* request, vtkInformationVector** inputVector,
                                          vtkInformationVector* outputVector, vtkImageData*** inData,
                                          vtkImageData** outData, int* outExt, int id) {
    inData[0][0]->GetPointData()->SetActiveScalars("Radiodensities");

    vtkImageThreshold::ThreadedRequestData(request, inputVector, outputVector, inData, outData, outExt, id);
}

void ThresholdFilter::PrepareImageData(vtkInformationVector** inputVector, vtkInformationVector* outputVector,
                                       vtkImageData*** inDataObjects, vtkImageData** outDataObjects) {

    vtkInformation* info = inputVector[0]->GetInformationObject(0);
    vtkImageData* inData = vtkImageData::SafeDownCast(info->Get(vtkDataObject::DATA_OBJECT()));

    vtkNew<vtkFloatArray> segmentedRadiodensitiesArray;
    segmentedRadiodensitiesArray->SetNumberOfComponents(1);
    segmentedRadiodensitiesArray->SetName("Segmented Radiodensities");
    segmentedRadiodensitiesArray->SetNumberOfTuples(inData->GetNumberOfPoints());
    segmentedRadiodensitiesArray->FillValue(0.0F);
    inData->GetPointData()->AddArray(segmentedRadiodensitiesArray);
    inData->GetPointData()->SetActiveScalars("Segmented Radiodensities");

    vtkThreadedImageAlgorithm::PrepareImageData(inputVector, outputVector, inDataObjects, outDataObjects);
}

auto ThresholdFilterWidget::FilterModeToString(ThresholdFilterWidget::FilterMode mode) -> std::string {
    return [mode]() -> std::string {
        switch (mode) {
            case FilterMode::UPPER:   return "Upper";
            case FilterMode::LOWER:   return "Lower";
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

    for (uint8_t i = 0; i < NumberOfFilterModes(); i++) {
        auto const mode = static_cast<FilterMode>(i);
        ModeComboBox->addItem(QString::fromStdString(FilterModeToString(mode)),
                              QVariant::fromValue(mode));
    }

    LowerThresholdSpinBox->setRange(-1000.0, 2000.0);
    LowerThresholdSpinBox->setSingleStep(10.0);
    LowerThresholdSpinBox->setValue(400.0);

    UpperThresholdSpinBox->setRange(-1000.0, 2000.0);
    UpperThresholdSpinBox->setSingleStep(10.0);
    UpperThresholdSpinBox->setValue(1000.0);

    FLayout->addRow("Threshold Mode", ModeComboBox);
    FLayout->addRow("Lower Threshold", LowerThresholdSpinBox);
    FLayout->addRow("Upper Threshold", UpperThresholdSpinBox);

    connect(ModeComboBox, &QComboBox::currentIndexChanged, this, &ThresholdFilterWidget::UpdateSpinBoxVisibility);
    UpdateSpinBoxVisibility(0);
}

void ThresholdFilterWidget::UpdateSpinBoxVisibility(int idx) {
    auto const mode = ModeComboBox->currentData().value<FilterMode>();

    bool const upperVisible = mode != FilterMode::LOWER;
    bool const lowerVisible = mode != FilterMode::UPPER;

    FLayout->setRowVisible(LowerThresholdSpinBox, lowerVisible);
    FLayout->setRowVisible(UpperThresholdSpinBox, upperVisible);
}

auto ThresholdFilterWidget::Populate(ThresholdFilter& thresholdFilter) -> void {
    float const lower = static_cast<float>(thresholdFilter.GetLowerThreshold());
    float const upper = static_cast<float>(thresholdFilter.GetUpperThreshold());

    bool const hasLower = lower > VTK_FLOAT_MIN;
    bool const hasUpper = upper < VTK_FLOAT_MAX;

    if (!(hasUpper || hasLower))
        throw std::runtime_error("must have at least one bound");

    FilterMode const mode = hasLower
                                ? (hasUpper ? FilterMode::BETWEEN : FilterMode::LOWER)
                                : FilterMode::UPPER;
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
    float const lower = static_cast<float>(LowerThresholdSpinBox->value());
    float const upper = static_cast<float>(UpperThresholdSpinBox->value());

    auto const mode = ModeComboBox->currentData().value<FilterMode>();

    switch (mode) {
        case FilterMode::UPPER:   thresholdFilter.ThresholdByUpper(upper);        break;
        case FilterMode::LOWER:   thresholdFilter.ThresholdByLower(lower);        break;
        case FilterMode::BETWEEN: thresholdFilter.ThresholdBetween(lower, upper); break;
        default: break;
    }
}
