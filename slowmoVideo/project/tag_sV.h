#ifndef TAG_SV_H
#define TAG_SV_H

#include <QString>

class Tag_sV
{
public:
    enum TagAxis { TagAxis_Source };

    Tag_sV(TagAxis axis = TagAxis_Source);
    Tag_sV(qreal time, QString description, TagAxis axis = TagAxis_Source);

    TagAxis axis() const { return m_axis; }
    qreal time() const { return m_time; }
    const QString& description() const { return m_description; }

    void setTime(qreal time);
    void setDescription(QString desc);

private:
    TagAxis m_axis;
    qreal m_time;
    QString m_description;

};

#endif // TAG_SV_H
