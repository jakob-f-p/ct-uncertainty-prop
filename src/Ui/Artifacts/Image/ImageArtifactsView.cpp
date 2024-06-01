#include "ImageArtifactsView.h"

#include "ImageArtifactsModel.h"
#include "ImageArtifactsDelegate.h"
#include "../../Utils/ModelUtils.h"
#include "../../../Artifacts/Image/ImageArtifact.h"
#include "../../../Artifacts/Pipeline.h"

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

    QModelIndex const parent = index.parent();
    QModelIndex ancestor = parent.parent();

    QStyleOptionViewItem opt;
    initViewItemOption(&opt);

    QPoint const oldBrushOrigin = painter->brushOrigin();
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
        QModelIndex const successorIndex = index.model()->sibling(index.row() + 1, 0, index);
        bool moreSiblings = successorIndex.isValid();

        opt.state = QStyle::State_Item | extraFlags
                    | (moreSiblings ? QStyle::State_Sibling : QStyle::State_None)
                    | (children ? QStyle::State_Children : QStyle::State_None)
                    | (expanded ? QStyle::State_Open : QStyle::State_None);

        style()->drawPrimitive(QStyle::PE_IndicatorBranch, &opt, painter, this);

        bool const isRootLevel = !parent.isValid();
        bool parentIsSequentialComposition = false;
        if (parent.isValid()) {
            auto data = parent.data(Qt::UserRole).value<ImageArtifactData>();
            parentIsSequentialComposition = std::holds_alternative<CompositeImageArtifactData>(data.Data)
                    && std::get<CompositeImageArtifactData>(data.Data).Data.CompositionType
                            == CompositeImageArtifact::CompositionType::SEQUENTIAL;
        }

        if (isRootLevel || parentIsSequentialComposition) {
            QFont const oldFont(painter->font());

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


ImageArtifactsReadOnlyView::ImageArtifactsReadOnlyView(Pipeline const& pipeline, QWidget* parent) :
        ImageArtifactsView(const_cast<Pipeline*>(&pipeline), parent),
        ArtifactsModel(new ImageArtifactsReadOnlyModel(pipeline.GetImageArtifactConcatenation())){

    setModel(ArtifactsModel);
}

auto ImageArtifactsReadOnlyView::model() const noexcept -> ImageArtifactsReadOnlyModel* {
    return ArtifactsModel;
}

auto ImageArtifactsReadOnlyView::Select(ImageArtifact const& imageArtifact) -> void {
    auto imageArtifactPointer = const_cast<ImageArtifact*>(&imageArtifact);
    auto match = Search(*ArtifactsModel, ImageArtifactsModel::Roles::POINTER,
                        QVariant::fromValue(imageArtifactPointer), rootIndex());

    if (match == QModelIndex{})
        throw std::runtime_error("Image artifact not found");

    QModelIndexList ancestorIndices;
    for (QModelIndex current = match.parent(); current.isValid(); current = current.parent())
        ancestorIndices.prepend(current);

    collapseAll();
    for (auto const& ancestorIndex : ancestorIndices)
        expand(ancestorIndex);

    selectionModel()->clearSelection();
    selectionModel()->select(match, QItemSelectionModel::SelectionFlag::Select);
}

void ImageArtifactsReadOnlyView::selectionChanged(QItemSelection const& selected, QItemSelection const& deselected) {
    QTreeView::selectionChanged(selected, deselected);

    QModelIndexList const selectedIndices = selected.indexes();

    if (selectedIndices.empty()) {
        Q_EMIT ImageArtifactChanged(nullptr);
        return;
    }

    if (selectedIndices.size() > 1)
        throw std::runtime_error("Invalid selection size");

    QModelIndex const selectedIndex = selectedIndices.at(0);

    auto* imageArtifact = selectedIndex.data(ImageArtifactsModel::POINTER).value<ImageArtifact*>();

    Q_EMIT ImageArtifactChanged(imageArtifact);
}
