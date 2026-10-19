#pragma once
#include <QFile>
#include <QTemporaryFile>
#include <cstdio>

// CPU replies are volatile IPC. Publish a complete same-directory file with
// atomic rename, without waiting for durable disk writeback. The reader still
// validates the complete session/ticket/history header and exact payload size.
inline bool publishCpuReplyFile(const QString& path,const QByteArray& bytes){
    QTemporaryFile file(path+QStringLiteral(".XXXXXX"));
    if(!file.open() || file.write(bytes)!=bytes.size() || !file.flush())return false;
    file.close();
    if(file.error()!=QFileDevice::NoError)return false;
    const auto source=QFile::encodeName(file.fileName()),destination=QFile::encodeName(path);
    // QFile::rename refuses an existing destination; POSIX rename replaces it
    // atomically. Failed publication preserves the previous reply and removes
    // this temporary file through QTemporaryFile's normal cleanup.
    return std::rename(source.constData(),destination.constData())==0;
}
