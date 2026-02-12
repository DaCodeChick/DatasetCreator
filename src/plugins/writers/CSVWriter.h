#pragma once
#include "core/PluginInterface.h"

namespace DatasetCreator {
class CSVWriter : public IDataWriter {
public:
    QString name() const override { return "CSVWriter"; }
    QString version() const override { return "1.0.0"; }
    QString fileExtension() const override { return ".csv"; }
    QString formatName() const override { return "CSV"; }
    QString description() const override { return "Comma-Separated Values format"; }
    bool write(const QString& outputPath, const Dataset& dataset) override;
};
}
