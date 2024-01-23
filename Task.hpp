
#ifndef PPL_TASK_H
#define PPL_TASK_H

class Task {
public:
    bool is_eos = false;

    virtual void run() = 0;
    virtual ~Task() = default;

    void set_is_eos(bool is_eos) {
        this->is_eos = is_eos;
    }

    bool get_is_eos() const {
        return this->is_eos;
    }
};

#endif //PPL_TASK_H
