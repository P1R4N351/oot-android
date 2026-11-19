"""Frame generation's choices the screen can show (PanelRates.java with android/app/src/jvmtest's PanelRatesJvmTest; Lsfg's rows): the fastest refresh is the main screen's
fastest display mode held to Android's peak refresh setting (the AYN Thor: 60 and 120 Hz modes, a 60 Hz setting), the multipliers offered are those whose output at 30 fps
fits it, the target rates those at most it that a mode shows evenly, and a saved choice the screen cannot show is used as the nearest offered one. Skipped without a JDK."""
import shutil
import subprocess
import tempfile
import unittest
from pathlib import Path

REPO = Path(__file__).resolve().parents[3]
J = REPO / 'android/app/src'
SRC = J / 'main/java/dev/nxnative/android/PanelRates.java'
TEST = J / 'jvmtest/java/dev/nxnative/android/PanelRatesJvmTest.java'
LSFG = J / 'main/java/dev/nxnative/android/Lsfg.java'


class Wiring(unittest.TestCase):
    def test_lsfg_offers_and_uses_what_the_screen_shows(self):
        t = LSFG.read_text(encoding='utf-8')
        self.assertIn('PanelRates.clamp(prefs(c).getInt("lsfg_mult", 2), multipliers(c))', t)   # the env and the in-game push take the held value
        self.assertIn('PanelRates.clamp(prefs(c).getInt("lsfg_target", 0), targets(c))', t)
        self.assertIn('choice(c, tv, target(c), tl', t)
        self.assertIn('choice(c, mv, multiplier(c), ml', t)
        self.assertIn('"peak_refresh_rate"', t)
        self.assertNotIn('TARGETS = {0, 60, 90, 120}', t)


@unittest.skipUnless(shutil.which('javac') and shutil.which('java'), 'no JDK')
class Rates(unittest.TestCase):
    def test_jvm(self):
        with tempfile.TemporaryDirectory(prefix='panelrates_') as d:
            subprocess.run(['javac', '-d', d, str(SRC), str(TEST)], check=True)
            r = subprocess.run(['java', '-cp', d, 'dev.nxnative.android.PanelRatesJvmTest'], capture_output=True, text=True)
            self.assertEqual(r.returncode, 0, r.stderr)
            self.assertEqual(r.stdout.strip(), 'ok')


if __name__ == '__main__':
    unittest.main()
