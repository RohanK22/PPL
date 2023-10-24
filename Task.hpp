
#ifndef PPL_TASK_H
#define PPL_TASK_H

class Task {
public:
    virtual void run() = 0;
    virtual ~Task() = default;
};

#endif //PPL_TASK_H