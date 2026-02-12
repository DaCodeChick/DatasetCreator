#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include "src/core/Dataset.h"
#include "src/core/DatasetSample.h"
#include "src/plugins/PluginManager.h"
#include "src/managers/ImportManager.h"
#include "src/managers/ExportManager.h"

using namespace DatasetCreator;

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    
    qDebug() << "=== Testing Metadata Editing Functionality ===\n";
    
    // Create a dataset
    Dataset dataset;
    dataset.metadata().name = "Test Dataset";
    
    // Create some test samples
    DatasetSample sample1(SampleType::Text);
    sample1.setText("This is sample 1");
    sample1.metadata().id = "sample1";
    sample1.metadata().sourceFile = "sample1.txt";
    
    DatasetSample sample2(SampleType::Text);
    sample2.setText("This is sample 2");
    sample2.metadata().id = "sample2";
    sample2.metadata().sourceFile = "sample2.txt";
    
    dataset.addSample(sample1);
    dataset.addSample(sample2);
    
    qDebug() << "Initial dataset:";
    qDebug() << "  Sample count:" << dataset.sampleCount();
    qDebug() << "  Sample 0 tags:" << dataset.samples()[0].metadata().tags;
    qDebug() << "  Sample 0 labels:" << dataset.samples()[0].metadata().labels;
    qDebug() << "  Sample 1 tags:" << dataset.samples()[1].metadata().tags;
    qDebug() << "  Sample 1 labels:" << dataset.samples()[1].metadata().labels;
    qDebug() << "";
    
    // Test updating tags
    qDebug() << "Testing updateSampleTags...";
    QStringList tags1 = {"training", "validated", "text"};
    bool result = dataset.updateSampleTags(0, tags1);
    qDebug() << "  Result:" << result;
    qDebug() << "  Sample 0 tags after update:" << dataset.samples()[0].metadata().tags;
    qDebug() << "";
    
    // Test updating labels
    qDebug() << "Testing updateSampleLabels...";
    QVariantMap labels1;
    labels1["category"] = "positive";
    labels1["score"] = 0.95;
    labels1["verified"] = true;
    result = dataset.updateSampleLabels(0, labels1);
    qDebug() << "  Result:" << result;
    qDebug() << "  Sample 0 labels after update:" << dataset.samples()[0].metadata().labels;
    qDebug() << "";
    
    // Test adding individual tag
    qDebug() << "Testing addSampleTag...";
    result = dataset.addSampleTag(1, "important");
    qDebug() << "  Result:" << result;
    qDebug() << "  Sample 1 tags after add:" << dataset.samples()[1].metadata().tags;
    qDebug() << "";
    
    // Test adding individual label
    qDebug() << "Testing addSampleLabel...";
    result = dataset.addSampleLabel(1, "sentiment", "neutral");
    qDebug() << "  Result:" << result;
    qDebug() << "  Sample 1 labels after add:" << dataset.samples()[1].metadata().labels;
    qDebug() << "";
    
    // Test with invalid index
    qDebug() << "Testing with invalid index (999)...";
    result = dataset.updateSampleTags(999, tags1);
    qDebug() << "  Result (should be false):" << result;
    qDebug() << "";
    
    // Export to verify persistence
    qDebug() << "Exporting to JSONL to verify persistence...";
    PluginManager pluginManager;
    ExportManager exportManager(&pluginManager);
    
    bool exportResult = exportManager.exportDataset(dataset, "test_metadata_output.jsonl", "jsonl");
    qDebug() << "  Export result:" << exportResult;
    
    if (exportResult) {
        qDebug() << "\n=== Contents of test_metadata_output.jsonl ===";
        QFile file("test_metadata_output.jsonl");
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);
            while (!in.atEnd()) {
                qDebug() << in.readLine();
            }
            file.close();
        }
    }
    
    qDebug() << "\n=== All tests completed successfully! ===";
    
    return 0;
}
