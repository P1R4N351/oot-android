# SPDX-License-Identifier: MIT
"""Host JVM verification and structural checks for OotCompanionModel (no Android SDK needed)."""
import shutil
import subprocess
import tempfile
import unittest
from pathlib import Path

REPO = Path(__file__).resolve().parents[3]
JAVA = REPO / "android/app/src/main/java/dev/nxnative/android"
SRCS = [JAVA / "CompanionState.java", JAVA / "OotCompanionModel.java", JAVA / "Words.java"]   # Words: the model's labels
TEST = REPO / "android/app/src/jvmtest/java/dev/nxnative/android/OotCompanionJvmTest.java"


class OotCompanionTests(unittest.TestCase):
    """Compile and execute pure-JDK OotCompanionModel tests plus structural checks."""

    def test_pure_jvm_model_has_no_android_imports(self):
        """Ensure OotCompanionModel stays free of android.* imports."""
        self.assertGreater(len(SRCS), 0)
        for p in SRCS:
            txt = p.read_text(encoding="utf-8")
            self.assertNotIn("import android.", txt, p.name)

    @unittest.skipUnless(shutil.which("javac") and shutil.which("java"), "no JDK")
    def test_oot_companion_jvm_suite(self):
        """Compile with javac -Xlint:all -Werror and assert PASS output."""
        with tempfile.TemporaryDirectory(prefix="oot_comp_") as tmp:
            classes = Path(tmp) / "classes"
            classes.mkdir()
            c = subprocess.run(
                [
                    "javac", "-Xlint:all", "-Werror", "-encoding", "UTF-8",
                    "-d", str(classes), *map(str, SRCS), str(TEST),
                ],
                capture_output=True, text=True, encoding="utf-8",
            )
            self.assertEqual(c.returncode, 0, c.stderr)
            r = subprocess.run(
                ["java", "-Dstdout.encoding=UTF-8", "-cp", str(classes),
                 "dev.nxnative.android.OotCompanionJvmTest"],
                capture_output=True, text=True, encoding="utf-8",
            )
            self.assertEqual(r.returncode, 0, r.stderr + r.stdout)
            self.assertIn("OotCompanionJvmTest: PASS", r.stdout)


if __name__ == "__main__":
    unittest.main()
