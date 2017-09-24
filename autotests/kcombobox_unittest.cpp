/* This file is part of the KDE libraries

    Copyright (c) 2007 David Faure <faure@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <QtTest/QtTest>
#include <khistorycombobox.h>
#include <klineedit.h>

class KTestComboBox : public KComboBox
{
public:
    KTestComboBox(bool rw, QWidget *parent = nullptr) : KComboBox(rw, parent) {}
    KCompletionBase *delegate() const
    {
        return KCompletionBase::delegate();
    }
};

class KComboBox_UnitTest : public QObject
{
    Q_OBJECT

private:
    void testComboReturnPressed(bool ctorArg)
    {
        KComboBox w(ctorArg /*initial value for editable*/);
        w.setEditable(true);
        w.setCompletionMode(KCompletion::CompletionPopup);
        w.addItem(QStringLiteral("Hello world"));
        QVERIFY(w.lineEdit());
        QVERIFY(qobject_cast<KLineEdit *>(w.lineEdit()));
        // KLineEdit signals
        QSignalSpy qReturnPressedSpy(w.lineEdit(), SIGNAL(returnPressed()));
        QSignalSpy kReturnPressedSpy(w.lineEdit(), SIGNAL(returnPressed(QString)));
        // KComboBox signals
        QSignalSpy comboReturnPressedSpy(&w, SIGNAL(returnPressed()));
        QSignalSpy comboReturnPressedStringSpy(&w, SIGNAL(returnPressed(QString)));
        QSignalSpy comboActivatedSpy(&w, SIGNAL(activated(QString)));
        QTest::keyClick(&w, Qt::Key_Return);
        QCOMPARE(qReturnPressedSpy.count(), 1);
        QCOMPARE(kReturnPressedSpy.count(), 1);
        QCOMPARE(kReturnPressedSpy[0][0].toString(), QString("Hello world"));
        QCOMPARE(comboReturnPressedSpy.count(), 1);
        QCOMPARE(comboReturnPressedStringSpy.count(), 1);
        QCOMPARE(comboReturnPressedStringSpy[0][0].toString(), QString("Hello world"));
        QCOMPARE(comboActivatedSpy.count(), 1);
        QCOMPARE(comboActivatedSpy[0][0].toString(), QString("Hello world"));
    }

private Q_SLOTS:
    void testComboReturnPressed()
    {
        testComboReturnPressed(false);
        testComboReturnPressed(true);
    }

    void testHistoryComboReturnPressed()
    {
        KHistoryComboBox w;
        QVERIFY(qobject_cast<KLineEdit *>(w.lineEdit()));
        connect(&w, SIGNAL(activated(QString)),
                &w, SLOT(addToHistory(QString)));
        QSignalSpy comboReturnPressedSpy(&w, SIGNAL(returnPressed()));
        QSignalSpy comboReturnPressedStringSpy(&w, SIGNAL(returnPressed(QString)));
        QSignalSpy comboActivatedSpy(&w, SIGNAL(activated(QString)));
        QTest::keyClicks(&w, QStringLiteral("Hello world"));
        QTest::keyClick(&w, Qt::Key_Return);
        qApp->processEvents(); // QueuedConnection in KHistoryComboBox
        QCOMPARE(comboReturnPressedSpy.count(), 1);
        QCOMPARE(comboReturnPressedStringSpy.count(), 1);
        QCOMPARE(comboReturnPressedStringSpy[0][0].toString(), QString("Hello world"));
        QCOMPARE(comboActivatedSpy.count(), 1);
        QCOMPARE(comboActivatedSpy[0][0].toString(), QString("Hello world"));
    }

    void testHistoryComboKeyUp()
    {
        KHistoryComboBox w;
        QStringList items;
        items << QStringLiteral("One") << QStringLiteral("Two") << QStringLiteral("Three") << QStringLiteral("Four");
        w.addItems(items);
        QSignalSpy currentIndexChangedSpy(&w, SIGNAL(currentIndexChanged(int)));
        w.completionObject()->setItems(items);
        QCOMPARE(w.currentIndex(), 0);
        QTest::keyClick(&w, Qt::Key_Up);
        QCOMPARE(w.currentIndex(), 1);
        QCOMPARE(currentIndexChangedSpy.count(), 1);
        QCOMPARE(currentIndexChangedSpy[0][0].toInt(), 1);
    }

    void testHistoryComboInsertItems()
    {
        KHistoryComboBox combo;
        // uic generates code like this, let's make sure it compiles
        combo.insertItems(0, QStringList() << QStringLiteral("foo"));
    }

    void testHistoryComboReset()
    {
        //It only tests that it doesn't crash
        //TODO: Finish
        KHistoryComboBox combo;
        QStringList items;
        items << QStringLiteral("One") << QStringLiteral("Two");
        combo.addItems(items);
        combo.reset();
    }

    void testDeleteLineEdit()
    {
        // Test for KCombo's KLineEdit destruction
        KTestComboBox *testCombo = new KTestComboBox(true, nullptr);   // rw, with KLineEdit
        testCombo->setEditable(false); // destroys our KLineEdit, with deleteLater
        qApp->sendPostedEvents(nullptr, QEvent::DeferredDelete);
        QVERIFY(testCombo->KTestComboBox::delegate() == nullptr);
        delete testCombo; // not needed anymore
    }

    void testLineEditCompletion()
    {
        // Test for KCombo's KLineEdit inheriting the completion object of the parent
        KTestComboBox testCombo(false, nullptr);
        QVERIFY(!testCombo.lineEdit());
        auto completion = testCombo.completionObject();
        QVERIFY(completion);
        testCombo.setEditable(true);
        auto lineEdit = qobject_cast<KLineEdit*>(testCombo.lineEdit());
        QVERIFY(lineEdit);
        QVERIFY(lineEdit->compObj());
        QCOMPARE(lineEdit->compObj(), completion);
        QCOMPARE(testCombo.completionObject(), completion);
    }

    void testSelectionResetOnReturn()
    {
        // void QComboBoxPrivate::_q_returnPressed() calls lineEdit->deselect()
        KHistoryComboBox *testCombo = new KHistoryComboBox(true, nullptr);
        QCOMPARE(testCombo->insertPolicy(), QComboBox::NoInsert); // not the Qt default; KHistoryComboBox changes that
        QTest::keyClicks(testCombo, QStringLiteral("Hello world"));
        testCombo->lineEdit()->setSelection(5, 3);
        QVERIFY(testCombo->lineEdit()->hasSelectedText());
        QTest::keyClick(testCombo, Qt::Key_Return);
        // Changed in Qt5: it only does that if insertionPolicy isn't NoInsert.
        // Should we add a lineEdit()->deselect() in KHistoryComboBox? Why does this matter?
        QEXPECT_FAIL("", "Qt5: QComboBox doesn't deselect text anymore on returnPressed", Continue);
        QVERIFY(!testCombo->lineEdit()->hasSelectedText());
    }
};

QTEST_MAIN(KComboBox_UnitTest)

#include "kcombobox_unittest.moc"
