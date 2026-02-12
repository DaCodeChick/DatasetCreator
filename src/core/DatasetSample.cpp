#include "DatasetSample.h"
#include <QBuffer>
#include <QImageWriter>

namespace DatasetCreator {

// AudioData implementation
QVariantMap AudioData::toVariantMap() const {
    QVariantMap map;
    map["samples"] = samples.toBase64();
    map["sample_rate"] = format.sampleRate();
    map["channel_count"] = format.channelCount();
    map["sample_format"] = static_cast<int>(format.sampleFormat());
    map["duration_ms"] = durationMs;
    return map;
}

AudioData AudioData::fromVariantMap(const QVariantMap& map) {
    AudioData data;
    data.samples = QByteArray::fromBase64(map.value("samples").toByteArray());
    
    data.format.setSampleRate(map.value("sample_rate").toInt());
    data.format.setChannelCount(map.value("channel_count").toInt());
    data.format.setSampleFormat(static_cast<QAudioFormat::SampleFormat>(
        map.value("sample_format").toInt()));
    
    data.durationMs = map.value("duration_ms").toLongLong();
    return data;
}

// MultimodalData implementation
QVariantMap MultimodalData::toVariantMap() const {
    QVariantMap map;
    if (!text.isEmpty()) map["text"] = text;
    
    if (!image.isNull()) {
        QByteArray imageData;
        QBuffer buffer(&imageData);
        buffer.open(QIODevice::WriteOnly);
        image.save(&buffer, "PNG");
        map["image"] = imageData.toBase64();
    }
    
    if (!audio.samples.isEmpty()) {
        map["audio"] = audio.toVariantMap();
    }
    
    if (!additionalData.isEmpty()) {
        map["additional"] = additionalData;
    }
    
    return map;
}

MultimodalData MultimodalData::fromVariantMap(const QVariantMap& map) {
    MultimodalData data;
    data.text = map.value("text").toString();
    
    if (map.contains("image")) {
        QByteArray imageData = QByteArray::fromBase64(map.value("image").toByteArray());
        data.image.loadFromData(imageData);
    }
    
    if (map.contains("audio")) {
        data.audio = AudioData::fromVariantMap(map.value("audio").toMap());
    }
    
    data.additionalData = map.value("additional").toMap();
    return data;
}

// DatasetSample implementation
DatasetSample::DatasetSample()
    : type_(SampleType::Text)
{
    metadata_.timestamp = QDateTime::currentDateTime();
}

DatasetSample::DatasetSample(SampleType type)
    : type_(type)
{
    metadata_.timestamp = QDateTime::currentDateTime();
}

QString DatasetSample::asText() const {
    return data_.toString();
}

QImage DatasetSample::asImage() const {
    return data_.value<QImage>();
}

AudioData DatasetSample::asAudio() const {
    return data_.value<AudioData>();
}

QByteArray DatasetSample::asBinary() const {
    return data_.toByteArray();
}

MultimodalData DatasetSample::asMultimodal() const {
    return data_.value<MultimodalData>();
}

void DatasetSample::setText(const QString& text) {
    type_ = SampleType::Text;
    data_ = text;
}

void DatasetSample::setImage(const QImage& image) {
    type_ = SampleType::Image;
    data_ = QVariant::fromValue(image);
}

void DatasetSample::setAudio(const AudioData& audio) {
    type_ = SampleType::Audio;
    data_ = QVariant::fromValue(audio);
}

void DatasetSample::setBinary(const QByteArray& data) {
    type_ = SampleType::Binary;
    data_ = data;
}

void DatasetSample::setMultimodal(const MultimodalData& data) {
    type_ = SampleType::Multimodal;
    data_ = QVariant::fromValue(data);
}

QVariantMap DatasetSample::toVariantMap() const {
    QVariantMap map;
    
    // Add type
    map["type"] = static_cast<int>(type_);
    
    // Add data based on type
    switch (type_) {
        case SampleType::Text:
            map["data"] = asText();
            break;
        case SampleType::Image: {
            QImage img = asImage();
            if (!img.isNull()) {
                QByteArray imageData;
                QBuffer buffer(&imageData);
                buffer.open(QIODevice::WriteOnly);
                img.save(&buffer, "PNG");
                map["data"] = imageData.toBase64();
            }
            break;
        }
        case SampleType::Audio:
            map["data"] = asAudio().toVariantMap();
            break;
        case SampleType::Binary:
            map["data"] = asBinary().toBase64();
            break;
        case SampleType::Multimodal:
            map["data"] = asMultimodal().toVariantMap();
            break;
    }
    
    // Add metadata
    map["metadata"] = metadata_.toVariantMap();
    
    return map;
}

DatasetSample DatasetSample::fromVariantMap(const QVariantMap& map) {
    auto type = static_cast<SampleType>(map.value("type").toInt());
    DatasetSample sample(type);
    
    QVariant dataVariant = map.value("data");
    
    switch (type) {
        case SampleType::Text:
            sample.setText(dataVariant.toString());
            break;
        case SampleType::Image: {
            QByteArray imageData = QByteArray::fromBase64(dataVariant.toByteArray());
            QImage img;
            img.loadFromData(imageData);
            sample.setImage(img);
            break;
        }
        case SampleType::Audio:
            sample.setAudio(AudioData::fromVariantMap(dataVariant.toMap()));
            break;
        case SampleType::Binary:
            sample.setBinary(QByteArray::fromBase64(dataVariant.toByteArray()));
            break;
        case SampleType::Multimodal:
            sample.setMultimodal(MultimodalData::fromVariantMap(dataVariant.toMap()));
            break;
    }
    
    sample.setMetadata(SampleMetadata::fromVariantMap(map.value("metadata").toMap()));
    
    return sample;
}

bool DatasetSample::isEmpty() const {
    switch (type_) {
        case SampleType::Text:
            return asText().isEmpty();
        case SampleType::Image:
            return asImage().isNull();
        case SampleType::Audio:
            return asAudio().samples.isEmpty();
        case SampleType::Binary:
            return asBinary().isEmpty();
        case SampleType::Multimodal:
            return asMultimodal().text.isEmpty() && 
                   asMultimodal().image.isNull() && 
                   asMultimodal().audio.samples.isEmpty();
    }
    return true;
}

qint64 DatasetSample::dataSize() const {
    switch (type_) {
        case SampleType::Text:
            return asText().size();
        case SampleType::Image: {
            QImage img = asImage();
            return img.sizeInBytes();
        }
        case SampleType::Audio:
            return asAudio().samples.size();
        case SampleType::Binary:
            return asBinary().size();
        case SampleType::Multimodal: {
            auto mm = asMultimodal();
            return mm.text.size() + mm.image.sizeInBytes() + mm.audio.samples.size();
        }
    }
    return 0;
}

} // namespace DatasetCreator
