"""Tests for generated OoT vector launcher icons, banner, safe-zone bounds, and manifest wiring."""
import re
import sys
import unittest
import xml.etree.ElementTree as ET
from pathlib import Path

REPO = Path(__file__).resolve().parents[3]
SCRIPTS = REPO / 'scripts'
DRAWABLE = REPO / 'android/app/src/main/res/drawable'
MIPMAP = REPO / 'android/app/src/main/res/mipmap-anydpi-v26'
MANIFEST = REPO / 'android/app/src/main/AndroidManifest.xml'
ANDROID_NS = '{http://schemas.android.com/apk/res/android}'

sys.path.insert(0, str(SCRIPTS))
import make_oot_launcher_art as art  # noqa: E402


def parse_path_coords(path_data: str) -> list[tuple[float, float]]:
    nums = [float(x) for x in re.findall(r'-?\d+(?:\.\d+)?', path_data)]
    assert len(nums) % 2 == 0, f'odd coordinate count in pathData: {len(nums)}'
    return [(nums[i], nums[i + 1]) for i in range(0, len(nums), 2)]


class OotLauncherArtTests(unittest.TestCase):
    def test_xml_well_formed_and_viewports(self):
        expected = {
            DRAWABLE / 'ic_oot_bg.xml': ('108dp', '108dp', '108', '108'),
            DRAWABLE / 'ic_oot_foreground.xml': ('108dp', '108dp', '108', '108'),
            DRAWABLE / 'ic_oot_monochrome.xml': ('108dp', '108dp', '108', '108'),
            DRAWABLE / 'ic_banner.xml': ('320dp', '180dp', '320', '180'),
        }
        for path, dims in expected.items():
            root = ET.fromstring(path.read_text(encoding='utf-8'))
            self.assertEqual(root.tag, 'vector', path.name)
            got = (
                root.attrib.get(ANDROID_NS + 'width'),
                root.attrib.get(ANDROID_NS + 'height'),
                root.attrib.get(ANDROID_NS + 'viewportWidth'),
                root.attrib.get(ANDROID_NS + 'viewportHeight'),
            )
            self.assertEqual(got, dims, path.name)
        for p in (
            MIPMAP / 'ic_launcher.xml',
            MIPMAP / 'ic_launcher_round.xml',
            DRAWABLE / 'ic_launcher_foreground.xml',
            DRAWABLE / 'ic_launcher_monochrome.xml',
            DRAWABLE / 'ic_launcher.xml',
        ):
            ET.fromstring(p.read_text(encoding='utf-8'))

    def test_foreground_safe_zone_and_clean_vocabulary(self):
        for fname in ('ic_oot_foreground.xml', 'ic_oot_monochrome.xml'):
            path = DRAWABLE / fname
            text = path.read_text(encoding='utf-8')
            lower = text.lower()
            for banned in ('triforce', 'hylian', 'nintendo', 'zelda'):
                self.assertNotIn(banned, lower, fname)
            root = ET.fromstring(text)
            for elem in root.iter('path'):
                pd = elem.attrib[ANDROID_NS + 'pathData']
                for x, y in parse_path_coords(pd):
                    self.assertGreaterEqual(x, 18.0, f'{fname}: x={x} < 18')
                    self.assertLessEqual(x, 90.0, f'{fname}: x={x} > 90')
                    self.assertGreaterEqual(y, 18.0, f'{fname}: y={y} < 18')
                    self.assertLessEqual(y, 90.0, f'{fname}: y={y} > 90')

    def test_banner_clean_and_manifest_wired(self):
        btext = (DRAWABLE / 'ic_banner.xml').read_text(encoding='utf-8').lower()
        for banned in ('triforce', 'hylian', 'nintendo', 'zelda'):
            self.assertNotIn(banned, btext, 'ic_banner.xml')
        mroot = ET.fromstring(MANIFEST.read_text(encoding='utf-8'))
        app = mroot.find('application')
        self.assertIsNotNone(app); assert app is not None
        self.assertEqual(app.attrib.get(ANDROID_NS + 'banner'), '@drawable/ic_banner')
        pkgs = [
            p.attrib.get(ANDROID_NS + 'name')
            for q in mroot.findall('queries')
            for p in q.findall('package')
        ]
        self.assertIn('com.moonbench.bifrost', pkgs)

    def test_generator_matches_checked_in_files(self):
        pairs = {
            DRAWABLE / 'ic_oot_bg.xml': art.render_bg(),
            DRAWABLE / 'ic_oot_foreground.xml': art.render_foreground(),
            DRAWABLE / 'ic_oot_monochrome.xml': art.render_monochrome(),
            DRAWABLE / 'ic_banner.xml': art.render_banner(),
        }
        for path, generated in pairs.items():
            self.assertEqual(path.read_text(encoding='utf-8'), generated, path.name)


if __name__ == '__main__':
    unittest.main()
