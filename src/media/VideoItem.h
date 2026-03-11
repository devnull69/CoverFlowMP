#pragma once

#include <QString>

struct VideoItem
{
    QString title;
    QString filePath;
    QString coverPath;
    double duration = 0.0;
    double resumePosition = 0.0;
    bool isDemo = false;
};
