/****************************************************************************
**
** Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the QtScript module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** No Commercial Usage
** This file contains pre-release code and may not be distributed.
** You may use this file in accordance with the terms and conditions
** contained in the either Technology Preview License Agreement or the
** Beta Release License Agreement.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain
** additional rights. These rights are described in the Nokia Qt LGPL
** Exception version 1.0, included in the file LGPL_EXCEPTION.txt in this
** package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at http://qt.nokia.com/contact.
** $QT_END_LICENSE$
**
****************************************************************************/

#include "config.h"
#include "qscriptobject_p.h"
#include "private/qobject_p.h"

QT_BEGIN_NAMESPACE

namespace JSC
{
ASSERT_CLASS_FITS_IN_CELL(QScriptObject);
ASSERT_CLASS_FITS_IN_CELL(QScriptObjectPrototype);
}

// masquerading as JSC::JSObject
const JSC::ClassInfo QScriptObject::info = { "Object", 0, 0, 0 };

QScriptObject::Data::~Data()
{
    delete delegate;
}

QScriptObject::QScriptObject(WTF::PassRefPtr<JSC::Structure> sid)
    : JSC::JSObject(sid), d(0)
{
}

QScriptObject::~QScriptObject()
{
    delete d;
}

JSC::JSValue QScriptObject::data() const
{
    if (!d)
        return JSC::JSValue();
    return d->data;
}

void QScriptObject::setData(JSC::JSValue data)
{
    if (!d)
        d = new Data();
    d->data = data;
}

QScriptObjectDelegate *QScriptObject::delegate() const
{
    if (!d)
        return 0;
    return d->delegate;
}

void QScriptObject::setDelegate(QScriptObjectDelegate *delegate)
{
    if (!d)
        d = new Data();
    d->delegate = delegate;
}

bool QScriptObject::getOwnPropertySlot(JSC::ExecState* exec,
                                       const JSC::Identifier& propertyName,
                                       JSC::PropertySlot& slot)
{
    if (!d || !d->delegate)
        return JSC::JSObject::getOwnPropertySlot(exec, propertyName, slot);
    return d->delegate->getOwnPropertySlot(this, exec, propertyName, slot);
}

void QScriptObject::put(JSC::ExecState* exec, const JSC::Identifier& propertyName,
                        JSC::JSValue value, JSC::PutPropertySlot& slot)
{
    if (!d || !d->delegate) {
        JSC::JSObject::put(exec, propertyName, value, slot);
        return;
    }
    d->delegate->put(this, exec, propertyName, value, slot);
}

bool QScriptObject::deleteProperty(JSC::ExecState* exec,
                                   const JSC::Identifier& propertyName,
                                   bool checkDontDelete)
{
    if (!d || !d->delegate)
        return JSC::JSObject::deleteProperty(exec, propertyName, checkDontDelete);
    return d->delegate->deleteProperty(this, exec, propertyName, checkDontDelete);
}

bool QScriptObject::getPropertyAttributes(JSC::ExecState* exec, const JSC::Identifier& propertyName,
                                          unsigned& attributes) const
{
    if (!d || !d->delegate)
        return JSC::JSObject::getPropertyAttributes(exec, propertyName, attributes);
    return d->delegate->getPropertyAttributes(this, exec, propertyName, attributes);
}

void QScriptObject::getPropertyNames(JSC::ExecState* exec, JSC::PropertyNameArray& propertyNames, unsigned listedAttributes)
{
    if (!d || !d->delegate) {
        JSC::JSObject::getPropertyNames(exec, propertyNames, listedAttributes);
        return;
    }
    d->delegate->getPropertyNames(this, exec, propertyNames, listedAttributes);
}

void QScriptObject::mark()
{
    Q_ASSERT(!marked());
    if (!d)
        d = new Data();
    if (d->isMarking)
        return;
    QBoolBlocker markBlocker(d->isMarking, true);
    if (d && d->data && !d->data.marked())
        d->data.mark();
    if (!d || !d->delegate) {
        JSC::JSObject::mark();
        return;
    }
    d->delegate->mark(this);
}

JSC::CallType QScriptObject::getCallData(JSC::CallData &data)
{
    if (!d || !d->delegate)
        return JSC::JSObject::getCallData(data);
    return d->delegate->getCallData(this, data);
}

JSC::ConstructType QScriptObject::getConstructData(JSC::ConstructData &data)
{
    if (!d || !d->delegate)
        return JSC::JSObject::getConstructData(data);
    return d->delegate->getConstructData(this, data);
}

bool QScriptObject::hasInstance(JSC::ExecState* exec, JSC::JSValue value, JSC::JSValue proto)
{
    if (!d || !d->delegate)
        return JSC::JSObject::hasInstance(exec, value, proto);
    return d->delegate->hasInstance(this, exec, value, proto);
}

QScriptObjectPrototype::QScriptObjectPrototype(JSC::ExecState*, WTF::PassRefPtr<JSC::Structure> structure,
                                               JSC::Structure* /*prototypeFunctionStructure*/)
    : QScriptObject(structure)
{
}

QScriptObjectDelegate::QScriptObjectDelegate()
{
}

QScriptObjectDelegate::~QScriptObjectDelegate()
{
}

bool QScriptObjectDelegate::getOwnPropertySlot(QScriptObject* object, JSC::ExecState* exec,
                                               const JSC::Identifier& propertyName,
                                               JSC::PropertySlot& slot)
{
    return object->JSC::JSObject::getOwnPropertySlot(exec, propertyName, slot);
}

void QScriptObjectDelegate::put(QScriptObject* object, JSC::ExecState* exec,
                                const JSC::Identifier& propertyName,
                                JSC::JSValue value, JSC::PutPropertySlot& slot)
{
    object->JSC::JSObject::put(exec, propertyName, value, slot);
}

bool QScriptObjectDelegate::deleteProperty(QScriptObject* object, JSC::ExecState* exec,
                                           const JSC::Identifier& propertyName,
                                           bool checkDontDelete)
{
    return object->JSC::JSObject::deleteProperty(exec, propertyName, checkDontDelete);
}

bool QScriptObjectDelegate::getPropertyAttributes(const QScriptObject* object,
                                                  JSC::ExecState* exec,
                                                  const JSC::Identifier& propertyName,
                                                  unsigned& attributes) const
{
    return object->JSC::JSObject::getPropertyAttributes(exec, propertyName, attributes);
}

void QScriptObjectDelegate::getPropertyNames(QScriptObject* object, JSC::ExecState* exec,
                                             JSC::PropertyNameArray& propertyNames,
                                             unsigned listedAttributes)
{
    object->JSC::JSObject::getPropertyNames(exec, propertyNames, listedAttributes);
}

void QScriptObjectDelegate::mark(QScriptObject* object)
{
    if (!object->marked())
        object->JSC::JSObject::mark();
}

JSC::CallType QScriptObjectDelegate::getCallData(QScriptObject* object, JSC::CallData& data)
{
    return object->JSC::JSObject::getCallData(data);
}

JSC::ConstructType QScriptObjectDelegate::getConstructData(QScriptObject* object, JSC::ConstructData& data)
{
    return object->JSC::JSObject::getConstructData(data);
}

bool QScriptObjectDelegate::hasInstance(QScriptObject* object, JSC::ExecState* exec,
                                        JSC::JSValue value, JSC::JSValue proto)
{
    return object->JSC::JSObject::hasInstance(exec, value, proto);
}

QT_END_NAMESPACE