The .zip archive contains a modified version of the pyradiomics v3.1.0 Python library.
The modifications pertain the C++ (.cpp and .h; previously .c and .h) files of the library.
More specifically, an improvement was made to the algorithm that calculates the 2D/3D diameter features,
i.e. the longest distances between any points within a given segmentation mask. The new tree-based algorithm
runs in O(n log n) while the previous algorithm had a runtime of O(n²). This modification leads to a significant
performance improvement which might be desired depending on the size of the processed input data.

There are a few lines you might need to adapt to your local system. These are marked with a comment `# adapt`.
 - You will certainly need to modify the file paths in the `CMakeLists.txt` file to point to your local `site-packages/numpy/_core/include` folder and to your local Python 3.9 `/include` folder.
 - You may need to update the syntax for the `extra_compile_args` argument in the `setup.py` file to match the command line syntax of your system and C++ compiler (unless you are using MSVC).
