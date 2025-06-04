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
    int start_time;
    int finish_time;
    int waiting_time;
    int turnaround_time;
    int remaining_time;
    QColor color;

    // Constructor predeterminado
    Process() : burst_time(0), arrival_time(0), priority(0), start_time(-1),
                finish_time(-1), waiting_time(-1), turnaround_time(-1), remaining_time(0), color(Qt::white) {}

    // Constructor con parámetros
    Process(QString p, int bt, int at, int pr, int st, int ft, int wt, int tat, QColor c)
        : pid(p), burst_time(bt), arrival_time(at), priority(pr),
          start_time(st), finish_time(ft), waiting_time(wt), turnaround_time(tat),
          remaining_time(bt), color(c) {}
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
    Resource(QString n, int c, int a) : name(n), count(c), available(a) {}
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