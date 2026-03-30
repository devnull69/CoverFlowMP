#pragma once

#include <QAbstractListModel>
#include <QVector>
#include "VideoItem.h"

class VideoLibraryModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Roles {
        TitleRole = Qt::UserRole + 1,
        FilePathRole,
        CoverPathRole,
        DurationRole,
        TotalDurationRole,
        ResumePositionRole,
        IsDemoRole,
        IsFolderRole,
        IsParentFolderRole,
        IsVirtualRootEntryRole
    };

    explicit VideoLibraryModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    void setItems(const QVector<VideoItem> &items);
    VideoItem itemAt(int index) const;
    void updateResumePosition(const QString &filePath, double position);
    void updateDuration(const QString &filePath, double duration, double totalDuration);
    void updatePlaybackState(const QString &filePath, double position, double duration, double totalDuration);
    void updateCoverPath(const QString &filePath, const QString &coverPath);

private:
    QVector<VideoItem> m_items;
};
