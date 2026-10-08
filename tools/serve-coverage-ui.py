#!/usr/bin/env python3
"""Launch the local Magic & Mayhem coverage dashboard; no dependencies required."""
import argparse
from ipaddress import IPv4Address
import webbrowser

from coverage_ui import CoverageServer


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--host', type=IPv4Address, default='127.0.0.1',
                        help='IPv4 bind address (default: 127.0.0.1; use 0.0.0.0 for LAN access)')
    parser.add_argument('--port', type=int, default=8786)
    parser.add_argument('--no-open', action='store_true', help='Print the URL without opening a browser')
    args = parser.parse_args()
    try:
        server = CoverageServer((str(args.host), args.port))
    except (OSError, ValueError, KeyError) as exc:
        parser.exit(1, f'Could not start coverage dashboard: {exc}\n')
    host = str(args.host)
    url = f'http://{host}:{server.server_port}'
    print(f'Magic & Mayhem coverage dashboard: {url}', flush=True)
    if host == '0.0.0.0':
        print(f'For remote access, open http://<server-ip>:{server.server_port} in your browser.', flush=True)
    print('Read-only coverage snapshot. Use Refresh audit after repository changes. Ctrl+C stops the server.', flush=True)
    if not args.no_open:
        webbrowser.open(f'http://127.0.0.1:{server.server_port}' if host == '0.0.0.0' else url)
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        pass
    finally:
        server.server_close()


if __name__ == '__main__':
    main()
