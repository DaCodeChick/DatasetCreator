#pragma once
#include <QWidget>
#include <QLineEdit>
#include <QTextEdit>
#include <QPushButton>
#include <QLabel>
#include "core/DatasetSample.h"

namespace DatasetCreator {

class MetadataEditor : public QWidget {
    Q_OBJECT
public:
    explicit MetadataEditor(QWidget* parent = nullptr);
    
public slots:
    void editSample(const DatasetSample& sample);
    void clear();
    
signals:
    void tagsChanged(const QStringList& tags);
    void labelsChanged(const QStringList& labels);
    void attributeChanged(const QString& key, const QVariant& value);
    
private slots:
    void onAddTag();
    void onAddLabel();
    void onApplyChanges();
    
private:
    void setupUI();
    
    QLabel* sampleIdLabel_;
    QLineEdit* tagsEdit_;
    QLineEdit* labelsEdit_;
    QTextEdit* attributesEdit_;
    QPushButton* addTagButton_;
    QPushButton* addLabelButton_;
    QPushButton* applyButton_;
    
    const DatasetSample* currentSample_;
};

}

