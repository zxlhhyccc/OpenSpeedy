#ifndef TASKSCHEDULER_H
#define TASKSCHEDULER_H

#include <windows.h>
#include <QObject>
#include <QString>
#include <comdef.h>
#include <taskschd.h>
class TaskScheduler : public QObject
{
    Q_OBJECT
  public:
    explicit TaskScheduler(QObject* parent = nullptr);
    ~TaskScheduler();

    bool createStartupTask(const QString& taskName,
                           const QString& executablePath);
    bool deleteTask(const QString& taskName);
    bool isTaskExists(const QString& taskName);
    bool enableTask(const QString& taskName, bool enable);

  private:
    ITaskService* pService;
    bool initializeTaskService();
    void cleanup();
  signals:
};

#endif // TASKSCHEDULER_H
