#include "SamplePreview.h"
#include <QVBoxLayout>
#include <QScrollArea>
#include <QPixmap>

namespace DatasetCreator {

SamplePreview::SamplePreview(QWidget* parent) 
    : QWidget(parent)
{
    setupUI();
}

void SamplePreview::setupUI() {
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    
    stackedWidget_ = new QStackedWidget(this);
    
    // Text preview
    textPreview_ = new QTextEdit(this);
    textPreview_->setReadOnly(true);
    stackedWidget_->addWidget(textPreview_);
    
    // Image preview
    QScrollArea* imageScroll = new QScrollArea(this);
    imagePreview_ = new QLabel(this);
    imagePreview_->setAlignment(Qt::AlignCenter);
    imagePreview_->setScaledContents(false);
    imageScroll->setWidget(imagePreview_);
    imageScroll->setWidgetResizable(true);
    stackedWidget_->addWidget(imageScroll);
    
    // Audio preview (metadata only)
    audioPreview_ = new QTextEdit(this);
    audioPreview_->setReadOnly(true);
    stackedWidget_->addWidget(audioPreview_);
    
    // Binary preview (hex dump)
    binaryPreview_ = new QTextEdit(this);
    binaryPreview_->setReadOnly(true);
    binaryPreview_->setFontFamily("Monospace");
    stackedWidget_->addWidget(binaryPreview_);
    
    layout->addWidget(stackedWidget_);
}

void SamplePreview::showSample(const DatasetSample& sample) {
    switch (sample.type()) {
        case SampleType::Text:
            showText(sample);
            break;
        case SampleType::Image:
            showImage(sample);
            break;
        case SampleType::Audio:
            showAudio(sample);
            break;
        case SampleType::Binary:
        case SampleType::Multimodal:
            showBinary(sample);
            break;
    }
}

void SamplePreview::clear() {
    textPreview_->clear();
    imagePreview_->clear();
    audioPreview_->clear();
    binaryPreview_->clear();
}

void SamplePreview::showText(const DatasetSample& sample) {
    QString text = sample.asText();
    
    // Limit preview to first 10000 characters
    if (text.length() > 10000) {
        text = text.left(10000) + "\n\n... (truncated)";
    }
    
    textPreview_->setPlainText(text);
    stackedWidget_->setCurrentWidget(textPreview_);
}

void SamplePreview::showImage(const DatasetSample& sample) {
    QImage image = sample.asImage();
    
    if (!image.isNull()) {
        QPixmap pixmap = QPixmap::fromImage(image);
        
        // Scale if too large, maintaining aspect ratio
        if (pixmap.width() > 800 || pixmap.height() > 600) {
            pixmap = pixmap.scaled(800, 600, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        }
        
        imagePreview_->setPixmap(pixmap);
    } else {
        imagePreview_->setText("Failed to load image");
    }
    
    stackedWidget_->setCurrentWidget(imagePreview_->parentWidget());
}

void SamplePreview::showAudio(const DatasetSample& sample) {
    QString info = "Audio Sample\n\n";
    info += "ID: " + sample.metadata().id + "\n";
    info += "Source: " + sample.metadata().sourceFile + "\n";
    info += "Size: " + QString::number(sample.dataSize()) + " bytes\n";
    
    // Show metadata from attributes
    const auto& meta = sample.metadata().attributes;
    if (meta.contains("duration")) {
        info += "Duration: " + meta["duration"].toString() + "\n";
    }
    if (meta.contains("sample_rate")) {
        info += "Sample Rate: " + meta["sample_rate"].toString() + " Hz\n";
    }
    if (meta.contains("channels")) {
        info += "Channels: " + meta["channels"].toString() + "\n";
    }
    if (meta.contains("codec")) {
        info += "Codec: " + meta["codec"].toString() + "\n";
    }
    
    audioPreview_->setPlainText(info);
    stackedWidget_->setCurrentWidget(audioPreview_);
}

void SamplePreview::showBinary(const DatasetSample& sample) {
    QByteArray data = sample.asBinary();
    
    QString hexDump = "Binary Data Preview\n\n";
    hexDump += "Size: " + QString::number(data.size()) + " bytes\n\n";
    
    // Show first 512 bytes as hex dump
    int bytesToShow = qMin(512, data.size());
    for (int i = 0; i < bytesToShow; i += 16) {
        // Address
        hexDump += QString("%1: ").arg(i, 4, 16, QChar('0'));
        
        // Hex bytes
        QString ascii;
        for (int j = 0; j < 16 && (i + j) < bytesToShow; ++j) {
            unsigned char byte = static_cast<unsigned char>(data[i + j]);
            hexDump += QString("%1 ").arg(byte, 2, 16, QChar('0'));
            ascii += (byte >= 32 && byte < 127) ? QChar(byte) : QChar('.');
        }
        
        // ASCII representation
        hexDump += "  " + ascii + "\n";
    }
    
    if (data.size() > 512) {
        hexDump += "\n... (truncated)";
    }
    
    binaryPreview_->setPlainText(hexDump);
    stackedWidget_->setCurrentWidget(binaryPreview_);
}

}
