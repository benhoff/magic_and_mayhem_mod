#pragma once
#include <QApplication>
#include <QCommandLineParser>
void addMediaOptions(QCommandLineParser& parser);
int runMedia(QApplication& app,const QCommandLineParser& parser);
