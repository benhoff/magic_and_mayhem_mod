#!/usr/bin/env python3
"""Validate and inventory the complete DRAW-02 prospective acceptance matrix."""
import argparse
import json
from pathlib import Path
from drawing_acceptance import PLAN, inventory


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--plan', default=PLAN)
    parser.add_argument('--report', type=Path, required=True)
    args = parser.parse_args()
    result = inventory(path=args.plan)
    args.report.parent.mkdir(parents=True, exist_ok=True)
    with args.report.open('x') as output:
        json.dump(result, output, indent=2)
        output.write('\n')
    print(json.dumps({k: result[k] for k in ('journey_count', 'journey_requirement_count', 'operation_count', 'execution_performed', 'draw02_complete')}))


if __name__ == '__main__':
    main()
