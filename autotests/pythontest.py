#!/usr/bin/env python
#-*- coding: utf-8 -*-

import sys

sys.path.append(sys.argv[1])

from PyQt5 import QtWidgets

from PyKF5 import KCompletion

def main():
    app = QtWidgets.QApplication(sys.argv)

    kc = KCompletion.KCompletion()
    kc.insertItems(["Monday", "Tuesday", "Wednesday",
        "Thursday", "Friday", "Saturday", "Sunday"])

    kl = KCompletion.KLineEdit()
    kl.setCompletionObject(kc)
    kl.setCompletionMode(KCompletion.KCompletion.CompletionAuto)

    kl.makeCompletion("M")
    assert(kl.text() == "Monday")

if __name__ == '__main__':
    sys.exit(main())
