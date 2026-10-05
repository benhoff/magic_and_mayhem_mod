#pragma once
class QApplication;
class QMainWindow;
class LiveMenuSession;
class QString;
void installLiveMenuTest(QApplication&,QMainWindow&,LiveMenuSession&,const QString&);

void installLiveBattleMenuTest(QApplication&,QMainWindow&,LiveMenuSession&,const QString&);

void installLivePreferencesMenuTest(QApplication&,QMainWindow&,LiveMenuSession&,const QString&);

void installLivePreferencesRestoreTest(QApplication&,QMainWindow&,LiveMenuSession&,const QString&);

void installLivePreferencesDisplayTest(QApplication&,QMainWindow&,LiveMenuSession&,const QString&);

void installLiveRegionEntryTest(QApplication&,QMainWindow&,LiveMenuSession&,const QString&);

void installLiveRegionEnterTest(QApplication&,QMainWindow&,LiveMenuSession&,const QString&);
