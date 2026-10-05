#pragma once
class QApplication;
class QMainWindow;
class LiveMenuSession;
class QString;
void installLiveMenuTest(QApplication&,QMainWindow&,LiveMenuSession&,const QString&);

void installLiveBattleMenuTest(QApplication&,QMainWindow&,LiveMenuSession&,const QString&);

void installLivePreferencesMenuTest(QApplication&,QMainWindow&,LiveMenuSession&,const QString&);
