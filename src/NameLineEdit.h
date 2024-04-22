#pragma once

#include <QLineEdit>

class NameLineEdit : public QLineEdit {
    Q_OBJECT

public:
    [[nodiscard]] auto
    GetData() noexcept -> QString { return text(); };

    auto
    SetData(const QString& name) noexcept -> void { setText(name); };
};

