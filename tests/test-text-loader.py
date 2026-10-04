#!/usr/bin/env python3
"""Independent TXT line/scroll and installed WBT subset comparison; no execution."""
import argparse
import hashlib
import json
from pathlib import Path
import re
import subprocess
import tempfile


def sha(data):
    return hashlib.sha256(data).hexdigest()


def reference(data,mode):
    lines=[]
    offset=0
    for match in re.finditer(rb'\r\n|\r|\n',data):
        value=data[offset:match.start()]
        lines.append(dict(offset=offset,length=len(value),ending={b'\r\n':'crlf',b'\r':'cr',b'\n':'lf'}[match.group()],text_hex=value.hex()))
        offset=match.end()
    if offset<len(data):
        lines.append(dict(offset=offset,length=len(data)-offset,ending='none',text_hex=data[offset:].hex()))
    result=dict(source_bytes=len(data),source_hex=data.hex(),lines=lines,mode=mode)
    if mode=='scrolls':
        entries=[]
        for match in re.finditer(rb'\[Scroll([0-9]+)\][ \t]*(?:\r\n|\r|\n)[ \t]*~HS(.*?)~HE[ \t]*(?:\r\n|\r|\n)[ \t]*~BS(.*?)~BE',data,re.DOTALL):
            entries.append(dict(id=int(match[1]),heading_hex=match[2].hex(),body_hex=match[3].hex()))
        result['entries']=entries
    if mode=='wbt':
        statements=[]
        operations={'dirchange':'dir_change','filedelete':'file_delete','runwait':'run_wait','filecopy':'file_copy'}
        for line in lines:
            text=bytes.fromhex(line['text_hex']).strip(b' \t')
            if not text or text.startswith(b';'):
                continue
            arguments=[]
            if text.startswith(b':'):
                operation='label';arguments=[dict(kind='identifier',text_hex=text[1:].strip().hex(),boolean=False)]
            elif text.lower().startswith(b'goto '):
                operation='goto';arguments=[dict(kind='identifier',text_hex=text[5:].strip().hex(),boolean=False)]
            else:
                match=re.fullmatch(rb'([A-Za-z]+)\s*\((.*)\)',text)
                assert match
                operation=operations[match[1].decode().lower()]
                for item in re.finditer(rb'"([^"]*)"|@(TRUE|FALSE)',match[2],re.IGNORECASE):
                    arguments.append(dict(kind='string' if item[1] is not None else 'boolean',
                                          text_hex=item[1].hex() if item[1] is not None else '',
                                          boolean=item[2] is not None and item[2].upper()==b'TRUE'))
            statements.append(dict(operation=operation,offset=line['offset'],arguments=arguments))
        result['statements']=statements
    return result


def compare(inspector,root,path,mode):
    data=(root/path).read_bytes()
    result=json.loads(subprocess.check_output([inspector,str(root),path.lower(),mode],text=True))
    assert result==reference(data,mode),(path,mode)
    endings={'none':b'','lf':b'\n','crlf':b'\r\n','cr':b'\r'}
    assert b''.join(bytes.fromhex(line['text_hex'])+endings[line['ending']] for line in result['lines'])==data
    return dict(path=path,mode=mode,source_bytes=len(data),source_sha256=sha(data),lines=len(result['lines']),
                entries=len(result.get('entries',[])),statements=len(result.get('statements',[])),
                decoded_sha256=sha(json.dumps(result,sort_keys=True,separators=(',',':')).encode()))


def run(args):
    with tempfile.TemporaryDirectory() as temp:
        root=Path(temp)
        cases=[('text',b''),('text',b'\r\nA\nB\rC\xff'),('text',b'\xef\xbb\xbfraw\t bytes \r\n'),
               ('scrolls',b';title\r\n[Scroll1]\r\n~HS ~HE\r\n~BS two\nlines~BE\r\n'),
               ('wbt',b';commands are parsed only\r\nGoto START\r\n:start\r\nDirChange("..")\r\nFileDelete("x")\r\nRunWait("a,b\\c", "/win/train")\r\nFileCopy("x", "y", @TRUE)\r\n')]
        for mode,data in cases:
            (root/'Case.TXT').write_bytes(data)
            compare(args.inspector,root,'Case.TXT',mode)
        for mode,data in [('text',b'A\0B'),('scrolls',b'[Scroll1]\n~HS bad'),('wbt',b'Goto nowhere'),('wbt',b':a\n:A'),('wbt',b'FileDelete("unterminated)')]:
            (root/'bad.txt').write_bytes(data)
            assert subprocess.run([args.inspector,str(root),'bad.txt',mode],capture_output=True).returncode==2
    records=[]
    if args.installation:
        root=args.installation.resolve()
        for path in sorted(root.rglob('*')):
            if not path.is_file() or path.suffix.lower() not in ('.txt','.wbt'):
                continue
            relative=path.relative_to(root).as_posix()
            mode='wbt' if path.suffix.lower()=='.wbt' else ('scrolls' if relative.lower()=='text/scrolls.txt' else 'text')
            records.append(compare(args.inspector,root,relative,mode))
        assert records
        for record in records:
            assert sha((root/record['path']).read_bytes())==record['source_sha256']
    report=dict(files=len(records),source_bytes=sum(r['source_bytes'] for r in records),
                lines=sum(r['lines'] for r in records),scroll_entries=sum(r['entries'] for r in records),
                wbt_statements=sum(r['statements'] for r in records),records=records)
    report['comparison_sha256']=sha(json.dumps(records,sort_keys=True,separators=(',',':')).encode())
    if args.report:
        args.report.parent.mkdir(parents=True,exist_ok=True);args.report.write_text(json.dumps(report,indent=2)+'\n')
    print(f"TXT/WBT comparison passed: 5 fixtures, {report['files']} installed files, {report['scroll_entries']} scrolls, {report['wbt_statements']} WBT statements")


def main():
    parser=argparse.ArgumentParser();parser.add_argument('inspector');parser.add_argument('--installation',type=Path);parser.add_argument('--report',type=Path)
    args=parser.parse_args();manifest=Path(__file__).resolve().parents[1]/'tools/original-manifest.sh'
    if args.installation:subprocess.run([str(manifest),'verify'],check=True)
    try:run(args)
    finally:
        if args.installation:subprocess.run([str(manifest),'verify'],check=True)


if __name__=='__main__':main()
