#pragma once
#include <QApplication>
#include <QCommandLineParser>
void addAudioOptions(QCommandLineParser&);
int runAudio(QApplication&,const QCommandLineParser&);
