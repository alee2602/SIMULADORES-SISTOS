/****************************************************************************
** Meta object code from reading C++ file 'processsimulator.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.2.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../processsimulator.h"
#include <QtGui/qtextcursor.h>
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'processsimulator.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 68
#error "This file was generated using the moc from 6.2.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_ProcessSimulator_t {
    const uint offsetsAndSize[20];
    char stringdata0[169];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(offsetof(qt_meta_stringdata_ProcessSimulator_t, stringdata0) + ofs), len 
static const qt_meta_stringdata_ProcessSimulator_t qt_meta_stringdata_ProcessSimulator = {
    {
QT_MOC_LITERAL(0, 16), // "ProcessSimulator"
QT_MOC_LITERAL(17, 21), // "returnToMenuRequested"
QT_MOC_LITERAL(39, 0), // ""
QT_MOC_LITERAL(40, 26), // "runNextAlgorithmInSequence"
QT_MOC_LITERAL(67, 14), // "cleanProcesses"
QT_MOC_LITERAL(82, 26), // "displayComparisonTableOnly"
QT_MOC_LITERAL(109, 10), // "algorithms"
QT_MOC_LITERAL(120, 19), // "std::vector<double>"
QT_MOC_LITERAL(140, 12), // "waitingTimes"
QT_MOC_LITERAL(153, 15) // "turnaroundTimes"

    },
    "ProcessSimulator\0returnToMenuRequested\0"
    "\0runNextAlgorithmInSequence\0cleanProcesses\0"
    "displayComparisonTableOnly\0algorithms\0"
    "std::vector<double>\0waitingTimes\0"
    "turnaroundTimes"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_ProcessSimulator[] = {

 // content:
      10,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,   38,    2, 0x06,    1 /* Public */,

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
       3,    0,   39,    2, 0x08,    2 /* Private */,
       4,    0,   40,    2, 0x08,    3 /* Private */,
       5,    3,   41,    2, 0x08,    4 /* Private */,

 // signals: parameters
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QStringList, 0x80000000 | 7, 0x80000000 | 7,    6,    8,    9,

       0        // eod
};

void ProcessSimulator::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<ProcessSimulator *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->returnToMenuRequested(); break;
        case 1: _t->runNextAlgorithmInSequence(); break;
        case 2: _t->cleanProcesses(); break;
        case 3: _t->displayComparisonTableOnly((*reinterpret_cast< std::add_pointer_t<QStringList>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<std::vector<double>>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<std::vector<double>>>(_a[3]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (ProcessSimulator::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ProcessSimulator::returnToMenuRequested)) {
                *result = 0;
                return;
            }
        }
    }
}

const QMetaObject ProcessSimulator::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_ProcessSimulator.offsetsAndSize,
    qt_meta_data_ProcessSimulator,
    qt_static_metacall,
    nullptr,
qt_incomplete_metaTypeArray<qt_meta_stringdata_ProcessSimulator_t
, QtPrivate::TypeAndForceComplete<ProcessSimulator, std::true_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>
, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<const QStringList &, std::false_type>, QtPrivate::TypeAndForceComplete<const std::vector<double> &, std::false_type>, QtPrivate::TypeAndForceComplete<const std::vector<double> &, std::false_type>


>,
    nullptr
} };


const QMetaObject *ProcessSimulator::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *ProcessSimulator::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_ProcessSimulator.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int ProcessSimulator::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 4)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 4)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 4;
    }
    return _id;
}

// SIGNAL 0
void ProcessSimulator::returnToMenuRequested()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
