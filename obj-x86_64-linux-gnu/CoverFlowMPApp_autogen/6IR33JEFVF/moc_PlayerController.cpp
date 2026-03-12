/****************************************************************************
** Meta object code from reading C++ file 'PlayerController.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.4.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../src/player/PlayerController.h"
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'PlayerController.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 68
#error "This file was generated using the moc from 6.4.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
namespace {
struct qt_meta_stringdata_PlayerController_t {
    uint offsetsAndSizes[40];
    char stringdata0[17];
    char stringdata1[14];
    char stringdata2[1];
    char stringdata3[16];
    char stringdata4[16];
    char stringdata5[18];
    char stringdata6[9];
    char stringdata7[9];
    char stringdata8[14];
    char stringdata9[11];
    char stringdata10[12];
    char stringdata11[13];
    char stringdata12[8];
    char stringdata13[5];
    char stringdata14[15];
    char stringdata15[13];
    char stringdata16[14];
    char stringdata17[7];
    char stringdata18[9];
    char stringdata19[9];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_PlayerController_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_PlayerController_t qt_meta_stringdata_PlayerController = {
    {
        QT_MOC_LITERAL(0, 16),  // "PlayerController"
        QT_MOC_LITERAL(17, 13),  // "pausedChanged"
        QT_MOC_LITERAL(31, 0),  // ""
        QT_MOC_LITERAL(32, 15),  // "positionChanged"
        QT_MOC_LITERAL(48, 15),  // "durationChanged"
        QT_MOC_LITERAL(64, 17),  // "audioDelayChanged"
        QT_MOC_LITERAL(82, 8),  // "playFile"
        QT_MOC_LITERAL(91, 8),  // "filePath"
        QT_MOC_LITERAL(100, 13),  // "startPosition"
        QT_MOC_LITERAL(114, 10),  // "audioDelay"
        QT_MOC_LITERAL(125, 11),  // "togglePause"
        QT_MOC_LITERAL(137, 12),  // "seekRelative"
        QT_MOC_LITERAL(150, 7),  // "seconds"
        QT_MOC_LITERAL(158, 4),  // "stop"
        QT_MOC_LITERAL(163, 14),  // "attachToWindow"
        QT_MOC_LITERAL(178, 12),  // "windowObject"
        QT_MOC_LITERAL(191, 13),  // "setAudioDelay"
        QT_MOC_LITERAL(205, 6),  // "paused"
        QT_MOC_LITERAL(212, 8),  // "position"
        QT_MOC_LITERAL(221, 8)   // "duration"
    },
    "PlayerController",
    "pausedChanged",
    "",
    "positionChanged",
    "durationChanged",
    "audioDelayChanged",
    "playFile",
    "filePath",
    "startPosition",
    "audioDelay",
    "togglePause",
    "seekRelative",
    "seconds",
    "stop",
    "attachToWindow",
    "windowObject",
    "setAudioDelay",
    "paused",
    "position",
    "duration"
};
#undef QT_MOC_LITERAL
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_PlayerController[] = {

 // content:
      10,       // revision
       0,       // classname
       0,    0, // classinfo
      12,   14, // methods
       4,  116, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       4,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,   86,    2, 0x06,    5 /* Public */,
       3,    0,   87,    2, 0x06,    6 /* Public */,
       4,    0,   88,    2, 0x06,    7 /* Public */,
       5,    0,   89,    2, 0x06,    8 /* Public */,

 // methods: name, argc, parameters, tag, flags, initial metatype offsets
       6,    3,   90,    2, 0x02,    9 /* Public */,
       6,    2,   97,    2, 0x22,   13 /* Public | MethodCloned */,
       6,    1,  102,    2, 0x22,   16 /* Public | MethodCloned */,
      10,    0,  105,    2, 0x02,   18 /* Public */,
      11,    1,  106,    2, 0x02,   19 /* Public */,
      13,    0,  109,    2, 0x02,   21 /* Public */,
      14,    1,  110,    2, 0x02,   22 /* Public */,
      16,    1,  113,    2, 0x02,   24 /* Public */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

 // methods: parameters
    QMetaType::Void, QMetaType::QString, QMetaType::Double, QMetaType::Double,    7,    8,    9,
    QMetaType::Void, QMetaType::QString, QMetaType::Double,    7,    8,
    QMetaType::Void, QMetaType::QString,    7,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Double,   12,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QObjectStar,   15,
    QMetaType::Void, QMetaType::Double,   12,

 // properties: name, type, flags
      17, QMetaType::Bool, 0x00015001, uint(0), 0,
      18, QMetaType::Double, 0x00015001, uint(1), 0,
      19, QMetaType::Double, 0x00015001, uint(2), 0,
       9, QMetaType::Double, 0x00015001, uint(3), 0,

       0        // eod
};

Q_CONSTINIT const QMetaObject PlayerController::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_PlayerController.offsetsAndSizes,
    qt_meta_data_PlayerController,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_PlayerController_t,
        // property 'paused'
        QtPrivate::TypeAndForceComplete<bool, std::true_type>,
        // property 'position'
        QtPrivate::TypeAndForceComplete<double, std::true_type>,
        // property 'duration'
        QtPrivate::TypeAndForceComplete<double, std::true_type>,
        // property 'audioDelay'
        QtPrivate::TypeAndForceComplete<double, std::true_type>,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<PlayerController, std::true_type>,
        // method 'pausedChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'positionChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'durationChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'audioDelayChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'playFile'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        QtPrivate::TypeAndForceComplete<double, std::false_type>,
        QtPrivate::TypeAndForceComplete<double, std::false_type>,
        // method 'playFile'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        QtPrivate::TypeAndForceComplete<double, std::false_type>,
        // method 'playFile'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'togglePause'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'seekRelative'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<double, std::false_type>,
        // method 'stop'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'attachToWindow'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<QObject *, std::false_type>,
        // method 'setAudioDelay'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<double, std::false_type>
    >,
    nullptr
} };

void PlayerController::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<PlayerController *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->pausedChanged(); break;
        case 1: _t->positionChanged(); break;
        case 2: _t->durationChanged(); break;
        case 3: _t->audioDelayChanged(); break;
        case 4: _t->playFile((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<double>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<double>>(_a[3]))); break;
        case 5: _t->playFile((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<double>>(_a[2]))); break;
        case 6: _t->playFile((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 7: _t->togglePause(); break;
        case 8: _t->seekRelative((*reinterpret_cast< std::add_pointer_t<double>>(_a[1]))); break;
        case 9: _t->stop(); break;
        case 10: _t->attachToWindow((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1]))); break;
        case 11: _t->setAudioDelay((*reinterpret_cast< std::add_pointer_t<double>>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (PlayerController::*)();
            if (_t _q_method = &PlayerController::pausedChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (PlayerController::*)();
            if (_t _q_method = &PlayerController::positionChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (PlayerController::*)();
            if (_t _q_method = &PlayerController::durationChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (PlayerController::*)();
            if (_t _q_method = &PlayerController::audioDelayChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 3;
                return;
            }
        }
    }else if (_c == QMetaObject::ReadProperty) {
        auto *_t = static_cast<PlayerController *>(_o);
        (void)_t;
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< bool*>(_v) = _t->paused(); break;
        case 1: *reinterpret_cast< double*>(_v) = _t->position(); break;
        case 2: *reinterpret_cast< double*>(_v) = _t->duration(); break;
        case 3: *reinterpret_cast< double*>(_v) = _t->audioDelay(); break;
        default: break;
        }
    } else if (_c == QMetaObject::WriteProperty) {
    } else if (_c == QMetaObject::ResetProperty) {
    } else if (_c == QMetaObject::BindableProperty) {
    }
}

const QMetaObject *PlayerController::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *PlayerController::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_PlayerController.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int PlayerController::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 12)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 12;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 12)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 12;
    }else if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::BindableProperty
            || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    }
    return _id;
}

// SIGNAL 0
void PlayerController::pausedChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void PlayerController::positionChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void PlayerController::durationChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void PlayerController::audioDelayChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
