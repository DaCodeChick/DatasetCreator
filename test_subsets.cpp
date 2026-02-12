#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include "src/core/Dataset.h"
#include "src/core/DatasetSample.h"
#include "src/plugins/PluginManager.h"
#include "src/managers/ExportManager.h"

using namespace DatasetCreator;

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    
    qDebug() << "=== Testing Subset Management Functionality ===\n";
    
    // Create a dataset with samples
    Dataset dataset;
    dataset.metadata().name = "ML Dataset with Subsets";
    
    // Create several samples
    for (int i = 0; i < 10; ++i) {
        DatasetSample sample(SampleType::Text);
        sample.setText(QString("Sample %1 content").arg(i));
        sample.metadata().id = QString("sample_%1").arg(i);
        sample.metadata().sourceFile = QString("sample_%1.txt").arg(i);
        dataset.addSample(sample);
    }
    
    qDebug() << "Initial dataset:";
    qDebug() << "  Total samples:" << dataset.totalSampleCount();
    qDebug() << "  Samples in root:" << dataset.sampleCount();
    qDebug() << "  Subset count:" << dataset.subsetCount();
    qDebug() << "";
    
    // Move samples to subsets (typical ML split: 70% train, 20% validation, 10% test)
    qDebug() << "Moving samples to subsets...";
    
    // Training set: samples 0-6 (7 samples = 70%)
    for (int i = 0; i <= 6; ++i) {
        dataset.moveSampleToSubset(0, "training");  // Always move index 0 since array shrinks
    }
    
    // Validation set: samples 0-1 (2 samples = 20% of remaining 10)
    for (int i = 0; i <= 1; ++i) {
        dataset.moveSampleToSubset(0, "validation");
    }
    
    // Test set: sample 0 (1 sample = 10%)
    dataset.moveSampleToSubset(0, "test");
    
    qDebug() << "\nAfter subset organization:";
    qDebug() << "  Total samples:" << dataset.totalSampleCount();
    qDebug() << "  Samples in root:" << dataset.sampleCount();
    qDebug() << "  Subset count:" << dataset.subsetCount();
    qDebug() << "  Subset names:" << dataset.subsetNames();
    qDebug() << "";
    
    // Print subset details
    for (const QString& subsetName : dataset.subsetNames()) {
        const DatasetSubset* subset = dataset.getSubset(subsetName);
        if (subset) {
            qDebug() << "  Subset" << subsetName << ":";
            qDebug() << "    Sample count:" << subset->sampleCount();
            qDebug() << "    Total size:" << subset->totalSize() << "bytes";
            
            // Print first few sample IDs
            QStringList sampleIds;
            for (int i = 0; i < qMin(3, subset->sampleCount()); ++i) {
                sampleIds.append(subset->samples()[i].metadata().id);
            }
            if (subset->sampleCount() > 3) {
                sampleIds.append("...");
            }
            qDebug() << "    Samples:" << sampleIds.join(", ");
        }
    }
    qDebug() << "";
    
    // Export to JSONL
    qDebug() << "Exporting to JSONL...";
    PluginManager pluginManager;
    ExportManager exportManager(&pluginManager);
    
    bool exportResult = exportManager.exportDataset(dataset, "test_subsets_output.jsonl", "jsonl");
    qDebug() << "  Export result:" << exportResult;
    
    if (exportResult) {
        qDebug() << "\n=== Contents of test_subsets_output.jsonl ===";
        QFile file("test_subsets_output.jsonl");
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);
            int lineNum = 0;
            while (!in.atEnd() && lineNum < 15) {  // Show first 15 lines
                qDebug() << in.readLine();
                lineNum++;
            }
            if (!in.atEnd()) {
                qDebug() << "... (more lines omitted)";
            }
            file.close();
        }
    }
    
    qDebug() << "\n=== Subset Management Tests Completed! ===";
    
    return 0;
}
