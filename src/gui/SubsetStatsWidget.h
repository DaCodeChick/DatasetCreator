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
    void mouseMoveEvent(QMouseEvent* event) override;
    void leaveEvent(QEvent* event) override;
    QSize sizeHint() const override;
    
private:
    void updateStats();
    int getSubsetAtPosition(const QPoint& pos) const;
    
    struct SubsetStats {
        QString name;
        int count;
        double percentage;
        QColor color;
        int xStart;  // For hover detection
        int xEnd;
    };
    
    Dataset* dataset_;
    QList<SubsetStats> stats_;
    int totalSamples_;
    int hoveredSubset_;  // Index of currently hovered subset (-1 if none)
    
    // Predefined colors for subsets
    static const QList<QColor> PRESET_COLORS;
};

}
