"""Validate complete producer metadata before COPY-order accounting."""
import importlib.util
from pathlib import Path


def producer_records(raw):
    path=Path(__file__).with_name('inspect-canvas-producers.py')
    spec=importlib.util.spec_from_file_location('copy_metadata_producers',path)
    parser=importlib.util.module_from_spec(spec)
    spec.loader.exec_module(parser)
    return parser.decode(raw,complete=True)[1]
