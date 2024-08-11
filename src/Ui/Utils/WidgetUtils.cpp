#include "WidgetUtils.h"


auto DialogDelegate::createEditor(QWidget* parent,
                                  QStyleOptionViewItem const& /*option*/,
                                  QModelIndex const& index) const -> QWidget* {
    if (!index.isValid())
        return nullptr;

    auto* dialog = getDialog(index, parent);

    connect(dialog, &QDialog::accepted, this, &DialogDelegate::commitEdit);
    connect(dialog, &QDialog::rejected, this, &DialogDelegate::discardChanges);

    return dialog;
}

auto DialogDelegate::updateEditorGeometry(QWidget* editor,
                                          QStyleOptionViewItem const& option,
                                          QModelIndex const& index) const -> void {}

auto DialogDelegate::commitEdit() -> void {
    auto* ctStructureEditDialog = qobject_cast<QDialog*>(sender());

    Q_EMIT commitData(ctStructureEditDialog);
    Q_EMIT closeEditor(ctStructureEditDialog);
}

auto DialogDelegate::discardChanges() -> void {
    auto* ctStructureEditDialog = qobject_cast<QDialog*>(sender());

    Q_EMIT closeEditor(ctStructureEditDialog);
}

auto DialogDelegate::eventFilter(QObject* /*object*/, QEvent* /*event*/) -> bool { return false; }

auto GenerateIcon(const std::string &fileNamePrefix) noexcept -> QIcon {
    QIcon icon;
    QString const qFilePrefix = QString::fromStdString(fileNamePrefix);
    icon.addPixmap(QPixmap(":/" + qFilePrefix + "Normal.png"), QIcon::Normal);
    icon.addPixmap(QPixmap(":/" + qFilePrefix + "Disabled.png"), QIcon::Disabled);
    return icon;
}

auto GenerateSimpleIcon(const std::string &fileName) noexcept -> QIcon {
    return { QPixmap(":/" + QString::fromStdString(fileName) + ".png") };
}

auto GetHeader1StyleSheet() noexcept -> QString {
    return QString("font-size: %1px; font-weight: bold").arg(QApplication::font().pointSize() * 1.6);
}

auto GetHeader2StyleSheet() noexcept -> QString {
    return QString("font-size: %1px; font-weight: bold").arg(QApplication::font().pointSize() * 1.4);
}

auto GetHeader3StyleSheet() noexcept -> QString {
    return QString("font-size: %1px; font-weight: bold").arg(QApplication::font().pointSize() * 1.2);
}
