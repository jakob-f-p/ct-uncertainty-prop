#include "ImageArtifactsDelegate.h"
#include "../ImageArtifactDetails.h"

QString ImageArtifactsDelegate::displayText(const QVariant& value, const QLocale& locale) const {
    if (value.canConvert<ImageArtifactDetails>()) {
        return value.value<ImageArtifactDetails>().ViewName;
    }

    return QStyledItemDelegate::displayText(value, locale);
}
