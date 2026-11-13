"""Keep the complete final predicate and select the reviewed interaction successor."""
from types import FunctionType
from broader_player_discovery_final_matrix import P, BINARY, specs, requests, replacements, replacement
from broader_player_discovery_final_matrix import current as original_final_current
from broader_player_current_interaction_matrix import current as complete_current


def bind_current(complete):
    namespace = dict(original_final_current.__globals__, complete_current=complete)
    return FunctionType(original_final_current.__code__, namespace, original_final_current.__name__,
                        original_final_current.__defaults__, original_final_current.__closure__)


current = bind_current(complete_current)
