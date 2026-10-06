#!/usr/bin/env python3
"""Launch the local Magic & Mayhem coverage dashboard; no dependencies required."""
import argparse
import webbrowser

from coverage_ui import CoverageServer


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--port', type=int, default=8786)
    parser.add_argument('--no-open', action='store_true', help='Print the URL without opening a browser')
    args = parser.parse_args()
    try:
        server = CoverageServer(('127.0.0.1', args.port))
    except (OSError, ValueError, KeyError) as exc:
        parser.exit(1, f'Could not start coverage dashboard: {exc}\n')
    url = f'http://127.0.0.1:{server.server_port}'
    print(f'Magic & Mayhem coverage dashboard: {url}', flush=True)
    print('Read-only local snapshot. Use Refresh audit after repository changes. Ctrl+C stops the server.', flush=True)
    if not args.no_open:
        webbrowser.open(url)
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        pass
    finally:
        server.server_close()


if __name__ == '__main__':
    main()
