#pragma once

#include "ImageArtifact.h"

class GaussianArtifact : public ImageArtifact {
public:
    static GaussianArtifact* New();

    SubType GetArtifactSubType() const override;

    QVariant Data() override;

    GaussianArtifact(const GaussianArtifact&) = delete;
    void operator=(const GaussianArtifact&) = delete;

protected:
    GaussianArtifact() = default;
    ~GaussianArtifact() override = default;
};