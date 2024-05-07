#pragma once

#include "../../Utils/WidgetUtils.h"

class ImageArtifactsDelegate : public DialogDelegate {
public:
    auto
    getDialog(const QModelIndex& index, QWidget* parent) const noexcept -> QDialog* override;

    void setEditorData(QWidget* editor, const QModelIndex& index) const override;

    void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override;
};
