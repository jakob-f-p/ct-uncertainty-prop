#pragma once

#include <QTreeView>
#include <QWidget>

class ImageArtifactsView : public QTreeView {
public:
    explicit ImageArtifactsView(QWidget* parent = nullptr);

protected:
    void drawBranches(QPainter* painter, const QRect& rect, const QModelIndex& index) const override;

    void iterate(const QModelIndex& index, const QAbstractItemModel* model,
                 const std::function<void(const QModelIndex&, int)>& f,
                 int depth = 0) const;

    bool hasHiddenIndices() const;

    static int getLevel(const QModelIndex& index) ;
};
