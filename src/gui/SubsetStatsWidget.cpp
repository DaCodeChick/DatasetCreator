#include "SubsetStatsWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPainter>
#include <QPainterPath>
#include <QFontMetrics>

namespace DatasetCreator {

const QList<QColor> SubsetStatsWidget::PRESET_COLORS = {
    QColor(52, 152, 219),   // Blue
    QColor(46, 204, 113),   // Green
    QColor(155, 89, 182),   // Purple
    QColor(241, 196, 15),   // Yellow
    QColor(231, 76, 60),    // Red
    QColor(26, 188, 156),   // Teal
    QColor(230, 126, 34),   // Orange
    QColor(149, 165, 166)   // Gray
};

SubsetStatsWidget::SubsetStatsWidget(QWidget* parent)
    : QWidget(parent), dataset_(nullptr), totalSamples_(0)
{
    setMinimumHeight(150);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
}

void SubsetStatsWidget::setDataset(Dataset* dataset) {
    dataset_ = dataset;
    refresh();
}

void SubsetStatsWidget::refresh() {
    updateStats();
    update(); // Trigger repaint
}

void SubsetStatsWidget::updateStats() {
    stats_.clear();
    
    if (!dataset_) {
        totalSamples_ = 0;
        return;
    }
    
    totalSamples_ = dataset_->totalSampleCount();
    
    if (totalSamples_ == 0) {
        return;
    }
    
    // Add root samples if any
    int rootCount = dataset_->sampleCount();
    if (rootCount > 0) {
        SubsetStats rootStats;
        rootStats.name = tr("Root");
        rootStats.count = rootCount;
        rootStats.percentage = (rootCount * 100.0) / totalSamples_;
        rootStats.color = QColor(189, 195, 199); // Light gray
        stats_.append(rootStats);
    }
    
    // Add subset stats
    const auto& subsets = dataset_->subsets();
    for (int i = 0; i < subsets.size(); ++i) {
        const auto& subset = subsets[i];
        SubsetStats subsetStats;
        subsetStats.name = subset.name();
        subsetStats.count = subset.sampleCount();
        subsetStats.percentage = (subsetStats.count * 100.0) / totalSamples_;
        subsetStats.color = PRESET_COLORS[i % PRESET_COLORS.size()];
        stats_.append(subsetStats);
    }
}

QSize SubsetStatsWidget::sizeHint() const {
    return QSize(400, 150);
}

void SubsetStatsWidget::paintEvent(QPaintEvent* event) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    if (totalSamples_ == 0 || stats_.isEmpty()) {
        // Draw empty state
        painter.setPen(QColor(149, 165, 166));
        QFont font = painter.font();
        font.setPointSize(10);
        painter.setFont(font);
        painter.drawText(rect(), Qt::AlignCenter, tr("No samples in dataset"));
        return;
    }
    
    // Draw title
    painter.setPen(QColor(44, 62, 80));
    QFont titleFont = painter.font();
    titleFont.setPointSize(11);
    titleFont.setBold(true);
    painter.setFont(titleFont);
    painter.drawText(10, 20, tr("Subset Distribution (%1 samples)").arg(totalSamples_));
    
    // Calculate layout
    int barHeight = 40;
    int barY = 40;
    int barX = 10;
    int barWidth = width() - 20;
    
    // Draw horizontal stacked bar
    int currentX = barX;
    for (const auto& stat : stats_) {
        int segmentWidth = (stat.count * barWidth) / totalSamples_;
        if (segmentWidth > 0) {
            // Draw segment
            painter.fillRect(currentX, barY, segmentWidth, barHeight, stat.color);
            
            // Draw border
            painter.setPen(QColor(255, 255, 255));
            painter.drawRect(currentX, barY, segmentWidth, barHeight);
            
            currentX += segmentWidth;
        }
    }
    
    // Draw legend below
    int legendY = barY + barHeight + 20;
    int legendX = barX;
    QFont legendFont = painter.font();
    legendFont.setPointSize(9);
    legendFont.setBold(false);
    painter.setFont(legendFont);
    QFontMetrics fm(legendFont);
    
    for (const auto& stat : stats_) {
        // Draw color box
        painter.fillRect(legendX, legendY, 12, 12, stat.color);
        painter.setPen(QColor(149, 165, 166));
        painter.drawRect(legendX, legendY, 12, 12);
        
        // Draw label
        painter.setPen(QColor(44, 62, 80));
        QString label = QString("%1: %2 (%3%)")
            .arg(stat.name)
            .arg(stat.count)
            .arg(stat.percentage, 0, 'f', 1);
        painter.drawText(legendX + 18, legendY + 10, label);
        
        // Move to next column
        int labelWidth = fm.horizontalAdvance(label) + 30;
        legendX += labelWidth;
        
        // Wrap to next row if needed
        if (legendX + 200 > width()) {
            legendX = barX;
            legendY += 20;
        }
    }
}

}
