#pragma once

#include "Metadata.h"
#include <QString>
#include <QVariant>
#include <QImage>
#include <QByteArray>
#include <QAudioFormat>
#include <memory>

namespace DatasetCreator {

/**
 * @brief Sample data type enumeration
 */
enum class SampleType {
    Text,          // Plain text data
    Image,         // Image data (QImage)
    Audio,         // Audio data (raw PCM samples)
    Binary,        // Generic binary data
    Multimodal     // Multiple data types combined
};

/**
 * @brief Audio data container
 */
struct AudioData {
    QByteArray samples;          // Raw audio samples (PCM)
    QAudioFormat format;         // Audio format information
    qint64 durationMs;          // Duration in milliseconds
    
    QVariantMap toVariantMap() const;
    static AudioData fromVariantMap(const QVariantMap& map);
};

/**
 * @brief Multimodal data container
 */
struct MultimodalData {
    QString text;
    QImage image;
    AudioData audio;
    QVariantMap additionalData;
    
    QVariantMap toVariantMap() const;
    static MultimodalData fromVariantMap(const QVariantMap& map);
};

/**
 * @brief Dataset sample - represents a single data point in the dataset
 */
class DatasetSample {
public:
    DatasetSample();
    explicit DatasetSample(SampleType type);
    
    // Type accessors
    SampleType type() const { return type_; }
    void setType(SampleType type) { type_ = type; }
    
    // Data accessors (type-safe getters)
    QString asText() const;
    QImage asImage() const;
    AudioData asAudio() const;
    QByteArray asBinary() const;
    MultimodalData asMultimodal() const;
    
    // Data setters
    void setText(const QString& text);
    void setImage(const QImage& image);
    void setAudio(const AudioData& audio);
    void setBinary(const QByteArray& data);
    void setMultimodal(const MultimodalData& data);
    
    // Generic data access (for custom types)
    QVariant data() const { return data_; }
    void setData(const QVariant& data) { data_ = data; }
    
    // Metadata accessors
    SampleMetadata& metadata() { return metadata_; }
    const SampleMetadata& metadata() const { return metadata_; }
    void setMetadata(const SampleMetadata& meta) { metadata_ = meta; }
    
    // Serialization
    QVariantMap toVariantMap() const;
    static DatasetSample fromVariantMap(const QVariantMap& map);
    
    // Utility
    bool isEmpty() const;
    qint64 dataSize() const;  // Approximate size in bytes
    
private:
    SampleType type_;
    QVariant data_;
    SampleMetadata metadata_;
};

} // namespace DatasetCreator
