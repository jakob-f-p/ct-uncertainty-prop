#pragma once

#include <QTreeView>

class ImageArtifact;
//class ImageArtifactsReadOnlyModel;
class Pipeline;


class ImageArtifactsView : public QTreeView {
public:
    explicit ImageArtifactsView(Pipeline const* pipeline = nullptr, QWidget* parent = nullptr);

protected:
    void drawBranches(QPainter* painter, const QRect& rect, const QModelIndex& index) const override;

    static void iterate(const QModelIndex& index, const QAbstractItemModel* model,
                        const std::function<void(const QModelIndex&, int)>& f,
                        int depth = 0);

    auto hasHiddenIndices() const -> bool;

    static auto getLevel(const QModelIndex& index) -> int;
};


class ImageArtifactsReadOnlyView : public ImageArtifactsView {
    Q_OBJECT

public:
    explicit ImageArtifactsReadOnlyView(Pipeline const& pipeline, QWidget* parent = nullptr);

    auto
    Select(ImageArtifact const& imageArtifact) -> void;

Q_SIGNALS:
    void ImageArtifactChanged(ImageArtifact* imageArtifact);

protected:
    void selectionChanged(QItemSelection const& selected, QItemSelection const& deselected) override;
};
