#pragma once

#include <QFormLayout>

class NoMarginFormLayout : public QFormLayout {
public:
    NoMarginFormLayout(QWidget* parent = nullptr) : QFormLayout(parent) {
        setContentsMargins(0, 0, 0, 0);
    }
};