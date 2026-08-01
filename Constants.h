#pragma once
#include <QStringList>
#include <QColor>

namespace MediaCategories {
    inline const QStringList ImageExtensions = {"jpg", "jpeg", "png", "bmp", "webp", "gif", "ico"};
    inline const QStringList VideoExtensions = {"mp4", "mkv", "webm", "mov", "avi", "flv"};
    inline const QStringList AudioExtensions = {"mp3", "aac", "wav", "flac", "ogg", "m4a"};

    inline const QStringList ImageProfiles = {"gif", "jpg", "jpeg", "webp", "ico", "bmp"};
    inline const QStringList VideoProfiles = {"mp4", "mp4_nvenc", "mp4_amd_vaapi", "mkv", "webm", "av1", "davinci_cuda_full", "davinci_amd_full", "prores_cuda_full"};
    inline const QStringList AudioProfiles = {"mp3", "aac", "wav"};
}

namespace Mocha {
    inline const QColor Base      (0x1e, 0x1e, 0x2e);
    inline const QColor Surface   (0x31, 0x32, 0x44);
    inline const QColor Text      (0xcd, 0xd6, 0xf4);
    inline const QColor Subtext   (0xa6, 0xad, 0xc8);
    inline const QColor Accent    (0xcb, 0xa6, 0xf7);
    inline const QColor Green     (0xa6, 0xe3, 0xa1);
    inline const QColor Red       (0xf3, 0x8b, 0xa8);
    inline const QColor Yellow    (0xf9, 0xe2, 0xaf);
    inline const QColor Hover     (0x45, 0x47, 0x5a);
}