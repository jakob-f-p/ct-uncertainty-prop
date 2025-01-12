#include "StructureArtifactsWidget.h"

#include "StructureArtifactsWidgetDelegate.h"
#include "../Image/ImageArtifactsView.h"
#include "../PipelinesWidget.h"
#include "../../Modeling/CtStructureTreeModel.h"
#include "../../Utils/WidgetUtils.h"
#include "../../../App.h"

#include <QLabel>
#include <QStackedLayout>

StructureArtifactsWidget::StructureArtifactsWidget(QWidget* parent) :
        QWidget(parent),
        Views(new QStackedLayout()),
        Dialog(nullptr) {

    auto* vLayout = new QVBoxLayout(this);
    vLayout->setContentsMargins({});

    auto* titleLabel = new QLabel("Structure Artifacts");
    titleLabel->setStyleSheet(GetHeader2StyleSheet());
    vLayout->addWidget(titleLabel);

    vLayout->addLayout(Views);
    auto* placeHolderView = new QTreeView();
    Views->addWidget(placeHolderView);
}

void StructureArtifactsWidget::SetCurrentView(int pipelineIdx) const {
    if (pipelineIdx + 1 >= Views->count()) {
        qWarning("Cannot set view");
        return;
    }

    Views->setCurrentIndex(pipelineIdx + 1);
}

void StructureArtifactsWidget::AddView(Pipeline& pipeline) const {
    auto* newView = new QTreeView();
    auto* newDelegate = new StructureArtifactsWidgetDelegate(pipeline, newView);
    newView->setItemDelegate(newDelegate);
    auto* newModel = new CtStructureArtifactsModel(App::GetInstance().GetCtDataTree(), pipeline, newView);
    newView->setModel(newModel);
    newView->setHeaderHidden(true);

    Views->addWidget(newView);
}

void StructureArtifactsWidget::RemoveCurrentView() const {
    if (Views->count() <= 1) {
        qWarning("Cannot remove any more views");
        return;
    }

    Views->removeWidget(Views->currentWidget());
}

void StructureArtifactsWidget::ResetModel() const {
    for (int i = 1; i < Views->count(); i++) {
        auto const* view = dynamic_cast<QTreeView*>(Views->widget(i));
        auto* model = dynamic_cast<CtStructureTreeModel*>(view->model());
        model->beginResetModel();
        model->endResetModel();
    }
}
