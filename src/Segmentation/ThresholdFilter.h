#pragma once

#include <QWidget>

#include <vtkImageThreshold.h>

class QComboBox;
class QDoubleSpinBox;
class QFormLayout;

class ThresholdFilter : public vtkImageThreshold {
public:
    ThresholdFilter(const ThresholdFilter&) = delete;
    void operator=(const ThresholdFilter&) = delete;

    static ThresholdFilter* New();
    vtkTypeMacro(ThresholdFilter, vtkImageThreshold);

protected:
    ThresholdFilter();
    ~ThresholdFilter() override = default;

    void ThreadedRequestData(vtkInformation* request, vtkInformationVector** inputVector,
                             vtkInformationVector* outputVector, vtkImageData*** inData, vtkImageData** outData,
                             int outExt[6], int id) override;

    void PrepareImageData(vtkInformationVector** inputVector, vtkInformationVector* outputVector,
                          vtkImageData*** inDataObjects, vtkImageData** outDataObjects) override;
};



class ThresholdFilterWidget : public QWidget {
    Q_OBJECT

public:
    enum struct FilterMode : uint8_t {
        UPPER = 0,
        LOWER,
        BETWEEN
    };

    ThresholdFilterWidget();

    auto
    Populate(ThresholdFilter& thresholdFilter) -> void;

    auto
    SetFilterData(ThresholdFilter& thresholdFilter) -> void;

private Q_SLOTS:
    void UpdateSpinBoxVisibility(int idx);

private:
    [[nodiscard]] auto static
    FilterModeToString(FilterMode mode) -> std::string;

    [[nodiscard]] auto consteval static
    NumberOfFilterModes() noexcept -> uint8_t { return 3; }

    QFormLayout* FLayout;
    QComboBox* ModeComboBox;
    QDoubleSpinBox* LowerThresholdSpinBox;
    QDoubleSpinBox* UpperThresholdSpinBox;
};