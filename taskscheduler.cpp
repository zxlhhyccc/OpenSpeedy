/*
 * OpenSpeedy - Open Source Game Speed Controller
 * Copyright (C) 2025 Game1024
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "taskscheduler.h"
#include <QDebug>
#include <QDir>
TaskScheduler::TaskScheduler(QObject* parent)
  : QObject{ parent }
  , pService(nullptr)
{
    initializeTaskService();
}

TaskScheduler::~TaskScheduler()
{
    cleanup();
}

bool
TaskScheduler::createStartupTask(const QString& taskName,
                                 const QString& executablePath)
{
    if (!pService)
        return false;

    HRESULT hr;
    ITaskFolder* pRootFolder = nullptr;
    ITaskDefinition* pTask = nullptr;
    IRegistrationInfo* pRegInfo = nullptr;
    IPrincipal* pPrincipal = nullptr;
    ITaskSettings* pSettings = nullptr;
    ITriggerCollection* pTriggerCollection = nullptr;
    ITrigger* pTrigger = nullptr;
    IBootTrigger* pBootTrigger = nullptr;
    IActionCollection* pActionCollection = nullptr;
    IAction* pAction = nullptr;
    IExecAction* pExecAction = nullptr;
    IRegisteredTask* pRegisteredTask = nullptr;

    do
    {
        // 获取根文件夹
        hr = pService->GetFolder(_bstr_t(L"\\"), &pRootFolder);
        if (FAILED(hr))
            break;

        // 创建任务定义
        hr = pService->NewTask(0, &pTask);
        if (FAILED(hr))
            break;

        // 设置注册信息
        hr = pTask->get_RegistrationInfo(&pRegInfo);
        if (FAILED(hr))
            break;

        hr = pRegInfo->put_Author(_bstr_t(L"OpenSpeedy"));
        if (FAILED(hr))
            break;

        hr = pRegInfo->put_Description(_bstr_t(L"OpenSpeedy Auto Start Task"));
        if (FAILED(hr))
            break;

        // 设置权限（以最高权限运行）
        hr = pTask->get_Principal(&pPrincipal);
        if (FAILED(hr))
            break;

        // 设置以最高权限运行
        hr = pPrincipal->put_RunLevel(TASK_RUNLEVEL_HIGHEST);
        if (FAILED(hr))
            break;

        // 设置登录类型
        hr = pPrincipal->put_LogonType(TASK_LOGON_INTERACTIVE_TOKEN);
        if (FAILED(hr))
            break;

        // 创建启动触发器
        hr = pTask->get_Triggers(&pTriggerCollection);
        if (FAILED(hr))
            break;

        hr = pTriggerCollection->Create(TASK_TRIGGER_BOOT, &pTrigger);
        if (FAILED(hr))
            break;

        hr = pTrigger->QueryInterface(IID_IBootTrigger, (void**)&pBootTrigger);
        if (FAILED(hr))
            break;

        hr = pBootTrigger->put_Id(_bstr_t(L"Trigger1"));
        if (FAILED(hr))
            break;

        // 设置延迟启动（可选）
        hr = pBootTrigger->put_Delay(_bstr_t(L"PT5S")); // 5秒延迟
        if (FAILED(hr))
            break;

        // 创建执行动作
        hr = pTask->get_Actions(&pActionCollection);
        if (FAILED(hr))
            break;

        hr = pActionCollection->Create(TASK_ACTION_EXEC, &pAction);
        if (FAILED(hr))
            break;

        hr = pAction->QueryInterface(IID_IExecAction, (void**)&pExecAction);
        if (FAILED(hr))
            break;

        hr =
          pExecAction->put_Path(_bstr_t(executablePath.toStdWString().c_str()));
        if (FAILED(hr))
            break;

        // 设置启动参数 最小化到托盘
        hr = pExecAction->put_Arguments(_bstr_t(L"--minimize-to-tray"));
        if (FAILED(hr))
            break;

        // 注册任务
        hr = pRootFolder->RegisterTaskDefinition(
          _bstr_t(taskName.toStdWString().c_str()),
          pTask,
          TASK_CREATE_OR_UPDATE,
          _variant_t(),
          _variant_t(),
          TASK_LOGON_INTERACTIVE_TOKEN,
          _variant_t(L""),
          &pRegisteredTask);

    } while (false);

    // 清理资源
    if (pRegisteredTask)
        pRegisteredTask->Release();
    if (pExecAction)
        pExecAction->Release();
    if (pAction)
        pAction->Release();
    if (pActionCollection)
        pActionCollection->Release();
    if (pBootTrigger)
        pBootTrigger->Release();
    if (pTrigger)
        pTrigger->Release();
    if (pTriggerCollection)
        pTriggerCollection->Release();
    if (pRegInfo)
        pRegInfo->Release();
    if (pTask)
        pTask->Release();
    if (pRootFolder)
        pRootFolder->Release();

    return SUCCEEDED(hr);
}

bool
TaskScheduler::deleteTask(const QString& taskName)
{
    if (!pService)
        return false;

    ITaskFolder* pRootFolder = nullptr;
    HRESULT hr = pService->GetFolder(_bstr_t(L"\\"), &pRootFolder);

    if (SUCCEEDED(hr))
    {
        hr =
          pRootFolder->DeleteTask(_bstr_t(taskName.toStdWString().c_str()), 0);
        pRootFolder->Release();
    }

    return SUCCEEDED(hr);
}

bool
TaskScheduler::isTaskExists(const QString& taskName)
{
    if (!pService)
        return false;

    ITaskFolder* pRootFolder = nullptr;
    IRegisteredTask* pRegisteredTask = nullptr;

    HRESULT hr = pService->GetFolder(_bstr_t(L"\\"), &pRootFolder);
    if (SUCCEEDED(hr))
    {
        hr = pRootFolder->GetTask(_bstr_t(taskName.toStdWString().c_str()),
                                  &pRegisteredTask);
        if (pRegisteredTask)
            pRegisteredTask->Release();
        pRootFolder->Release();
    }

    return SUCCEEDED(hr);
}

bool
TaskScheduler::enableTask(const QString& taskName, bool enable)
{
    if (!pService)
        return false;

    ITaskFolder* pRootFolder = nullptr;
    IRegisteredTask* pRegisteredTask = nullptr;

    HRESULT hr = pService->GetFolder(_bstr_t(L"\\"), &pRootFolder);
    if (FAILED(hr))
    {
        qDebug() << "Failed to get root folder:" << hr;
        return false;
    }

    do
    {
        // 获取指定的任务
        hr = pRootFolder->GetTask(_bstr_t(taskName.toStdWString().c_str()),
                                  &pRegisteredTask);
        if (FAILED(hr))
        {
            qDebug() << "Task not found:" << taskName;
            break;
        }

        // 启用或禁用任务
        hr =
          pRegisteredTask->put_Enabled(enable ? VARIANT_TRUE : VARIANT_FALSE);
        if (FAILED(hr))
        {
            qDebug() << "Failed to" << (enable ? "enable" : "disable")
                     << "task:" << hr;
            break;
        }

        qDebug() << "Task" << taskName << (enable ? "enabled" : "disabled")
                 << "successfully";

    } while (false);

    // 清理资源
    if (pRegisteredTask)
        pRegisteredTask->Release();
    if (pRootFolder)
        pRootFolder->Release();

    return SUCCEEDED(hr);
}

bool
TaskScheduler::initializeTaskService()
{
    // 先尝试初始化COM，如果已经初始化则忽略错误
    HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if (FAILED(hr) && hr != RPC_E_CHANGED_MODE)
    {
        qDebug() << "CoInitializeEx failed:" << hr;
        return false;
    }

    // 如果是RPC_E_CHANGED_MODE，尝试单线程模式
    if (hr == RPC_E_CHANGED_MODE)
    {
        hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
        if (FAILED(hr) && hr != RPC_E_CHANGED_MODE)
        {
            qDebug() << "CoInitializeEx with APARTMENTTHREADED failed:" << hr;
            return false;
        }
    }

    // 如果COM已经初始化，跳过安全初始化
    if (hr != RPC_E_CHANGED_MODE)
    {
        hr = CoInitializeSecurity(NULL,
                                  -1,
                                  NULL,
                                  NULL,
                                  RPC_C_AUTHN_LEVEL_PKT_PRIVACY,
                                  RPC_C_IMP_LEVEL_IMPERSONATE,
                                  NULL,
                                  0,
                                  NULL);
        if (FAILED(hr) && hr != RPC_E_TOO_LATE)
        {
            qDebug() << "CoInitializeSecurity failed:" << hr;
            CoUninitialize();
            return false;
        }
    }

    if (FAILED(hr) && hr != RPC_E_TOO_LATE)
    {
        qDebug() << "CoInitializeSecurity failed:" << hr;
        CoUninitialize();
        return false;
    }

    hr = CoCreateInstance(CLSID_TaskScheduler,
                          NULL,
                          CLSCTX_INPROC_SERVER,
                          IID_ITaskService,
                          (void**)&pService);
    if (FAILED(hr))
    {
        qDebug() << "Failed to create TaskService instance:" << hr;
        CoUninitialize();
        return false;
    }

    hr =
      pService->Connect(_variant_t(), _variant_t(), _variant_t(), _variant_t());
    if (FAILED(hr))
    {
        qDebug() << "ITaskService::Connect failed:" << hr;
        pService->Release();
        pService = nullptr;
        CoUninitialize();
        return false;
    }

    return true;
}

void
TaskScheduler::cleanup()
{
    if (pService)
    {
        pService->Release();
        pService = nullptr;
    }
    CoUninitialize();
}
