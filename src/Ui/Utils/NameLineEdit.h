#pragma once

#include <QLineEdit>

class NameLineEdit : public QLineEdit {
    Q_OBJECT

public:
    explicit NameLineEdit(QWidget* parent = nullptr) : QLineEdit(parent) {
        connect(this, &QLineEdit::textChanged, this, &NameLineEdit::TextChanged);
    }

    [[nodiscard]] auto
    GetText() const noexcept -> QString { return text(); }

    auto
    SetText(QString const& name) noexcept -> void { setText(name); }

Q_SIGNALS:
    void TextChanged(QString const& text);
};

