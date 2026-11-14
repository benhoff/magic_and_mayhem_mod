"""Preserve the full final current predicate and explicit terminal pilot successor."""
from broader_player_current_interaction_final_matrix import bind_current
from broader_player_terminal_matrix import P,BINARY,specs,requests,replacements,replacement
from broader_player_terminal_matrix import current as complete_current
current=bind_current(complete_current)
