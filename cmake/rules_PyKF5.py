# SPDX-FileCopyrightText: 2016 Stephen Kelly <steveire@gmail.com>
# SPDX-License-Identifier: BSD-3-Clause

import os, sys

import rules_engine
sys.path.append(os.path.dirname(os.path.dirname(rules_engine.__file__)))
import Qt5Ruleset

def _container_discard_base(container, sip, matcher):
    sip["base_specifiers"] = ""

def local_container_rules():
    return [
        [".*", "KCompletionMatches", ".*", ".*", ".*", _container_discard_base],
    ]

def local_function_rules():
    return [
        ["KCompletionBase", "keyBindingMap", ".*", ".*", ".*", rules_engine.function_discard],
        ["KCompletionBase", "getKeyBindings", ".*", ".*", ".*", rules_engine.function_discard],
        ["KCompletionBase", "setKeyBindingMap", ".*", ".*", ".*", rules_engine.function_discard],
        ["KCompletionMatches", "KCompletionMatches", ".*", ".*", ".*KCompletionMatchesWrapper.*", rules_engine.function_discard],
    ]

class RuleSet(Qt5Ruleset.RuleSet):
    def __init__(self):
        Qt5Ruleset.RuleSet.__init__(self)
        self._fn_db = rules_engine.FunctionRuleDb(lambda: local_function_rules() + Qt5Ruleset.function_rules())
        self._container_db = rules_engine.ContainerRuleDb(lambda: local_container_rules() + Qt5Ruleset.container_rules())
