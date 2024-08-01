#pragma once

#include "CtDataSource.h"

#include <filesystem>


class NrrdCtDataSource : public CtDataSource {
public:
    static NrrdCtDataSource* New();
    vtkTypeMacro(NrrdCtDataSource, CtDataSource);
    void PrintSelf(ostream& os, vtkIndent indent) override;

    NrrdCtDataSource(const NrrdCtDataSource&) = delete;
    void operator=(const NrrdCtDataSource&) = delete;

    virtual auto
    SetFilepath(std::filesystem::path const& filename) noexcept -> void {
        if (Filename == filename)
            return;

        Filename = filename;

        Modified();
    }

    [[nodiscard]] virtual auto
    GetFilepath() const noexcept -> std::filesystem::path { return Filename; }

protected:
    NrrdCtDataSource() = default;
    ~NrrdCtDataSource() override = default;

    void ExecuteDataWithInformation(vtkDataObject *output, vtkInformation *outInfo) override;

    std::filesystem::path Filename;
};
