#include "MetadataEditor.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>

namespace DatasetCreator {

MetadataEditor::MetadataEditor(QWidget* parent) 
    : QWidget(parent), currentSample_(nullptr)
{
    setupUI();
}

void MetadataEditor::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(5, 5, 5, 5);
    
    // Sample info
    QGroupBox* infoGroup = new QGroupBox("Sample Information", this);
    QFormLayout* infoLayout = new QFormLayout(infoGroup);
    
    sampleIdLabel_ = new QLabel("No sample selected", this);
    infoLayout->addRow("ID:", sampleIdLabel_);
    
    mainLayout->addWidget(infoGroup);
    
    // Tags and Labels section (side by side)
    QHBoxLayout* metadataLayout = new QHBoxLayout();
    
    // Tags section
    QGroupBox* tagsGroup = new QGroupBox("Tags", this);
    QVBoxLayout* tagsLayout = new QVBoxLayout(tagsGroup);
    
    tagsEdit_ = new QLineEdit(this);
    tagsEdit_->setPlaceholderText("Comma-separated tags (e.g., training, text, sample)");
    tagsLayout->addWidget(tagsEdit_);
    
    addTagButton_ = new QPushButton("Add Tag", this);
    connect(addTagButton_, &QPushButton::clicked, this, &MetadataEditor::onAddTag);
    tagsLayout->addWidget(addTagButton_);
    
    metadataLayout->addWidget(tagsGroup);
    
    // Labels section
    QGroupBox* labelsGroup = new QGroupBox("Labels", this);
    QVBoxLayout* labelsLayout = new QVBoxLayout(labelsGroup);
    
    labelsEdit_ = new QLineEdit(this);
    labelsEdit_->setPlaceholderText("Comma-separated labels (e.g., positive, category1)");
    labelsLayout->addWidget(labelsEdit_);
    
    addLabelButton_ = new QPushButton("Add Label", this);
    connect(addLabelButton_, &QPushButton::clicked, this, &MetadataEditor::onAddLabel);
    labelsLayout->addWidget(addLabelButton_);
    
    metadataLayout->addWidget(labelsGroup);
    
    mainLayout->addLayout(metadataLayout);
    
    // Apply button
    applyButton_ = new QPushButton("Apply Changes", this);
    applyButton_->setEnabled(false);
    connect(applyButton_, &QPushButton::clicked, this, &MetadataEditor::onApplyChanges);
    mainLayout->addWidget(applyButton_);
    
    mainLayout->addStretch();
}

void MetadataEditor::editSample(const DatasetSample& sample) {
    currentSample_ = &sample;
    
    sampleIdLabel_->setText(sample.metadata().id);
    tagsEdit_->setText(sample.metadata().tags.join(", "));
    
    // Convert labels QVariantMap to string
    QString labelsStr;
    QVariantMap labels = sample.metadata().labels;
    for (auto it = labels.begin(); it != labels.end(); ++it) {
        if (!labelsStr.isEmpty()) labelsStr += ", ";
        labelsStr += it.key() + ":" + it.value().toString();
    }
    labelsEdit_->setText(labelsStr);
    
    applyButton_->setEnabled(true);
}

void MetadataEditor::clear() {
    currentSample_ = nullptr;
    sampleIdLabel_->setText("No sample selected");
    tagsEdit_->clear();
    labelsEdit_->clear();
    applyButton_->setEnabled(false);
}

void MetadataEditor::onAddTag() {
    QString newTag = tagsEdit_->text().trimmed();
    if (newTag.isEmpty()) return;
    
    QStringList tags = newTag.split(",", Qt::SkipEmptyParts);
    for (QString& tag : tags) {
        tag = tag.trimmed();
    }
    
    emit tagsChanged(tags);
}

void MetadataEditor::onAddLabel() {
    QString newLabel = labelsEdit_->text().trimmed();
    if (newLabel.isEmpty()) return;
    
    QStringList labels = newLabel.split(",", Qt::SkipEmptyParts);
    for (QString& label : labels) {
        label = label.trimmed();
    }
    
    emit labelsChanged(labels);
}

void MetadataEditor::onApplyChanges() {
    if (!currentSample_) return;
    
    onAddTag();
    onAddLabel();
}

}

