#pragma once
#include <QDialog>

namespace DatasetCreator {
class FileImportDialog : public QDialog {
    Q_OBJECT
public:
    explicit FileImportDialog(QWidget* parent = nullptr);
};
}
