"""Prepare only a fresh owned palette counter with the existing probe's exact schema."""
import ast,hashlib,os,sqlite3
from contextlib import closing
from pathlib import Path
from rendering_pilot import require
SOURCE='tools/run-palette-observation-probe.py'
SCHEMA='CREATE TABLE IF NOT EXISTS attempts(id INTEGER PRIMARY KEY,report TEXT UNIQUE,kind TEXT,started_ns INTEGER,plan_sha256 TEXT)'

def source_schema(root):
 raw=(Path(root)/SOURCE).read_bytes();tree=ast.parse(raw);matches=[n.value for n in ast.walk(tree)if isinstance(n,ast.Constant)and isinstance(n.value,str)and n.value.startswith('CREATE TABLE IF NOT EXISTS attempts(')]
 require(matches==[SCHEMA],'Palette source schema changed or ambiguous');return SCHEMA,hashlib.sha256(raw).hexdigest()

def namespace(root,relative):
 root=Path(root).resolve();p=Path(relative);require(not p.is_absolute()and '..'not in p.parts and len(p.parts)==5 and p.parts[:4]==('working','broader-player','bp34','world-palette')and p.name=='attempts.sqlite3','Unknown palette schema namespace');target=root/p
 require(not any(v.is_symlink()for v in[target,*target.parents]if v!=root and v.is_relative_to(root)),'Aliased palette schema namespace');return target

def initialize(root,relative,sql):
 require(sql==SCHEMA,'Unknown palette schema SQL');target=namespace(root,relative)
 require(all(not Path(str(target)+s).exists()and not Path(str(target)+s).is_symlink()for s in['','-wal','-shm','-journal']),'Never reuse,truncate or repair an existing counter');target.parent.mkdir(parents=True,exist_ok=True);fd=os.open(target,os.O_WRONLY|os.O_CREAT|os.O_EXCL|os.O_NOFOLLOW,0o600);os.close(fd)
 with closing(sqlite3.connect(target))as db:db.execute(sql);db.commit()
 return verify(root,relative)

def verify(root,relative):
 target=namespace(root,relative);require(target.is_file(),'Missing prepared palette counter')
 with closing(sqlite3.connect('file:'+str(target)+'?mode=rw',uri=True))as db:
  actual=db.execute("select type,name,tbl_name,sql from sqlite_schema order by type,name").fetchall()
  with closing(sqlite3.connect(':memory:'))as model:
   model.execute(SCHEMA);expected=model.execute("select type,name,tbl_name,sql from sqlite_schema order by type,name").fetchall()
  require(actual==expected and db.execute('pragma integrity_check').fetchone()==('ok',),'Wrong palette counter schema')
  require(db.execute('select count(*)from attempts').fetchone()==(0,),'Prepared counter already contains an attempt')
  db.execute('BEGIN IMMEDIATE');require(db.execute('SELECT COUNT(*) FROM attempts').fetchone()[0]<9,'Unchanged palette cap refuses');db.execute('INSERT INTO attempts(report,kind,started_ns,plan_sha256)VALUES(?,?,?,?)',('schema-control-only','control',0,'control'))
  try:db.execute('INSERT INTO attempts(report,kind,started_ns,plan_sha256)VALUES(?,?,?,?)',('schema-control-only','control',0,'control'))
  except sqlite3.IntegrityError:pass
  else:raise ValueError('Palette report uniqueness constraint missing')
  db.rollback();require(db.execute('select count(*)from attempts').fetchone()==(0,),'Synthetic control persisted an attempt');columns=db.execute('pragma table_info(attempts)').fetchall()
 return dict(path=str(target),schema_sql=SCHEMA,schema_rows=actual,columns=columns,empty=True,actual_collector_count_and_insert_verified=True,duplicate_report_refused=True,synthetic_control_rolled_back=True,actual_game_attempts_inserted=0,source_database_held=True)
