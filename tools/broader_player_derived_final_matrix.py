"""Keep the exact full Native/current/no-waiver predicate with derived admission."""
from broader_player_current_interaction_final_matrix import bind_current
from broader_player_derived_matrix import P,BINARY,specs,requests,replacements,replacement
from broader_player_derived_matrix import current as complete_current
current=bind_current(complete_current)
