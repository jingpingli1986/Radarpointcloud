#include "rspprocessor.h"

QLibrary RspProcessor::library("rspProcessing.dll");
RspProssingFunc RspProcessor::funcPtr = nullptr;
bool RspProcessor::initialized = false;

void RspProcessor::initialize() {
    if (initialized) {
        return; // 已经初始化，直接退出
    }

    qDebug() << "Attempting to load DLL...";
    if (!library.load()) {
        qCritical() << "Error: Failed to load rspProcessing.dll:" << library.errorString();
        initialized = true; // 标记为已尝试初始化，即使失败
        return;
    }

    qDebug() << "DLL loaded successfully. Resolving symbol...";
    funcPtr = (RspProssingFunc)library.resolve("rspProssing");

    if (!funcPtr) {
        qCritical() << "Error: Failed to resolve function 'rspProssing':" << library.errorString();
    } else {
        qDebug() << "Function 'rspProssing' resolved successfully.";
    }

    initialized = true;
}

RspProssingFunc RspProcessor::getRspProssingFunction() {
    // 第一次调用时触发初始化
    if (!initialized) {
        initialize();
    }
    return funcPtr;
}
