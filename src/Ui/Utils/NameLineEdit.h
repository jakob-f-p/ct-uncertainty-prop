#pragma once

#include <QLineEdit>

class NameLineEdit : public QLineEdit {
    Q_OBJECT

public:
    explicit NameLineEdit(QWidget* parent = nullptr) : QLineEdit(parent) {
        connect(this, &QLineEdit::textChanged, this, &NameLineEdit::TextChanged);
    }

    [[nodiscard]] auto
    GetText() noexcept -> QString { return text(); };

    auto
    SetText(QString const& name) noexcept -> void { setText(name); };

signals:
    void TextChanged(QString const& text);
};

