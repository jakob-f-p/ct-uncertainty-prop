#pragma once

#include <QObject>


class VoidWorker : public QObject {
    Q_OBJECT

public:
    using WorkerFunction = std::function<void()>;

    explicit VoidWorker(WorkerFunction&& workerFunction) :
            Function(std::move(workerFunction)) {}

public Q_SLOTS:
    void DoWork() {
        Function();

        Q_EMIT Done();
    }

Q_SIGNALS:
    void Done();

private:
    WorkerFunction Function;
};
