#include "VideoLibraryModel.h"

VideoLibraryModel::VideoLibraryModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int VideoLibraryModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_items.size();
}

QVariant VideoLibraryModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_items.size())
        return {};

    const auto &item = m_items.at(index.row());

    switch (role) {
    case TitleRole: return item.title;
    case FilePathRole: return item.filePath;
    case CoverPathRole: return item.coverPath;
    case DurationRole: return item.duration;
    case ResumePositionRole: return item.resumePosition;
    case IsDemoRole: return item.isDemo;
    case IsFolderRole: return item.isFolder;
    case IsParentFolderRole: return item.isParentFolder;
    default: return {};
    }
}

QHash<int, QByteArray> VideoLibraryModel::roleNames() const
{
    return {
        { TitleRole, "title" },
        { FilePathRole, "filePath" },
        { CoverPathRole, "coverPath" },
        { DurationRole, "duration" },
        { ResumePositionRole, "resumePosition" },
        { IsDemoRole, "isDemo" },
        { IsFolderRole, "isFolder" },
        { IsParentFolderRole, "isParentFolder" }
    };
}

void VideoLibraryModel::setItems(const QVector<VideoItem> &items)
{
    beginResetModel();
    m_items = items;
    endResetModel();
}

VideoItem VideoLibraryModel::itemAt(int index) const
{
    if (index < 0 || index >= m_items.size())
        return {};
    return m_items.at(index);
}

void VideoLibraryModel::updateResumePosition(const QString &filePath, double position)
{
    for (int i = 0; i < m_items.size(); ++i) {
        if (m_items[i].filePath == filePath) {
            m_items[i].resumePosition = position;
            const QModelIndex idx = index(i);
            emit dataChanged(idx, idx, { ResumePositionRole });
            return;
        }
    }
}

void VideoLibraryModel::updateCoverPath(const QString &filePath, const QString &coverPath)
{
    for (int i = 0; i < m_items.size(); ++i) {
        if (m_items[i].filePath == filePath) {
            if (m_items[i].coverPath == coverPath)
                return;

            m_items[i].coverPath = coverPath;
            const QModelIndex idx = index(i);
            emit dataChanged(idx, idx, { CoverPathRole });
            return;
        }
    }
}
