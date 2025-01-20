# Investigating the Propagation of CT Acquisition Artifacts along the Medical Imaging Pipeline
[Jakob Peischl](mailto:e12123459@student.tuwien.ac.at), [Renata G. Raidou](https://www.cg.tuwien.ac.at/staff/RenataRaidou)

<img src="assets/teaser-pipeline.png" alt="Workflow pipeline image" width="100%" style="max-width:1080px; margin:auto;"/>
<div>
 <img src="assets/teaser-pca.png" alt="Analysis PCA" width="58.5%" style="max-width:615px; margin:auto;"/>
 <img src="assets/teaser-tsne.png" alt="Analysis t-SNE" width="41%" style="max-width:432px; margin:auto;"/>
</div>

<img src="assets/tuwien-logo.svg" alt="TU Wien" width="25%" style="max-width:270px; display:block">

This repository contains the official implementation of the paper
"Investigating the Propagation of CT Acquisition Artifacts along the Medical Imaging Pipeline".

Abstract: *We propose a framework to support the simulation, exploration, and analysis of uncertainty propagation in
the medical imaging pipeline—exemplified with artifacts arising during CT acquisition. Uncertainty in the
acquired data can affect multiple subsequent stages of the medical imaging pipeline, as artifacts propagate
and accumulate along the latter, influencing the diagnostic power of CT and potentially introducing biases in
eventual decision-making processes. We designed and developed an interactive visual analytics framework that
simulates real-world CT artifacts using mathematical models, and empowers users to manipulate parameters
and observe their effects on segmentation outcomes. By extracting radiomics features from artifact-affected
segmented images and analyzing them using dimensionality reduction, we uncover distinct patterns related to
individual artifacts or combinations thereof. We demonstrate our proposed framework on use cases simulating
the effects of individual and combined artifacts on segmentation outcomes. Our application supports the
effective and flexible exploration and analysis of the impact of uncertainties on the outcomes of the medical
imaging pipeline. Initial insights into the nature and patterns of the simulated artifacts could also be derived.*

https://github.com/user-attachments/assets/843fb424-93d6-45a4-9239-2377be7dc947

Download [video in original quality](https://github.com/jakob-f-p/ct-uncertainty-prop/blob/main/assets/video.mp4).


## Running the code

The project uses two languages that interact with each other. Most of the application is written in C++ while data
processing is done using Python. For running the code, you need
 - C++ Compiler with full C++20 support
 - C++ Dependencies
   - Manual installation (configuration in `src/CMakeLists.txt` file might need adaptations):
      - Qt 6.6.2 (with modules Core, GUI, Widgets, and Charts)
      - VTK 9.3.0 with Qt Support
      - HDF5 with zlib support
   - The dependencies listed in the `/deps` directory get installed automatically by CMake when running the
   application for the first time.
 - Python 3.9 (strict minor version)
 - Python Dependencies:
   - PyRadiomics 3.1.0: Either install the official version using `pip install 'pyradiomics==3.1.0'`
   or install the custom performance-optimized version in the `/external` directory of this project.
   For more information, please read the `README.md` file there.

These are the dependencies the system was last tested with. Newer minor releases will probably also work.

Unfortunately, dependency management through CMake can be fragile.
So some things might not work immediately on some systems.
The dependencies are configured in the respective `CMakeList.txt` files in the `/src` directory
and in the `/deps/**` directories of this project.

## BibTeX
Paper:
```
TBD
```

Code:
```
TBD
```
