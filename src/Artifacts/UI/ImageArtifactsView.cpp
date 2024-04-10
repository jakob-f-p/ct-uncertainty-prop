#include "ImageArtifactsView.h"

#include "ImageArtifactsModel.h"
#include "ImageArtifactsDelegate.h"
#include "../CompositeArtifact.h"
#include "../ImageArtifact.h"
#include "../Pipeline.h"

#include <QPainter>

ImageArtifactsView::ImageArtifactsView(Pipeline* pipeline, QWidget* parent) : QTreeView(parent) {
    setIndentation(indentation() + 2);

    setHeaderHidden(true);

    setItemDelegate(new ImageArtifactsDelegate());

    if (pipeline)
        QTreeView::setModel(new ImageArtifactsModel(pipeline->GetImageArtifactConcatenation()));
}

void ImageArtifactsView::drawBranches(QPainter* painter, const QRect& rect, const QModelIndex& index) const {
    const int indent = indentation();
    const int outerMostLevel = rootIsDecorated() ? 0 : 1;
    int currentLevel = getLevel(index);
    QRect frame(rect.right() + 1, rect.top(), indent, rect.height());

    QModelIndex parent = index.parent();
    QModelIndex ancestor = parent.parent();

    QStyleOptionViewItem opt;
    initViewItemOption(&opt);

    QPoint oldBrushOrigin = painter->brushOrigin();
    QStyle::State extraFlags = QStyle::State_None;
    if (isEnabled())
        extraFlags |= QStyle::State_Enabled;
    if (hasFocus())
        extraFlags |= QStyle::State_Active;
    if (verticalScrollMode() == QAbstractItemView::ScrollPerPixel)
        painter->setBrushOrigin(QPoint(0, verticalOffset()));
    if (selectionModel()->isSelected(index))
        extraFlags |= QStyle::State_Selected;

    if (currentLevel >= outerMostLevel) {
        frame.moveLeft(frame.left() - indent);
        opt.rect = frame;

        const bool expanded = isExpanded(index);
        const bool children = index.model()->hasChildren(index);
        QModelIndex successorIndex = index.model()->sibling(index.row() + 1, 0, index);
        bool moreSiblings = successorIndex.isValid();

        opt.state = QStyle::State_Item | extraFlags
                    | (moreSiblings ? QStyle::State_Sibling : QStyle::State_None)
                    | (children ? QStyle::State_Children : QStyle::State_None)
                    | (expanded ? QStyle::State_Open : QStyle::State_None);

        style()->drawPrimitive(QStyle::PE_IndicatorBranch, &opt, painter, this);

        bool isRootLevel = !parent.isValid();
        bool parentIsSequentialComposition = parent.isValid()
                                             && static_cast<ImageArtifact*>(parent.internalPointer())->IsComposition()
                                             && static_cast<CompositeArtifact*>(parent.internalPointer())->GetCompType()
                                                == CompositeArtifact::CompositionType::SEQUENTIAL;
        if (isRootLevel || parentIsSequentialComposition) {
            QFont oldFont(painter->font());

            QFont smallFont(painter->font());
            smallFont.setPointSize(5);
            painter->setFont(smallFont);

            style()->drawItemText(painter, frame, Qt::AlignRight | Qt::AlignVCenter,
                                  palette(), isEnabled(), QString::number(index.row() + 1), foregroundRole());

            painter->setFont(oldFont);
        }
    }

    QModelIndex current = parent;
    for (--currentLevel; currentLevel >= outerMostLevel; --currentLevel) { // we have already drawn the innermost branch
        frame.moveLeft(frame.left() - indent);
        opt.rect = frame;
        opt.state = extraFlags;

        bool moreSiblings = false;
        if (!hasHiddenIndices()) {
            moreSiblings = (model()->rowCount(ancestor) - 1 > current.row());
        } else {
            QModelIndex successorIndex = index.model()->sibling(index.row() + 1, 0, index);
            while (successorIndex.isValid()) {
                if (getLevel(successorIndex) == uint(currentLevel)) {
                    moreSiblings = true;
                    break;
                }
                successorIndex = successorIndex.model()->sibling(successorIndex.row() + 1, 0, successorIndex);
            }
        }
        if (moreSiblings)
            opt.state |= QStyle::State_Sibling;

        style()->drawPrimitive(QStyle::PE_IndicatorBranch, &opt, painter, this);
        current = ancestor;
        ancestor = current.parent();
    }
    painter->setBrushOrigin(oldBrushOrigin);
}

void ImageArtifactsView::iterate(const QModelIndex& index, const QAbstractItemModel* model,
                                 const std::function<void(const QModelIndex&, int)>& f, int depth) const {
    if (index.isValid())
        f(index, depth);

    auto numberOfChildren = model->rowCount(index);
    for (int i = 0; i < numberOfChildren; ++i)
        iterate(model->index(i, 0, index), model, f, depth + 1);
}

bool ImageArtifactsView::hasHiddenIndices() const {
    int hiddenItems = 0;
    iterate(rootIndex(), model(), [&](const QModelIndex& index, int depth) {
        if (isIndexHidden(index))
            hiddenItems++;
    });
    return hiddenItems != 0;
}

int ImageArtifactsView::getLevel(const QModelIndex& index) {
    int level = 0;
    for (QModelIndex current = index.parent(); current.isValid(); current = current.parent())
        level++;

    return level;
}
