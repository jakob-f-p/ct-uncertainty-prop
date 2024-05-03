#include "StructureArtifactsWidget.h"

#include "ImageArtifactsView.h"
#include "PipelinesWidget.h"
#include "StructureArtifactsWidgetDelegate.h"
#include "../../App.h"
#include "../../Modeling/UI/CtStructureTreeModel.h"

#include <QLabel>
#include <QStackedLayout>

StructureArtifactsWidget::StructureArtifactsWidget(QWidget* parent) :
        QWidget(parent),
        Views(new QStackedLayout()),
        Dialog(nullptr) {

    auto* vLayout = new QVBoxLayout(this);

    auto* titleLabel = new QLabel("Structure Artifacts");
    titleLabel->setStyleSheet(PipelinesWidget::GetHeaderStyleSheet());
    vLayout->addWidget(titleLabel);

    vLayout->addLayout(Views);
    auto* placeHolderView = new QTreeView();
    Views->addWidget(placeHolderView);
}

void StructureArtifactsWidget::SetCurrentView(int pipelineIdx) {
    if (pipelineIdx + 1 >= Views->count()) {
        qWarning("Cannot set view");
        return;
    }

    Views->setCurrentIndex(pipelineIdx + 1);
}

void StructureArtifactsWidget::AddView(Pipeline& pipeline) {
    auto* newView = new QTreeView();
    auto* newDelegate = new StructureArtifactsWidgetDelegate(pipeline);
    newView->setItemDelegate(newDelegate);
    auto* newModel = new CtStructureTreeModel(App::GetInstance()->GetCtDataTree());
    newView->setModel(newModel);
    newView->setHeaderHidden(true);

    Views->addWidget(newView);
}

void StructureArtifactsWidget::RemoveCurrentView() {
    if (Views->count() <= 1) {
        qWarning("Cannot remove any more views");
        return;
    }

    Views->removeWidget(Views->currentWidget());
}

void StructureArtifactsWidget::ResetModel() {
    for (int i = 1; i < Views->count(); i++) {
        auto* view = dynamic_cast<QTreeView*>(Views->widget(i));
        auto* model = dynamic_cast<CtStructureTreeModel*>(view->model());
        model->beginResetModel();
        model->endResetModel();
    }
}
