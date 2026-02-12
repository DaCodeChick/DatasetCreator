#pragma once
#include <QWidget>
#include <QLabel>
#include <QPainter>
#include "core/Dataset.h"

namespace DatasetCreator {

/**
 * @brief Widget that displays subset distribution statistics
 * Shows a horizontal bar chart with sample counts per subset
 */
class SubsetStatsWidget : public QWidget {
    Q_OBJECT
public:
    explicit SubsetStatsWidget(QWidget* parent = nullptr);
    
    void setDataset(Dataset* dataset);
    void refresh();
    
protected:
    void paintEvent(QPaintEvent* event) override;
    QSize sizeHint() const override;
    
private:
    void updateStats();
    
    struct SubsetStats {
        QString name;
        int count;
        double percentage;
        QColor color;
    };
    
    Dataset* dataset_;
    QList<SubsetStats> stats_;
    int totalSamples_;
    
    // Predefined colors for subsets
    static const QList<QColor> PRESET_COLORS;
};

}
