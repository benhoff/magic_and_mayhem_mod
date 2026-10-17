"""Read the versioned per-route optional pixel producer diagnostics after exit."""
import hashlib
import json
import struct

ROUTES = {
    'panel': ['bevel', 'halo', 'wash'],
    'pixel': ['pcx_surface', 'rgb_add', 'pcx_indices', 'pcx_words', 'brighten', 'row_copy', 'fade', 'dib'],
    'jpeg': ['decode'],
    'minimap': ['terrain', 'cell', 'creature', 'camera0', 'camera1', 'camera2', 'camera3'],
    'colour_rect': ['rectangle'],
    'raster': [f'splice{i}' for i in range(23)],
}


def read_counters(game):
    raw = (game/'native-pixel-producers.bin').read_bytes()
    if len(raw) != (4+6*32*3)*4:
        raise ValueError('Incomplete pixel producer counters')
    values = struct.unpack('<'+'I'*(len(raw)//4), raw)
    if values[:4] != (0x50504e4d, 2, len(raw), 6):
        raise ValueError('Unsupported pixel producer counter layout')
    families = {}
    for family, (name, routes) in enumerate(ROUTES.items()):
        rows = []
        for route, label in enumerate(routes):
            installed, native, forwarded = values[4+(family*32+route)*3:7+(family*32+route)*3]
            if installed != 1:
                raise ValueError(f'Pixel producer route was not installed exactly once: {name}/{label}')
            rows.append(dict(route=label, installed=installed, native=native, forwarded=forwarded,
                             exercised=bool(native+forwarded)))
        families[name] = rows
        if any(values[4+(family*32+len(routes))*3:4+(family+1)*32*3]):
            raise ValueError('Unexpected pixel producer counter route: '+name)
    refusal = game/'native-cpu-history-refusal.bin'
    result = dict(schema=2, sha256=hashlib.sha256(raw).hexdigest(), families=families,
                  native=sum(r['native'] for rows in families.values() for r in rows),
                  forwarded=sum(r['forwarded'] for rows in families.values() for r in rows),
                  unexercised=[name+'/'+r['route'] for name, rows in families.items() for r in rows if not r['exercised']],
                  history_refused=refusal.exists(), complete_replacement=False)
    guard=game/'native-pixel-forward-refusal.bin'
    if guard.exists():
        from pixel_forwarding_guard import decode_refusal
        result['forward_guard_refusal']=decode_refusal(guard.read_bytes())
        result['forward_guard_refusal_sha256']=hashlib.sha256(guard.read_bytes()).hexdigest()
    if refusal.exists():
        result['history_refusal_sha256'] = hashlib.sha256(refusal.read_bytes()).hexdigest()
    return result


def validate_builds(experiment, root, declared):
    metadata=json.loads((experiment/'manifest.json').read_text())
    dlls=['MnmPanelWriters.dll','MnmPixelWriters.dll','MnmJpegPixels.dll','MnmMinimapWriters.dll',
          'MnmColourRect.dll','MnmRasterWriters.dll']
    results={}
    for name,dll in zip(ROUTES,dlls):
        build=json.loads((experiment/(name+'-bridge-build.json')).read_text())
        digest=hashlib.sha256((experiment/'game'/dll).read_bytes()).hexdigest()
        if metadata.get('native_'+name+'_takeover') is not True or metadata.get(name+'_dll_sha256')!=digest or build['sha256']!=digest:
            raise ValueError('Combined producer staging changed: '+name)
        for path,expected in build['sources'].items():
            if declared.get(path)!=expected or hashlib.sha256((root/path).read_bytes()).hexdigest()!=expected:
                raise ValueError('Undeclared or changed producer input: '+path)
        results[name]=dict(dll_sha256=digest,sources=build['sources'])
    return results
