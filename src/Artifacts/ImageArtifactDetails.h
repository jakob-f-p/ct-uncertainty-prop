#pragma once

#include "GaussianArtifact.h"
#include "ImageArtifactComposition.h"

struct ImageArtifactDetails : ArtifactDetails {
    QString ViewName;
    ImageArtifactCompositionDetails Composition;
    GaussianArtifactDetails Gaussian;

    ImageArtifactDetails(const ArtifactDetails& artifact,
                         QString viewName,
                         ImageArtifactCompositionDetails composition,
                         GaussianArtifactDetails gaussian) :
            ArtifactDetails(artifact),
            ViewName(std::move(viewName)),
            Composition(composition),
            Gaussian(gaussian) {
    };

    ImageArtifactDetails(const ImageArtifactDetails& artifact,
                         ImageArtifactCompositionDetails composition,
                         GaussianArtifactDetails gaussian) :
            ArtifactDetails(artifact),
            ViewName(artifact.ViewName),
            Composition(composition),
            Gaussian(gaussian) {
    };

    ImageArtifactDetails(const ArtifactDetails& artifact,
                         ImageArtifactCompositionDetails composition,
                         GaussianArtifactDetails gaussian) :
            ArtifactDetails(artifact),
            Composition(composition),
            Gaussian(gaussian) {
    };

    ImageArtifactDetails() :
            ArtifactDetails(),
            Composition(),
            Gaussian() {
    }
};
