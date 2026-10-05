#pragma once
#include <QString>
// Complete-file admission followed by timer-driven persistent GPU execution.
// Requires an existing QApplication; owns its standalone viewport/event loop.
int runCommandReplay(const QString& path,bool smokeTest,bool verifyChecks=false);
