#!/usr/bin/env python3
# The donation targets stay consistent: README.md carries the "Support with Bitcoin" badge linking #support and a '## Support' heading, the Bitcoin
# address written there is the one in .github/FUNDING.yml, and FUNDING.yml lists exactly the two targets (Bitcoin and the VitaDB profile). No YAML module:
# FUNDING.yml is one `custom:` key whose value is a flow list of quoted strings, read with ast.literal_eval.
import ast
import re
import unittest
from pathlib import Path

REPO = Path(__file__).resolve().parents[3]
BTC = re.compile(r'\bbc1[02-9ac-hj-np-z]{11,71}\b')   # bech32 alphabet (no 1, b, i, o after the separator)
BADGE = re.compile(r'^\[!\[Support with Bitcoin\]\([^)\s]+\)\]\(#support\)$', re.M)
HEADING = re.compile(r'^## Support\s*$', re.M)
VITADB = 'https://www.rinnegatamante.eu/vitadb/#/user/PiSCES'


def funding_targets(text):
    # the keys of FUNDING.yml (comments and blank lines skipped) and the custom list
    keys, custom = [], None
    for line in text.split('\n'):
        if not line.strip() or line.lstrip().startswith('#'): continue
        key, sep, value = line.partition(':')
        if not sep or line[0].isspace(): raise ValueError(f'FUNDING.yml: unexpected line {line!r}')
        keys.append(key.strip())
        if key.strip() == 'custom': custom = ast.literal_eval(value.strip())
    return keys, custom


class Support(unittest.TestCase):
    def setUp(self):
        self.readme = (REPO / 'README.md').read_text(encoding='utf-8')
        self.funding = (REPO / '.github/FUNDING.yml').read_text(encoding='utf-8')

    def test_readme_badge_and_heading(self):
        self.assertRegex(self.readme, BADGE, 'README.md: the "Support with Bitcoin" badge line linking #support is missing')
        self.assertRegex(self.readme, HEADING, "README.md: no '## Support' heading for the badge's #support to land on")

    def test_funding_lists_exactly_the_two_targets(self):
        keys, custom = funding_targets(self.funding)
        self.assertEqual(keys, ['custom'], 'FUNDING.yml: only the custom key is expected')
        self.assertIsInstance(custom, list); assert isinstance(custom, list)
        self.assertEqual(len(custom), 2, f'FUNDING.yml: expected two targets, got {custom!r}')
        self.assertEqual(sorted(c.partition(':')[0] for c in custom), ['bitcoin', 'https'])
        self.assertIn(VITADB, custom)

    def test_bitcoin_address_matches(self):
        _, custom = funding_targets(self.funding)
        self.assertIsInstance(custom, list); assert isinstance(custom, list)
        funded = [c.partition(':')[2] for c in custom if c.startswith('bitcoin:')]
        self.assertEqual(len(funded), 1, 'FUNDING.yml: expected one bitcoin: target')
        written = set(BTC.findall(self.readme))
        self.assertEqual(written, {funded[0]}, 'README.md and FUNDING.yml name different Bitcoin addresses')


    def test_bitcoin_address_is_the_published_one(self):
        self.assertIn('bc1q4a3lqcg87rvw7vxulausdqhhuwkwwa6w6v4mxx', self.readme, 'README.md does not carry the published Bitcoin address')
        self.assertIn('bitcoin:bc1q4a3lqcg87rvw7vxulausdqhhuwkwwa6w6v4mxx', self.funding, '.github/FUNDING.yml does not carry the published Bitcoin address')


if __name__ == '__main__':
    unittest.main()
