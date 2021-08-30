/*
    This file is part of the KDE libraries

    SPDX-FileCopyrightText: 2007 David Faure <faure@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <QClipboard>
#include <QSignalSpy>
#include <QTest>
#include <QToolButton>
#include <kcompletionbox.h>
#include <klineedit.h>

class KLineEdit_UnitTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
#if KCOMPLETION_BUILD_DEPRECATED_SINCE(5, 83)
    void testPassword()
    {
        KLineEdit w;
        w.setPasswordMode(true);
        QTest::keyClick(&w, Qt::Key_1);
        QTest::keyClick(&w, Qt::Key_2);
        QTest::keyClick(&w, Qt::Key_3);
        QCOMPARE(w.text(), QString("123"));
    }
#endif

    void testReturnPressed()
    {
        KLineEdit w;
        w.setText(QStringLiteral("Hello world"));
        QSignalSpy qReturnPressedSpy(&w, &QLineEdit::returnPressed);

#if KCOMPLETION_BUILD_DEPRECATED_SINCE(5, 81)
        QSignalSpy kReturnPressedSpy(&w, qOverload<const QString &>(&KLineEdit::returnPressed));
#endif
        QSignalSpy returnKeyPressedSpy(&w, &KLineEdit::returnKeyPressed);

        QTest::keyClick(&w, Qt::Key_Return);
        QCOMPARE(qReturnPressedSpy.count(), 1);

#if KCOMPLETION_BUILD_DEPRECATED_SINCE(5, 81)
        QCOMPARE(kReturnPressedSpy.count(), 1);
        QCOMPARE(kReturnPressedSpy[0][0].toString(), QString("Hello world"));
#endif
        QCOMPARE(returnKeyPressedSpy.count(), 1);
        QCOMPARE(returnKeyPressedSpy.at(0).at(0).toString(), QStringLiteral("Hello world"));
    }

    void testTextEditedSignals()
    {
        KLineEdit w;
        QVERIFY(!w.isModified());

        // setText emits textChanged and userTextChanged, but not textEdited
        QSignalSpy textChangedSpy(&w, &QLineEdit::textChanged);
        QSignalSpy textEditedSpy(&w, &QLineEdit::textEdited);
#if KCOMPLETION_ENABLE_DEPRECATED_SINCE(4, 5)
        QSignalSpy userTextChangedSpy(&w, &KLineEdit::userTextChanged);
#endif
        w.setText(QStringLiteral("Hello worl"));
#if KCOMPLETION_ENABLE_DEPRECATED_SINCE(4, 5)
        QCOMPARE(userTextChangedSpy.count(), 1);
        QCOMPARE(userTextChangedSpy[0][0].toString(), w.text());
#endif
        QCOMPARE(textChangedSpy.count(), 1);
        QCOMPARE(textChangedSpy[0][0].toString(), w.text());
        QCOMPARE(textEditedSpy.count(), 0);
        QVERIFY(!w.isModified());
#if KCOMPLETION_ENABLE_DEPRECATED_SINCE(4, 5)
        userTextChangedSpy.clear();
#endif
        textChangedSpy.clear();
        textEditedSpy.clear();

        // calling clear should emit textChanged and userTextChanged, but not textEdited
        w.clear();
        QCOMPARE(textChangedSpy.count(), 1);
        QCOMPARE(textEditedSpy.count(), 0);
#if KCOMPLETION_ENABLE_DEPRECATED_SINCE(4, 5)
        QCOMPARE(userTextChangedSpy.count(), 1);
#endif

        // if text box is already empty, calling clear() shouldn't emit
        // any more signals
        w.clear();
        QCOMPARE(textChangedSpy.count(), 1);
        QCOMPARE(textEditedSpy.count(), 0);
#if KCOMPLETION_ENABLE_DEPRECATED_SINCE(4, 5)
        QCOMPARE(userTextChangedSpy.count(), 1);
#endif

        // set the text back for further tests below
        w.setText(QStringLiteral("Hello worl"));
#if KCOMPLETION_ENABLE_DEPRECATED_SINCE(4, 5)
        userTextChangedSpy.clear();
#endif
        textChangedSpy.clear();
        textEditedSpy.clear();

        // typing emits all three signals
        QTest::keyClick(&w, Qt::Key_D);
        QCOMPARE(w.text(), QString::fromLatin1("Hello world"));
#if KCOMPLETION_ENABLE_DEPRECATED_SINCE(4, 5)
        QCOMPARE(userTextChangedSpy.count(), 1);
        QCOMPARE(userTextChangedSpy[0][0].toString(), w.text());
#endif
        QCOMPARE(textChangedSpy.count(), 1);
        QCOMPARE(textChangedSpy[0][0].toString(), w.text());
        QCOMPARE(textEditedSpy.count(), 1);
        QCOMPARE(textEditedSpy[0][0].toString(), w.text());
        QVERIFY(w.isModified());

        w.setText(QStringLiteral("K")); // prepare for next test
#if KCOMPLETION_ENABLE_DEPRECATED_SINCE(4, 5)
        userTextChangedSpy.clear();
#endif
        textChangedSpy.clear();
        textEditedSpy.clear();
        QVERIFY(!w.isModified());

        // the suggestion from auto completion emits textChanged but not userTextChanged nor textEdited
        w.setCompletionMode(KCompletion::CompletionAuto);
        KCompletion completion;
        completion.setSoundsEnabled(false);
        QStringList items;
        items << QStringLiteral("KDE is cool") << QStringLiteral("KDE is really cool");
        completion.setItems(items);
        w.setCompletionObject(&completion);

        w.doCompletion(w.text());
        QCOMPARE(w.text(), items.at(0));

#if KCOMPLETION_ENABLE_DEPRECATED_SINCE(4, 5)
        QCOMPARE(userTextChangedSpy.count(), 0);
#endif
        QCOMPARE(textChangedSpy.count(), 1);
        QCOMPARE(textChangedSpy[0][0].toString(), w.text());
        QCOMPARE(textEditedSpy.count(), 0);
        QVERIFY(!w.isModified());
#if KCOMPLETION_ENABLE_DEPRECATED_SINCE(4, 5)
        userTextChangedSpy.clear();
#endif
        textChangedSpy.clear();
        textEditedSpy.clear();

        // accepting the completion suggestion now emits all three signals too
        QTest::keyClick(&w, Qt::Key_Right);
        QCOMPARE(w.text(), items.at(0));

#if KCOMPLETION_ENABLE_DEPRECATED_SINCE(4, 5)
        QCOMPARE(userTextChangedSpy.count(), 1);
        QCOMPARE(userTextChangedSpy[0][0].toString(), w.text());
#endif
        QCOMPARE(textChangedSpy.count(), 1);
        QCOMPARE(textChangedSpy[0][0].toString(), w.text());
        QCOMPARE(textEditedSpy.count(), 1);
        QCOMPARE(textEditedSpy[0][0].toString(), w.text());
        QVERIFY(w.isModified());
#if KCOMPLETION_ENABLE_DEPRECATED_SINCE(4, 5)
        userTextChangedSpy.clear();
#endif
        textChangedSpy.clear();
        textEditedSpy.clear();

        // Now with popup completion
        w.setCompletionMode(KCompletion::CompletionPopup);
        w.setText(QStringLiteral("KDE"));
        QVERIFY(!w.isModified());
#if KCOMPLETION_ENABLE_DEPRECATED_SINCE(4, 5)
        userTextChangedSpy.clear();
#endif
        textChangedSpy.clear();
        textEditedSpy.clear();
        w.doCompletion(w.text()); // popup appears
        QCOMPARE(w.text(), QString::fromLatin1("KDE"));
#if KCOMPLETION_ENABLE_DEPRECATED_SINCE(4, 5)
        QCOMPARE(textChangedSpy.count() + userTextChangedSpy.count() + textEditedSpy.count(), 0);
#else
        QCOMPARE(textChangedSpy.count() + textEditedSpy.count(), 0);
#endif
        w.completionBox()->down(); // select 1st item
        QCOMPARE(w.text(), items.at(0));
        QVERIFY(w.isModified());
        w.completionBox()->down(); // select 2nd item
        QCOMPARE(w.text(), items.at(1));

        // Selecting an item in the popup completion changes the lineedit text
        // and emits textChanged and userTextChanged, but not textEdited.
#if KCOMPLETION_ENABLE_DEPRECATED_SINCE(4, 5)
        QCOMPARE(userTextChangedSpy.count(), 2);
#endif
        QCOMPARE(textChangedSpy.count(), 2);
        QCOMPARE(textEditedSpy.count(), 0);
#if KCOMPLETION_ENABLE_DEPRECATED_SINCE(4, 5)
        userTextChangedSpy.clear();
#endif
        textChangedSpy.clear();
        textEditedSpy.clear();

        QTest::keyClick(&w, Qt::Key_Enter); // activate
        QVERIFY(!w.completionBox()->isVisible());
        QCOMPARE(w.text(), items.at(1));
        QVERIFY(w.isModified());
        // Nothing else happens, the text was already set in the lineedit
        QCOMPARE(textChangedSpy.count(), 0);
        QCOMPARE(textEditedSpy.count(), 0);
#if KCOMPLETION_ENABLE_DEPRECATED_SINCE(4, 5)
        QCOMPARE(userTextChangedSpy.count(), 0);
#endif

        // Now when using the mouse in the popup completion
        w.setText(QStringLiteral("KDE"));
        w.doCompletion(w.text()); // popup appears
        QCOMPARE(w.text(), QString::fromLatin1("KDE"));
        // Selecting an item in the popup completion changes the lineedit text and emits all 3 signals
        const QRect rect = w.completionBox()->visualRect(w.completionBox()->model()->index(1, 0));

#if KCOMPLETION_BUILD_DEPRECATED_SINCE(5, 81)
        QSignalSpy activatedSpy(w.completionBox(), qOverload<const QString &>(&KCompletionBox::activated));
#endif
        QSignalSpy textActivatedSpy(w.completionBox(), &KCompletionBox::textActivated);

        QTest::mouseClick(w.completionBox()->viewport(), Qt::LeftButton, Qt::NoModifier, rect.center());

#if KCOMPLETION_BUILD_DEPRECATED_SINCE(5, 81)
        QCOMPARE(activatedSpy.count(), 1);
#endif
        QCOMPARE(textActivatedSpy.count(), 1);

        QCOMPARE(w.text(), items.at(1));
        QVERIFY(w.isModified());
    }

    void testCompletionBox()
    {
        KLineEdit w;
        w.setText(QStringLiteral("/"));
        w.setCompletionMode(KCompletion::CompletionPopup);
        KCompletion completion;
        completion.setSoundsEnabled(false);
        w.setCompletionObject(&completion);
        QStringList items;
        items << QStringLiteral("/home/") << QStringLiteral("/hold/") << QStringLiteral("/hole/");
        completion.setItems(items);
        QTest::keyClick(&w, 'h');
        QCOMPARE(w.text(), QString::fromLatin1("/h"));
        QCOMPARE(w.completionBox()->currentRow(), -1);
        QCOMPARE(w.completionBox()->items(), items);
        QTest::keyClick(&w, 'o');
        QCOMPARE(w.text(), QString::fromLatin1("/ho"));
        QCOMPARE(w.completionBox()->currentRow(), -1);
        w.completionBox()->down(); // select 1st item
        QCOMPARE(w.text(), items.at(0));
        w.completionBox()->down(); // select 2nd item
        QCOMPARE(w.text(), items.at(1));
        w.completionBox()->up(); // select 1st item again
        QCOMPARE(w.text(), items.at(0));
        w.completionBox()->up(); // select last item
        QCOMPARE(w.text(), items.at(2));
        w.completionBox()->down(); // select 1st item again
        QCOMPARE(w.text(), items.at(0));

        QStringList newItems;
        newItems << QStringLiteral("/home/kde");
        completion.setItems(newItems);
        QTest::keyClick(&w, 'k');
        QCOMPARE(w.text(), QString("/home/k"));
        // QCOMPARE(w.completionBox()->currentRow(), -1); // #247552
        w.completionBox()->down(); // select the item
        QCOMPARE(w.completionBox()->items(), newItems);
        QCOMPARE(w.text(), newItems.at(0));
    }

    void testPaste()
    {
        const QString origText = QApplication::clipboard()->text();
        const QString pastedText = QStringLiteral("Test paste from klineedit_unittest");
        QApplication::clipboard()->setText(pastedText);
        KLineEdit w;
        w.setText(QStringLiteral("Hello world"));
        w.selectAll();
        QTest::keyClick(&w, Qt::Key_V, Qt::ControlModifier);
        QCOMPARE(w.text(), pastedText);
        QApplication::clipboard()->setText(origText);
    }

    void testClearButtonClicked()
    {
        KLineEdit w;
        w.setText(QStringLiteral("Hello world"));
        w.setClearButtonEnabled(true);
        w.setClearButtonEnabled(false);
        w.setClearButtonEnabled(true);
        QSignalSpy spy(&w, &KLineEdit::clearButtonClicked);
        QToolButton *tb = w.findChild<QToolButton *>();
        QTest::mouseClick(tb, Qt::LeftButton, Qt::NoModifier);
        QCOMPARE(w.text(), QString());
        QCOMPARE(spy.count(), 1);
    }
};

QTEST_MAIN(KLineEdit_UnitTest)

#include "klineedit_unittest.moc"
