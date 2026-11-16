"""The launcher's notice for skipped work (android/README.md, In-game notices; SkipNotice.java with android/app/src/jvmtest's SkipNoticeJvmTest): the host's status
"<percent> <causes>", the notice text with the driver in use, and the host side's wiring (switch documented, status exported to the launcher). The JVM part is skipped
without a JDK."""
import shutil
import subprocess
import tempfile
import unittest
from pathlib import Path

REPO = Path(__file__).resolve().parents[3]
SRC = REPO / 'android/app/src/main/java/dev/nxnative/android/SkipNotice.java'
TEST = REPO / 'android/app/src/jvmtest/java/dev/nxnative/android/SkipNoticeJvmTest.java'


class Wiring(unittest.TestCase):
    def test_switch_and_status_are_wired(self):
        self.assertIn('DM_VK_SKIP_WARN\t', (REPO / 'docs/env_switches.tsv').read_text(encoding='utf-8'))
        self.assertIn('nxhost_skip_status', (REPO / 'dm/gfx/vk_android/vka_exec.c').read_text(encoding='utf-8'))
        self.assertIn('Native_skipStatus', (REPO / 'android/app/src/main/cpp/nxlaunch.c').read_text(encoding='utf-8'))
        self.assertIn('native String skipStatus', (REPO / 'android/app/src/main/java/dev/nxnative/android/Native.java').read_text(encoding='utf-8'))

    def test_the_launcher_reads_the_hosts_cause_names(self):
        # SkipNotice matches the host's cause names (vka_skipwarn.h vks_name) by their text: a renamed cause would silently lose its notice
        h, java = (REPO / 'dm/gfx/vk_android/vka_skipwarn.h').read_text(encoding='utf-8'), SRC.read_text(encoding='utf-8')
        for name in ('mesh draws without VK_EXT_mesh_shader', 'shaders missing from the pack'):
            self.assertIn(f'"{name}"', h); self.assertIn(f'"{name}"', java)


@unittest.skipUnless(shutil.which('javac') and shutil.which('java'), 'no JDK')
class Notice(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.tmp = Path(tempfile.mkdtemp(prefix='skipnotice_'))
        subprocess.run(['javac', '-Xlint:all', '-Werror', '-encoding', 'UTF-8', '-d', str(cls.tmp / 'classes'), str(SRC), str(TEST)], check=True, capture_output=True, text=True)

    @classmethod
    def tearDownClass(cls): shutil.rmtree(cls.tmp, ignore_errors=True)

    def java(self, *a):
        r = subprocess.run(['java', '-Dstdout.encoding=UTF-8', '-cp', str(self.tmp / 'classes'), 'dev.nxnative.android.SkipNoticeJvmTest', *map(str, a)],
                           capture_output=True, text=True, encoding='utf-8')
        self.assertEqual(r.returncode, 0, r.stderr)
        return r.stdout.rstrip('\n').split(' ', 1)[1] if ' ' in r.stdout else ''

    def test_status(self):
        s = '12 pipelines not made 480, mesh draws without VK_EXT_mesh_shader 20'
        self.assertEqual(self.java('percent', s), '12')
        self.assertEqual(self.java('causes', s), 'pipelines not made 480, mesh draws without VK_EXT_mesh_shader 20')
        self.assertEqual(self.java('percent', '7'), '7')
        self.assertEqual(self.java('causes', '7'), '')

    def test_no_mesh_shading(self):
        r = self.java('nomesh', '30 mesh draws without VK_EXT_mesh_shader 900, shaders refused 49', 30)
        self.assertTrue(r.startswith('true This GPU has no mesh shading, so some models are missing'), r)
        self.assertTrue(self.java('nomesh', '9 pipelines not made 50, mesh draws without VK_EXT_mesh_shader 5', 9).startswith('false'))

    def test_shaders_missing_from_the_pack(self):
        # BananaNative #356: a pack without the add-on's shaders skipped 99 % of draws, and the notice sent the player after a driver; the pack's cause names no driver
        r = self.java('missing', '99 shaders missing from the pack 488394, shaders refused 12', 99)
        self.assertEqual(r, 'true Part of the picture is not drawn: this game\'s pack lacks shaders the game uses (99 % of draws skipped). Another GPU driver will not '
                            'change this. Prepare the game again from a complete dump: its page › Prepare again.')
        self.assertTrue(self.java('missing', '20 shaders refused 400, shaders missing from the pack 10', 20).startswith('false'))   # the largest cause decides
        self.assertTrue(self.java('nomesh', '99 shaders missing from the pack 488394', 99).startswith('false'))

    def test_no_warning(self):
        for s in ('-', '', '0 x', 'abc', '101 x', '-3 x'):
            self.assertEqual(self.java('percent', s), '0', s)

    def test_text(self):
        t = self.java('text', 18, 'Mesa Turnip (26.1)')
        self.assertEqual(t, "Part of the picture is not drawn on this GPU driver (18 % of draws skipped). Another driver may draw it: Settings › GPU driver, "
                            "or this game's Game settings. Driver now: Mesa Turnip (26.1).")
        self.assertTrue(self.java('text', 6, '-').endswith('Driver now: System.'))


if __name__ == '__main__':
    unittest.main()
