#pragma once

#include <QStyledItemDelegate>

class ImageArtifactsDelegate : public QStyledItemDelegate {
    Q_OBJECT

public:
    QString displayText(const QVariant& value, const QLocale& locale) const override;

protected:
//    bool eventFilter(QObject *object, QEvent *event) override;
};
