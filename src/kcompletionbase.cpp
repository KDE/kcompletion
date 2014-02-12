/* This file is part of the KDE libraries

   Copyright (c) 2000 Dawit Alemayehu <adawit@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License (LGPL) as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include <kcompletionbase.h>

#include <kcompletion.h>

#include <QtCore/QMap>
#include <QtCore/QObject>

class KCompletionBasePrivate
{
public:
    KCompletionBasePrivate()
    // Assign the default completion type to use.
        : completionMode(KCompletion::CompletionPopup)
        , delegate(0)
    {
    }
    ~KCompletionBasePrivate()
    {
        if (autoDeletesCompletionObject && completionObject) {
            delete completionObject;
        }
    }

    bool autoDeletesCompletionObject;
    bool handlesCompletionSignals;
    bool emitsRotationSignals;
    KCompletion::CompletionMode completionMode;
    QPointer<KCompletion> completionObject;
    KCompletionBase::KeyBindingMap keyBindingMap;
    // we may act as a proxy to another KCompletionBase object
    KCompletionBase *delegate;
};

KCompletionBase::KCompletionBase()
    : d(new KCompletionBasePrivate)
{
    // Initialize all key-bindings to 0 by default so that
    // the event filter will use the global settings.
    useGlobalKeyBindings();

    // By default we initialize everything except hsig to false.
    // All the variables would be setup properly when
    // the appropriate member functions are called.
    setup(false, true, false);
}

KCompletionBase::~KCompletionBase()
{
    delete d;
}

void KCompletionBase::setDelegate(KCompletionBase *delegate)
{
    d->delegate = delegate;

    if (delegate) {
        delegate->d->autoDeletesCompletionObject = d->autoDeletesCompletionObject;
        delegate->d->handlesCompletionSignals = d->handlesCompletionSignals;
        delegate->d->emitsRotationSignals = d->emitsRotationSignals;
        delegate->d->completionMode = d->completionMode;
        delegate->d->keyBindingMap = d->keyBindingMap;
    }
}

KCompletionBase *KCompletionBase::delegate() const
{
    return d->delegate;
}

KCompletion *KCompletionBase::completionObject(bool hsig)
{
    if (d->delegate) {
        return d->delegate->completionObject(hsig);
    }

    if (!d->completionObject) {
        setCompletionObject(new KCompletion(), hsig);
        d->autoDeletesCompletionObject = true;
    }
    return d->completionObject;
}

void KCompletionBase::setCompletionObject(KCompletion *completionObject, bool handleCompletionSignals)
{
    if (d->delegate) {
        d->delegate->setCompletionObject(completionObject, handleCompletionSignals);
        return;
    }

    if (d->autoDeletesCompletionObject && completionObject != d->completionObject) {
        delete d->completionObject;
    }

    d->completionObject = completionObject;

    // We emit rotation and completion signals
    // if completion object is not NULL.
    setup(false, handleCompletionSignals, !d->completionObject.isNull());
}

// BC: Inline this function and possibly rename it to setHandleEvents??? (DA)
void KCompletionBase::setHandleSignals(bool handle)
{
    if (d->delegate) {
        d->delegate->setHandleSignals(handle);
    } else {
        d->handlesCompletionSignals = handle;
    }
}

bool KCompletionBase::isCompletionObjectAutoDeleted() const
{
    return d->delegate ? d->delegate->isCompletionObjectAutoDeleted()
           : d->autoDeletesCompletionObject;
}

void KCompletionBase::setAutoDeleteCompletionObject(bool autoDelete)
{
    if (d->delegate) {
        d->delegate->setAutoDeleteCompletionObject(autoDelete);
    } else {
        d->autoDeletesCompletionObject = autoDelete;
    }
}

void KCompletionBase::setEnableSignals(bool enable)
{
    if (d->delegate) {
        d->delegate->setEnableSignals(enable);
    } else {
        d->emitsRotationSignals = enable;
    }
}

bool KCompletionBase::handleSignals() const
{
    return d->delegate ? d->delegate->handleSignals() : d->handlesCompletionSignals;
}

bool KCompletionBase::emitSignals() const
{
    return d->delegate ? d->delegate->emitSignals() : d->emitsRotationSignals;
}

void KCompletionBase::setCompletionMode(KCompletion::CompletionMode mode)
{
    if (d->delegate) {
        d->delegate->setCompletionMode(mode);
        return;
    }

    d->completionMode = mode;
    // Always sync up KCompletion mode with ours as long as we
    // are performing completions.
    if (d->completionObject && d->completionMode != KCompletion::CompletionNone) {
        d->completionObject->setCompletionMode(d->completionMode);
    }
}

KCompletion::CompletionMode KCompletionBase::completionMode() const
{
    return d->delegate ? d->delegate->completionMode() : d->completionMode;
}

bool KCompletionBase::setKeyBinding(KeyBindingType item, const QList<QKeySequence> &cut)
{
    if (d->delegate) {
        return d->delegate->setKeyBinding(item, cut);
    }

    if (!cut.isEmpty()) {
        for (KeyBindingMap::Iterator it = d->keyBindingMap.begin(); it != d->keyBindingMap.end(); ++it)
            if (it.value() == cut) {
                return false;
            }
    }
    d->keyBindingMap.insert(item, cut);
    return true;
}

QList<QKeySequence> KCompletionBase::getKeyBinding(KeyBindingType item) const
{
    return d->delegate ? d->delegate->getKeyBinding(item) : d->keyBindingMap[ item ];
}

void KCompletionBase::useGlobalKeyBindings()
{
    if (d->delegate) {
        d->delegate->useGlobalKeyBindings();
        return;
    }

    d->keyBindingMap.clear();
    d->keyBindingMap.insert(TextCompletion, QList<QKeySequence>());
    d->keyBindingMap.insert(PrevCompletionMatch, QList<QKeySequence>());
    d->keyBindingMap.insert(NextCompletionMatch, QList<QKeySequence>());
    d->keyBindingMap.insert(SubstringCompletion, QList<QKeySequence>());
}

KCompletion *KCompletionBase::compObj() const
{
    return d->delegate ? d->delegate->compObj()
           : static_cast<KCompletion *>(d->completionObject);
}

KCompletionBase::KeyBindingMap KCompletionBase::getKeyBindings() const
{
    return d->delegate ? d->delegate->getKeyBindings() : d->keyBindingMap;
}

void KCompletionBase::setup(bool autodel, bool hsig, bool esig)
{
    if (d->delegate) {
        d->delegate->setup(autodel, hsig, esig);
        return;
    }

    d->autoDeletesCompletionObject = autodel;
    d->handlesCompletionSignals = hsig;
    d->emitsRotationSignals = esig;
}

void KCompletionBase::virtual_hook(int, void *)
{
    /*BASE::virtual_hook( id, data );*/
}

