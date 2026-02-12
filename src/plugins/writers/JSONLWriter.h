#pragma once
#include "core/PluginInterface.h"

namespace DatasetCreator {
class JSONLWriter : public IDataWriter {
public:
    QString name() const override { return "JSONLWriter"; }
    QString version() const override { return "1.0.0"; }
    QString fileExtension() const override { return ".jsonl"; }
    QString formatName() const override { return "JSONL"; }
    QString description() const override { return "JSON Lines format (newline-delimited JSON)"; }
    bool write(const QString& outputPath, const Dataset& dataset) override;
};
}
