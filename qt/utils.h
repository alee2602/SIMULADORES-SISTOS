#ifndef UTILS_H
#define UTILS_H

#include <QString>
#include <QColor>
#include <vector>

struct Process {
    QString pid;
    int burst_time;
    int arrival_time;
    int priority;
    int start_time = -1;
    int finish_time = -1;
    int waiting_time = -1;
    int remaining_time = -1;
    QColor color;
    
    Process() = default;
    Process(QString p, int bt, int at, int prio) 
        : pid(p), burst_time(bt), arrival_time(at), priority(prio) {
        remaining_time = burst_time;
    }
};

struct ExecutionSlice {
    QString pid;
    int start_time;
    int duration;
    QColor color;
    
    ExecutionSlice(QString p, int start, int dur, QColor c)
        : pid(p), start_time(start), duration(dur), color(c) {}
};

struct Resource {
    QString name;
    int count;
    int available;
    Resource() : name(""), count(0), available(0) {}
    Resource(QString n, int c) : name(n), count(c), available(c) {}
};

struct Action {
    QString pid;     
    QString type;    
    QString resource; 
    int cycle;      

    Action() : pid(""), type(""), resource(""), cycle(0) {}

    Action(QString p, QString t, QString r, int c)
        : pid(p), type(t), resource(r), cycle(c) {}
};

#endif 
