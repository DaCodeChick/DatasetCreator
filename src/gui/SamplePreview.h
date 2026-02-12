#pragma once
#include <QWidget>
#include <QStackedWidget>
#include <QTextEdit>
#include <QLabel>
#include "core/DatasetSample.h"

namespace DatasetCreator {

class SamplePreview : public QWidget {
    Q_OBJECT
public:
    explicit SamplePreview(QWidget* parent = nullptr);
    
public slots:
    void showSample(const DatasetSample& sample);
    void clear();
    
private:
    void setupUI();
    void showText(const DatasetSample& sample);
    void showImage(const DatasetSample& sample);
    void showAudio(const DatasetSample& sample);
    void showBinary(const DatasetSample& sample);
    
    QStackedWidget* stackedWidget_;
    QTextEdit* textPreview_;
    QLabel* imagePreview_;
    QTextEdit* audioPreview_;
    QTextEdit* binaryPreview_;
};

}
