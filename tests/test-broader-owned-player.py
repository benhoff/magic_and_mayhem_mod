import sys,unittest
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from broader_owned_player_binding import player_command
class OwnedPlayerBinding(unittest.TestCase):
 def test_exact_qt_arguments_and_shell_defaults(self):
  root=Path('/owned');old=[str(root/'tools/run-qt-shell.sh'),'--repo',str(root),'--native-only','--live-menu-test','flow.json'];actual,env=player_command(root,'/pinned-player',old,{},False)
  self.assertEqual(actual,['/pinned-player','--repo','/owned',*old[1:]]);self.assertEqual(env['MNM_GLYPH_BACKEND'],'takeover');self.assertEqual(env['MNM_VERIFY_ASSETS'],'0');self.assertNotIn('LIBGL_ALWAYS_SOFTWARE',env)
 def test_software_flag_and_provider(self):
  actual,env=player_command('/owned','/pinned-player',['/owned/tools/run-qt-shell.sh','--native-only'],dict(GALLIUM_DRIVER='zink'),True)
  self.assertEqual(actual[-1],'--software-rendering');self.assertEqual(env['LIBGL_ALWAYS_SOFTWARE'],'1');self.assertNotIn('GALLIUM_DRIVER',env)
 def test_explicit_shell_settings_preserved(self):
  _,env=player_command('/owned','/pinned-player',['/owned/tools/run-qt-shell.sh'],dict(MNM_GLYPH_BACKEND='',MNM_CACHE_ADAPTERS='0'),True)
  self.assertEqual(env['MNM_GLYPH_BACKEND'],'');self.assertEqual(env['MNM_CACHE_ADAPTERS'],'0')
 def test_other_process_cannot_bind(self):
  with self.assertRaises(ValueError):player_command('/owned','/pinned',['/other/tool'],{},True)
 def test_existing_or_untyped_profile_refuses(self):
  with self.assertRaises(ValueError):player_command('/owned','/pinned',['/owned/tools/run-qt-shell.sh','--software-rendering'],{},True)
  with self.assertRaises(ValueError):player_command('/owned','/pinned',['/owned/tools/run-qt-shell.sh'],{},'true')
if __name__=='__main__':unittest.main()
