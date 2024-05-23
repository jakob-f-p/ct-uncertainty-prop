#pragma once

#include <QTreeView>

class ImageArtifact;
class ImageArtifactsReadOnlyModel;
class Pipeline;


class ImageArtifactsView : public QTreeView {
public:
    explicit ImageArtifactsView(Pipeline* pipeline = nullptr, QWidget* parent = nullptr);

protected:
    void drawBranches(QPainter* painter, const QRect& rect, const QModelIndex& index) const override;

    void iterate(const QModelIndex& index, const QAbstractItemModel* model,
                 const std::function<void(const QModelIndex&, int)>& f,
                 int depth = 0) const;

    auto hasHiddenIndices() const -> bool;

    static auto getLevel(const QModelIndex& index) -> int;
};


class ImageArtifactsReadOnlyView : public ImageArtifactsView {
    Q_OBJECT

public:
    explicit ImageArtifactsReadOnlyView(Pipeline const& pipeline, QWidget* parent = nullptr);

    [[nodiscard]] auto
    model() const noexcept -> ImageArtifactsReadOnlyModel*;

signals:
    void ImageArtifactChanged(ImageArtifact* imageArtifact);

private slots:
    void OnSelectionChanged(QItemSelection const& selected, QItemSelection const& deselected);

private:
    ImageArtifactsReadOnlyModel* ArtifactsModel;
};