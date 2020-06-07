/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2007 David Faure <faure@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <QTest>
#include <QSignalSpy>
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
        auto lineEdit = w.lineEdit();
        // set editable again, don't recreate the line edit
        w.setEditable(true);
        QCOMPARE(w.lineEdit(), lineEdit);
        // KLineEdit signals
        QSignalSpy qReturnPressedSpy(w.lineEdit(), SIGNAL(returnPressed()));
        QSignalSpy kReturnPressedSpy(w.lineEdit(), SIGNAL(returnPressed(QString)));
        // KComboBox signals
        QSignalSpy comboReturnPressedSpy(&w, SIGNAL(returnPressed()));
        QSignalSpy comboReturnPressedStringSpy(&w, SIGNAL(returnPressed(QString)));
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
        QSignalSpy comboActivatedSpy(&w, SIGNAL(activated(QString)));
#else
        QSignalSpy comboActivatedSpy(&w, &QComboBox::textActivated);
#endif
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
        QSignalSpy comboReturnPressedSpy(&w, SIGNAL(returnPressed()));
        QSignalSpy comboReturnPressedStringSpy(&w, SIGNAL(returnPressed(QString)));
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
        connect(&w, SIGNAL(activated(QString)),
                &w, SLOT(addToHistory(QString)));
        QSignalSpy comboActivatedSpy(&w, SIGNAL(activated(QString)));
#else
        connect(&w, &KHistoryComboBox::textActivated,
                &w, &KHistoryComboBox::addToHistory);
        QSignalSpy comboActivatedSpy(&w, &QComboBox::textActivated);
#endif
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
        QFETCH(bool, editable);
        QPointer<KCompletion> completion;
        QPointer<KLineEdit> lineEdit, lineEdit2;

        {
            // Test for KCombo's KLineEdit inheriting the completion object of the parent
            KTestComboBox testCombo(editable, nullptr);

            // we only have a line edit when we are editable already
            QCOMPARE(static_cast<bool>(testCombo.lineEdit()), editable);

            // we can always get a completion object
            // NOTE: for an editable combo, this refers to the completion object of
            // the internal line edit
            // NOTE: for a not-yet-editable combo, this refers to the completion object
            // that belongs to the combo directly
            completion = testCombo.completionObject();
            QVERIFY(completion);

            // make editable
            testCombo.setEditable(true);
            QVERIFY(completion); // completion is still alive

            // verify that the completion object was set on the line edit
            lineEdit = qobject_cast<KLineEdit*>(testCombo.lineEdit());
            QVERIFY(lineEdit);
            QVERIFY(lineEdit->compObj());
            QCOMPARE(lineEdit->compObj(), completion.data());
            QCOMPARE(testCombo.completionObject(), completion.data());

            // don't lose the completion and don't crash when we set a new line edit
            // NOTE: only reuse the completion when it belongs to the combo box
            lineEdit2 = new KLineEdit(&testCombo);
            QVERIFY(!lineEdit2->compObj());
            testCombo.setLineEdit(lineEdit2);
            QVERIFY(!lineEdit); // first line edit got deleted now
            if (editable) {
                QVERIFY(!completion); // got deleted with the line edit
                // but we get a new one from the second line edit
                completion = testCombo.completionObject();
            }
                QVERIFY(completion);
            QCOMPARE(lineEdit2->compObj(), completion.data());
            QCOMPARE(testCombo.completionObject(), completion.data());
        }

        // ensure nothing gets leaked
        QVERIFY(!completion);
        QVERIFY(!lineEdit2);
    }

    void testLineEditCompletion_data()
    {
        QTest::addColumn<bool>("editable");
        QTest::newRow("deferred-editable") << false;
        QTest::newRow("editable") << true;
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
